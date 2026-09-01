//
//  bql.cpp -- "Bloomberg Query Language" query function for the BLP API
//
//  Copyright (C) 2025  Whit Armstrong and Dirk Eddelbuettel and John Laing
//
//  This file is part of Rblpapi
//
//  Rblpapi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  Rblpapi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

#if defined(HaveBlp)
#include <vector>
#include <string>
#include <blpapi_session.h>
#include <blpapi_service.h>
#include <blpapi_request.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_exception.h>
#include <blpapi_utils.h>

using namespace Rcpp;

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;
using BloombergLP::blpapi::Name;
using BloombergLP::blpapi::NotFoundException;

// The //blp/bqlsvc service returns each response message as a single
// string-typed element holding a JSON document. Collect those strings;
// parsing is done R-side (see R/bql.R).
void processBqlEvent(Event event, std::vector<std::string>& res, const bool verbose) {
    MessageIterator msgIter(event);
    while (msgIter.next()) {
        Message msg = msgIter.message();
        if (verbose) msg.print(Rcpp::Rcout);

        Element response = msg.asElement();
        if (response.hasElement(Name{"responseError"})) {
            Element err = response.getElement(Name{"responseError"});
            Rcpp::stop("Response error: " + std::string(err.getElementAsString(Name{"message"})));
        }
        if (response.datatype() == BLPAPI_DATATYPE_STRING) {
            res.push_back(response.getValueAsString());
        } else if (verbose) {
            Rcpp::Rcout << "Skipping non-string message of type "
                        << msg.messageType().string() << std::endl;
        }
    }
}
#else
#include <Rcpp/Lightest>
#endif

// [[Rcpp::export]]
Rcpp::CharacterVector bql_Impl(SEXP con,
                               std::string expression,
                               bool verbose=false) {
#if defined(HaveBlp)
    Session* session = reinterpret_cast<Session*>(checkExternalPointer(con, "blpapi::Session*"));

    const std::string bqlsvc = "//blp/bqlsvc";
    if (!session->openService(bqlsvc.c_str())) {
        Rcpp::stop("Failed to open " + bqlsvc);
    }

    Service bqlService = session->getService(bqlsvc.c_str());
    Request request = bqlService.createRequest("sendQuery");
    request.getElement(Name{"expression"}).setValue(expression.c_str());
    // the service expects the same client context the Excel BQL add-in sends
    try {
        Element clientContext = request.getElement(Name{"clientContext"});
        clientContext.setElement(Name{"appName"}, "EXCEL");
    } catch (NotFoundException& e) {
        if (verbose) Rcpp::Rcout << "No 'clientContext' element in request schema" << std::endl;
    }

    if (verbose) Rcpp::Rcout << "Sending Request: " << request << std::endl;
    session->sendRequest(request);

    std::vector<std::string> res;

    // Wait for events from Session
    bool done = false;
    while (!done) {
        Event event = session->nextEvent();
        if (event.eventType() == Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            processBqlEvent(event, res, verbose);
        } else if (event.eventType() == Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            processBqlEvent(event, res, verbose);
            done = true;
        } else if (event.eventType() == Event::REQUEST_STATUS) {
            // a rejected or timed-out request sends this and never a RESPONSE,
            // so without it nextEvent() would block for good (cf. bdh.cpp)
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (verbose) msg.asElement().print(Rcpp::Rcout);
            }
            Rcpp::stop("Bloomberg request timed out on server side");
        } else {
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::SESSION_STATUS) {
                    if (msg.messageType() == "SessionTerminated" ||
                        msg.messageType() == "SessionStartupFailure") {
                        done = true;
                    }
                }
            }
        }
    }

    return Rcpp::wrap(res);
#else // ie no Blp
    return Rcpp::CharacterVector();
#endif
}
