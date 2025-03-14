//
//  lookup.cpp -- symbol look-up
//
//  Copyright (C) 2017 - 2025  Dirk Eddelbuettel and Kevin Jin
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

#include <blpapi_session.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <iostream>
#include <vector>
#include <string>
#include <blpapi_utils.h>

namespace bbg = BloombergLP::blpapi;	// shortcut to not globally import both namespace

namespace {
    const bbg::Name SECURITY("security");
    const bbg::Name DESCRIPTION("description");
    const bbg::Name RESPONSE_ERROR("responseError");
    const bbg::Name SESSION_TERMINATED("SessionTerminated");
}

struct InstrumentListResults {
    std::vector<std::string> security;
    std::vector<std::string> description;
};

void processMessage(bbg::Message &msg, InstrumentListResults &matches, const bool verbose) {
    bbg::Element response = msg.asElement();
    if (verbose) response.print(Rcpp::Rcout);
    if (std::strcmp(response.name().string(),"InstrumentListResponse")) {
        Rcpp::stop("Not a valid InstrumentListResponse.");
    }

    bbg::Element data = response.getElement(bbg::Name{"results"});
    int numItems = data.numValues();
    if (verbose) {
        Rcpp::Rcout <<"Response contains " << numItems << " items" << std::endl;
        Rcpp::Rcout <<"security\t\tdescription" << std::endl;
    }

    for (int i = 0; i < numItems; ++i) {
        bbg::Element item = data.getValueAsElement(i);
        std::string security = item.getElementAsString(SECURITY);
        std::string description = item.getElementAsString(DESCRIPTION);
        if (verbose) {
            Rcpp::Rcout << security << "\t\t" << description << std::endl;
        }
        matches.security.push_back(security);
        matches.description.push_back(description);
    }
}

void processResponseEvent(bbg::Event &event, InstrumentListResults &matches, const bool verbose) {
    bbg::MessageIterator msgIter(event);
    while (msgIter.next()) {
        bbg::Message msg = msgIter.message();
        if (msg.hasElement(RESPONSE_ERROR)) {
            Rcpp::Rcerr << "REQUEST FAILED: " << msg.getElement(RESPONSE_ERROR) << std::endl;
            continue;
        }
        processMessage(msg, matches, verbose);
    }
}
#else
#include <Rcpp/Lightest>
#endif

// [[Rcpp::export]]
Rcpp::DataFrame lookup_Impl(SEXP con,
                            std::string query,
                            std::string yellowKeyFilter="YK_FILTER_NONE",
                            std::string languageOverride="LANG_OVERRIDE_NONE",
                            int maxResults=20,
                            bool verbose=false) {
#if defined(HaveBlp)
    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    bbg::Session* session =
        reinterpret_cast<bbg::Session*>(checkExternalPointer(con,"blpapi::Session*"));

    if (!session->openService("//blp/instruments")) {
        Rcpp::stop("Failed to open //blp/instruments");
    }

    bbg::Service secfService = session->getService("//blp/instruments");
    bbg::Request request = secfService.createRequest("instrumentListRequest");

    request.set(bbg::Name{"query"}, query.c_str());
    request.set(bbg::Name{"yellowKeyFilter"}, yellowKeyFilter.c_str());
    request.set(bbg::Name{"languageOverride"}, languageOverride.c_str());
    request.set(bbg::Name{"maxResults"}, maxResults);

    if (verbose) Rcpp::Rcout <<"Sending Request: " << request << std::endl;
    session->sendRequest(request);

    InstrumentListResults matches;

    // eventLoop
    bool done = false;
    while (!done) {
        bbg::Event event = session->nextEvent();
        if (event.eventType() == bbg::Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            processResponseEvent(event, matches, verbose);
        } else if (event.eventType() == bbg::Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            processResponseEvent(event, matches, verbose);
            done = true;
        } else {
            bbg::MessageIterator msgIter(event);
            while (msgIter.next()) {
                bbg::Message msg = msgIter.message();
                if (event.eventType() == bbg::Event::SESSION_STATUS) {
                    if (msg.messageType() == SESSION_TERMINATED) {
                        done = true;
                    }
                }
            }
        }
    }

    return Rcpp::DataFrame::create(Rcpp::Named("security") = matches.security,
                                   Rcpp::Named("description") = matches.description);
#else // ie no Blp
    return Rcpp::DataFrame();
#endif
}
