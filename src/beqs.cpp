// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  beqs.cpp -- "Bloomberg EQS" query function for the BLP API
//
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel and John Laing
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

// TODO: Date, Datetime, Int (?), ... results

#include <vector>
#include <string>
#include <blpapi_session.h>
#include <blpapi_service.h>
#include <blpapi_request.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <Rcpp.h>
#include <blpapi_utils.h>


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <array>
#include <iterator>


#include <sstream>
using namespace std;
using namespace Rcpp;

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

Rcpp::DataFrame processResponseEvent(Event event, const bool verbose) {

    MessageIterator msgIter(event); 			// create message iterator
    if (!msgIter.next()) throw std::logic_error("Not a valid MessageIterator.");

    Message msg = msgIter.message(); 			// get message
    if (verbose) msg.print(Rcpp::Rcout);

    Element response = msg.asElement(); 		// view as element
    if (std::strcmp(response.name().string(), "BeqsResponse") != 0)
        throw std::logic_error("Not a valid EQSDataResponse.");

    Element data = msg.getElement("data"); 		// get data payload, extract field with display units
    Element fieldDisplayUnits = data.getElement("fieldDisplayUnits");
    if (verbose) fieldDisplayUnits.print(Rcpp::Rcout);

    int cols = fieldDisplayUnits.numElements(); 	// copy display units into column names
    std::vector<std::string> colnames(cols);
    for (int i=0; i<cols; i++) {
        colnames[i] = fieldDisplayUnits.getElement(i).name().string();
    }

    Rcpp::List lst(cols);       			// Rcpp 'List' container of given number of columns
    Rcpp::LogicalVector chk(cols, false);
    bool allgood;

    Element results = data.getElement("securityData"); 	// get security data payload == actual result set
    int rows = results.numValues();                     // total number of rows in result set
    if (verbose) results.print(Rcpp::Rcout);
    if (verbose) Rcpp::Rcout << rows << " Rows expected" << std::endl;

    for (int j = 0; j < rows; j++) { 		// look at all rows to infer types, break if all found
        response = results.getValueAsElement(j); 	// pick j-th element to infer types
        data = response.getElement("fieldData");       	// get data payload of first elemnt
        if (verbose) data.print(Rcpp::Rcout);

        allgood = true;
        for (int i=0; i<cols; i++) { 			// loop over first data set, and infer types
            if (!chk(i) &&                              // column has not been set yet
                data.hasElement(colnames[i].c_str())) {
                Element val = data.getElement(colnames[i].c_str());
                if (val.datatype() == BLPAPI_DATATYPE_STRING) {
                    lst[i] = Rcpp::CharacterVector(rows, R_NaString);
                    chk[i] = true;
                } else if (val.datatype() == BLPAPI_DATATYPE_FLOAT64) {
                    lst[i] = Rcpp::NumericVector(rows, NA_REAL);
                    chk[i] = true;
                } else if (val.datatype() == BLPAPI_DATATYPE_DATE) {
                    lst[i] = Rcpp::DateVector(rows);
                    chk[i] = true;
                } else {                  			// fallback
                    //Rcpp::Rcout << "Seeing type " << val.datatype() << std::endl;
                    lst[i] = Rcpp::CharacterVector(rows, R_NaString);
                    chk[i] = true;
                }
            }
            allgood = allgood and chk[i];
        }

        if (allgood) {
            break;
        }
    }

    if (!allgood) {               // Check if any columns have not been checked successfully - these will all set to fallback NA
        for (int i=0; i<cols; i++) {
            if (!chk[i]) {
                lst[i] = Rcpp::NumericVector(rows, NA_REAL);
                chk[i] = true;
            }
        }
    }

    for (int i = 0; i < rows; i++) { 			// now process data

        Element elem = results.getValueAsElement(i); 	// extract i-th element
        Element data = elem.getElement("fieldData");    // extract its data payload
        if (verbose) data.print(Rcpp::Rcout);

        for (int j = 0; j < cols; j++) { 		// over all columns

            if (data.hasElement(colnames[j].c_str())) { // assign, if present, to proper column and type
                Element datapoint = data.getElement(colnames[j].c_str());
                if (datapoint.datatype() == BLPAPI_DATATYPE_STRING) {
                    Rcpp::CharacterVector v = lst[j];
                    if (colnames[j] == "Ticker") {
                        std::string tickerValue = datapoint.getValueAsString() + std::string(" Equity");
                        v[i] = tickerValue;
                    } else {
                        std::string sValue = datapoint.getValueAsString();
                        v[i] = sValue;
                    }
                    lst[j] = v;
                } else if (datapoint.datatype() == BLPAPI_DATATYPE_FLOAT64) {
                    Rcpp::NumericVector v = lst[j];
                    v[i] = datapoint.getValueAsFloat64();
                    lst[j] = v;
                } else if (datapoint.datatype() == BLPAPI_DATATYPE_DATE) {
                    Rcpp::DateVector v = lst[j];
                    v[i] = Rcpp::Date(datapoint.getValueAsString());
                    lst[j] = v;
                } else {
                    Rcpp::CharacterVector v = lst[j];
                    std::string sValue = datapoint.getValueAsString();
                    v[i] = sValue;
                    lst[j] = v;
                }
            }
        }
    }

    lst.attr("names") = colnames;
    Rcpp::DataFrame df(lst);
    return df;
}



// [[Rcpp::export]]
DataFrame beqs_Impl(SEXP con,
                    std::string screenName,
                    std::string screenType,
                    std::string group,
                    std::string pitdate,
                    std::string languageId,
                    bool verbose=false) {

    Session* session = reinterpret_cast<Session*>(checkExternalPointer(con, "blpapi::Session*"));

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }

    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("BeqsRequest");

    request.set("screenName", screenName.c_str());
    request.set("screenType", screenType.c_str());

    if (group != "") {
        request.set ("Group", group.c_str());
    }

    if (languageId != "") {
        request.set ("languageId", languageId.c_str());
    }

    Element overrides = request.getElement("overrides");
    if (pitdate != "") {
        Element override1 = overrides.appendElement();
        override1.setElement("fieldId", "PiTDate");
        override1.setElement("value", pitdate.c_str());
    }

    if (verbose) Rcpp::Rcout <<"Sending Request: " << request << std::endl;
    session->sendRequest(request);

    DataFrame ans;

    // Wait for events from Session
    bool done = false;
    while (!done) {
        Event event = session->nextEvent();
        if (event.eventType() == Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            ans = processResponseEvent(event, verbose);
        } else if (event.eventType() == Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            ans = processResponseEvent(event, verbose);
            done = true;
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

    return ans;

}
