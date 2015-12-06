// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  getBars.cpp -- a simple intraday bar retriever
//
//  Copyright (C) 2013         Whit Armstrong
//  Copyright (C) 2014 - 2015  Whit Armstrong and Dirk Eddelbuettel
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

//  Derived from IntradayBarExample.cpp in the blpapi examples.
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

#include <blpapi_utils.h>

namespace bbg = BloombergLP::blpapi;	// shortcut to not globally import both namespace

namespace {
    const bbg::Name BAR_DATA("barData");
    const bbg::Name BAR_TICK_DATA("barTickData");
    const bbg::Name OPEN("open");
    const bbg::Name HIGH("high");
    const bbg::Name LOW("low");
    const bbg::Name CLOSE("close");
    const bbg::Name VOLUME("volume");
    const bbg::Name NUM_EVENTS("numEvents");
    const bbg::Name TIME("time");
    const bbg::Name RESPONSE_ERROR("responseError");
    const bbg::Name SESSION_TERMINATED("SessionTerminated");
    const bbg::Name CATEGORY("category");
    //const bbg::Name MESSAGE("message"); // for some reason this does not compile
    const bbg::Name VALUE("value");
}

struct Bars {
    std::vector<double> time;     // to be converted to POSIXct later
    std::vector<double> open;
    std::vector<double> high;
    std::vector<double> low;
    std::vector<double> close;
    std::vector<int> numEvents;
    std::vector<double> volume;   // instread of long long
    std::vector<double> value;   // instread of long long
};

void processMessage(bbg::Message &msg, Bars &bars,
                    const int barInterval, const bool verbose) {
    bbg::Element data = msg.getElement(BAR_DATA).getElement(BAR_TICK_DATA);
    int numBars = data.numValues();
    if (verbose) {
        Rcpp::Rcout <<"Response contains " << numBars << " bars" << std::endl;
        Rcpp::Rcout <<"Datetime\t\tOpen\t\tHigh\t\tLow\t\tClose" <<
            "\t\tNumEvents\tVolume\t\tV" << std::endl;
    }
    for (int i = 0; i < numBars; ++i) {
        bbg::Element bar = data.getValueAsElement(i);
        bbg::Datetime time = bar.getElementAsDatetime(TIME);
        assert(time.hasParts(bbg::DatetimeParts::DATE
                             | bbg::DatetimeParts::HOURS
                             | bbg::DatetimeParts::MINUTES));
        double open = bar.getElementAsFloat64(OPEN);
        double high = bar.getElementAsFloat64(HIGH);
        double low = bar.getElementAsFloat64(LOW);
        double close = bar.getElementAsFloat64(CLOSE);
        int numEvents = bar.getElementAsInt32(NUM_EVENTS);
        long long volume = bar.getElementAsInt64(VOLUME);
        double value = bar.getElementAsFloat64(VALUE);

        if (verbose) {
            Rcpp::Rcout.setf(std::ios::fixed, std::ios::floatfield);
            Rcpp::Rcout << time.month() << '/' << time.day() << '/' << time.year()
                        << " " << time.hours() << ":" << time.minutes()
                        <<  "\t\t" << std::showpoint
                        << std::setprecision(3) << open << "\t\t"
                        << high << "\t\t"
                        << low <<  "\t\t"
                        << close <<  "\t\t"
                        << numEvents <<  "\t\t"
                        << std::noshowpoint
                        << volume << "\t\t"
                        << value << std::endl;
        }
        bars.time.push_back(bbgDatetimeToUTC(time));
        bars.open.push_back(open);
        bars.high.push_back(high);
        bars.low.push_back(low);
        bars.close.push_back(close);
        bars.numEvents.push_back(numEvents);
        bars.volume.push_back(volume);
        bars.value.push_back(value);
    }
}

void processResponseEvent(bbg::Event &event, Bars &bars,
                          const int barInterval, const bool verbose) {
    bbg::MessageIterator msgIter(event);
    while (msgIter.next()) {
        bbg::Message msg = msgIter.message();
        if (msg.hasElement(RESPONSE_ERROR)) {
            Rcpp::Rcerr << "REQUEST FAILED: " << msg.getElement(RESPONSE_ERROR) << std::endl;
            continue;
        }
        processMessage(msg, bars, barInterval, verbose);
    }
}

// [[Rcpp::export]]
Rcpp::DataFrame getBars_Impl(SEXP con,
                             std::string security,
                             std::string eventType,
                             int barInterval,
                             std::string startDateTime,
                             std::string endDateTime,
                             Rcpp::Nullable<Rcpp::CharacterVector> options,
                             bool verbose=false) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    bbg::Session* session =
        reinterpret_cast<bbg::Session*>(checkExternalPointer(con,"blpapi::Session*"));

    if (!session->openService("//blp/refdata")) {
        Rcpp::stop("Failed to open //blp/refdata");
    }

    bbg::Service refDataService = session->getService("//blp/refdata");
    bbg::Request request = refDataService.createRequest("IntradayBarRequest");

    // only one security/eventType per request
    request.set("security", security.c_str());
    request.set("eventType", eventType.c_str());
    request.set("interval", barInterval);

    request.set("startDateTime", startDateTime.c_str());
    request.set("endDateTime", endDateTime.c_str());
    if (options.isNotNull()) {
        appendOptionsToRequest(request, options);
    }

    if (verbose) Rcpp::Rcout <<"Sending Request: " << request << std::endl;
    session->sendRequest(request);

    Bars bars;

    // eventLoop
    bool done = false;
    while (!done) {
        bbg::Event event = session->nextEvent();
        if (event.eventType() == bbg::Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            processResponseEvent(event, bars, barInterval, verbose);
        } else if (event.eventType() == bbg::Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            processResponseEvent(event, bars, barInterval, verbose);
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

    return Rcpp::DataFrame::create(Rcpp::Named("times")     = createPOSIXtVector(bars.time),
                                   Rcpp::Named("open")      = bars.open,
                                   Rcpp::Named("high")      = bars.high,
                                   Rcpp::Named("low")       = bars.low,
                                   Rcpp::Named("close")     = bars.close,
                                   Rcpp::Named("numEvents") = bars.numEvents,
                                   Rcpp::Named("volume")    = bars.volume,
                                   Rcpp::Named("value")     = bars.value);

}



