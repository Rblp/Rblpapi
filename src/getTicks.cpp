// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  getTicks.cpp -- a simple intraday tick retriever
//
//  Copyright (C) 2013         Whit Armstrong
//  Copyright (C) 2014 - 2016  Whit Armstrong and Dirk Eddelbuettel
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

//  Derived from IntradayTickExample.cpp in the blpapi examples.
//  It contained the following header

/* Copyright 2012. Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <vector>
#include <string>
#include <blpapi_session.h>
#include <blpapi_eventdispatcher.h>

#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_defs.h>


#include <time.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>

#include <Rcpp.h>
#include <blpapi_utils.h>

namespace bbg = BloombergLP::blpapi;	// shortcut to not globally import both namespace

namespace {
    const bbg::Name TICK_DATA("tickData");
    const bbg::Name COND_CODE("conditionCodes");
    const bbg::Name TICK_SIZE("size");
    const bbg::Name TIME("time");
    const bbg::Name TYPE("type");
    const bbg::Name VALUE("value");
    const bbg::Name RESPONSE_ERROR("responseError");
    const bbg::Name CATEGORY("category");
    //const bbg::Name MESSAGE("message"); // for some reason this does not compile
    const bbg::Name SESSION_TERMINATED("SessionTerminated");
}

struct Ticks {
    std::vector<double> time;     // to be converted to POSIXct later
    std::vector<std::string> type;     // string field to save quote type
    std::vector<double> value;
    std::vector<double> size;
    std::vector<std::string> conditionCode;
};

void processMessage(bbg::Message &msg, Ticks &ticks, const bool verbose) {
    bbg::Element data = msg.getElement(TICK_DATA).getElement(TICK_DATA);
    int numItems = data.numValues();
    if (verbose) {
        Rcpp::Rcout <<"Response contains " << numItems << " items" << std::endl;
        Rcpp::Rcout <<"Time\t\tType\t\tValue\t\tSize\t\tCondition Code" << std::endl;
    }
    for (int i = 0; i < numItems; ++i) {
        bbg::Element item = data.getValueAsElement(i);
        bbg::Datetime time = item.getElementAsDatetime(TIME);
        std::string type = item.getElementAsString(TYPE);
        double value = item.getElementAsFloat64(VALUE);
        int size = item.getElementAsInt32(TICK_SIZE);
        std::string conditionCode;
        conditionCode = (item.hasElement(COND_CODE)) ? item.getElementAsString(COND_CODE) : "";
        if (verbose) {
            Rcpp::Rcout.setf(std::ios::fixed, std::ios::floatfield);
            Rcpp::Rcout << time.month() << '/' << time.day() << '/' << time.year()
                        << " " << time.hours() << ":" << time.minutes()
                        <<  "\t\t" << std::showpoint
                        << type << "\t\t"
                        << value << "\t\t"
                        << size << "\t\t"
                        << conditionCode
                        << std::endl;
        }
        ticks.time.push_back(bbgDatetimeToUTC(time));
        ticks.type.push_back(type);    
        ticks.value.push_back(value);
        ticks.size.push_back(size);
        ticks.conditionCode.push_back(conditionCode);
    }
}

void processResponseEvent(bbg::Event &event, Ticks &ticks, const bool verbose) {
    bbg::MessageIterator msgIter(event);
    while (msgIter.next()) {
        bbg::Message msg = msgIter.message();
        if (msg.hasElement(RESPONSE_ERROR)) {
            Rcpp::Rcerr << "REQUEST FAILED: " << msg.getElement(RESPONSE_ERROR) << std::endl;
            continue;
        }
        processMessage(msg, ticks, verbose);
    }
}

// [[Rcpp::export]]
Rcpp::DataFrame getTicks_Impl(SEXP con,
                              std::string security,
                              std::vector<std::string> eventType,
                              std::string startDateTime,
                              std::string endDateTime,
                              bool setCondCodes=true,  
                              bool verbose=false) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    bbg::Session* session =
        reinterpret_cast<bbg::Session*>(checkExternalPointer(con,"blpapi::Session*"));

    if (!session->openService("//blp/refdata")) {
        Rcpp::stop("Failed to open //blp/refdata");
    }

    bbg::Service refDataService = session->getService("//blp/refdata");
    bbg::Request request = refDataService.createRequest("IntradayTickRequest");

    // only one security/eventType per request
    request.set("security", security.c_str());

    bbg::Element eventTypes = request.getElement("eventTypes");
    for (size_t i = 0; i < eventType.size(); i++) {
        eventTypes.appendValue(eventType[i].c_str());
    }
    
    request.set("includeConditionCodes", setCondCodes);
    request.set("includeNonPlottableEvents", setCondCodes);
    request.set("startDateTime", startDateTime.c_str());
    request.set("endDateTime", endDateTime.c_str());

    if (verbose) Rcpp::Rcout <<"Sending Request: " << request << std::endl;
    session->sendRequest(request);

    Ticks ticks;

    // eventLoop
    bool done = false;
    while (!done) {
        bbg::Event event = session->nextEvent();
        if (event.eventType() == bbg::Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            processResponseEvent(event, ticks, verbose);
        } else if (event.eventType() == bbg::Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            processResponseEvent(event, ticks, verbose);
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

    return Rcpp::DataFrame::create(Rcpp::Named("times") = createPOSIXtVector(ticks.time),
                                   Rcpp::Named("type") = ticks.type, 
                                   Rcpp::Named("value") = ticks.value,
                                   Rcpp::Named("size")  = ticks.size, 
    	                           Rcpp::Named("condcode") = ticks.conditionCode);

}

