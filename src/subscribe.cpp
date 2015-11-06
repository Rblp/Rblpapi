// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  subscribe.cpp -- "Bloomberg Data Point" query function for the BLP API
//
//  Copyright (C) 2015  Whit Armstrong
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


// compare to SimpleSubscriptionExample.cpp


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <blpapi_defs.h>
#include <blpapi_correlationid.h>
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_session.h>
#include <blpapi_subscriptionlist.h>
#include <Rcpp.h>
#include <blpapi_utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::SubscriptionList;
using BloombergLP::blpapi::CorrelationId;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

const std::map<Event::EventType,std::string> BlpapiEventToString { {Event::ADMIN,"ADMIN"},{Event::SESSION_STATUS,"SESSION_STATUS"},{Event::SUBSCRIPTION_STATUS,"SUBSCRIPTION_STATUS"},{Event::REQUEST_STATUS,"REQUEST_STATUS"},{Event::RESPONSE,"RESPONSE"},{Event::PARTIAL_RESPONSE,"PARTIAL_RESPONSE"},{Event::SUBSCRIPTION_DATA,"SUBSCRIPTION_DATA"},{Event::SERVICE_STATUS,"SERVICE_STATUS"},{Event::TIMEOUT,"TIMEOUT"},{Event::AUTHORIZATION_STATUS,"AUTHORIZATION_STATUS"},{Event::RESOLUTION_STATUS,"RESOLUTION_STATUS"},{Event::TOPIC_STATUS,"TOPIC_STATUS"},{Event::TOKEN_STATUS,"TOKEN_STATUS"},{Event::REQUEST,"REQUEST"},{Event::UNKNOWN,"UNKNOWN"} };

SEXP recursiveParse(const Element& e) {
    if(e.numElements()) {
        Rcpp::List ans(e.numElements());
        Rcpp::StringVector names(e.numElements());
        for(size_t i = 0; i < e.numElements(); ++i) {
            //std::cout << "************: " << e.getElement(i).name().string() << std::endl;
            names(i) = e.getElement(i).name().string();
            ans[i] = recursiveParse(e.getElement(i));
        }
        ans.attr("names") = names;
        return Rcpp::wrap(ans);
    } else {
        if(e.numValues()==0) {
            return R_NilValue;
        }
        Rcpp::StringVector ans(e.numValues());
        for(size_t i = 0; i < e.numValues(); ++i) {
            ans[i] = e.getValueAsString();
        }
        return Rcpp::wrap(ans);
    }
}

// [[Rcpp::export]]
SEXP subscribe_Impl(SEXP con_, std::vector<std::string> securities, std::vector<std::string> fields,
                    SEXP options_, SEXP identity_) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    const std::string mdsrv = "//blp/mktdata";
    if (!session->openService(mdsrv.c_str())) {
        Rcpp::stop("Failed to open " + mdsrv);
    }

    SubscriptionList subscriptions;

    // put fields into comma delimited format
    std::string fields_collapsed(vectorToCSVString(fields));
    std::string options_collapsed("");
    if(options_ != R_NilValue && Rf_length(options_)) {
        std::vector<std::string> options(Rcpp::as< std::vector<std::string> >(options_));
        options_collapsed = vectorToCSVString(options);
    }

    for(const std::string& security : securities) {
        CorrelationId cid(reinterpret_cast<void*>(const_cast<char *>(security.c_str())));
        subscriptions.add(security.c_str(),fields_collapsed.c_str(),options_collapsed.c_str(),cid);
    }

    // check if identity was passed, if so, use it
    if(identity_ != R_NilValue) {
        Identity* ip;
        ip = reinterpret_cast<Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
        session->subscribe(subscriptions,*ip);
    } else {
        session->subscribe(subscriptions);
    }

    int d_eventCount(0), d_maxEvents(10);

    while (true) {
        Event event = session->nextEvent();
        MessageIterator msgIter(event);
        while (msgIter.next()) {
            Message msg = msgIter.message();
            const char *topic = (char *)msg.correlationId().asPointer();
            if (event.eventType() == Event::SUBSCRIPTION_STATUS ||
                event.eventType() == Event::SUBSCRIPTION_DATA) {

                Rcpp::List ans;
                auto it = BlpapiEventToString.find(event.eventType());
                if(it==BlpapiEventToString.end()) {
                    // throw
                }
                ans["event.type"] = it->second;
                ans["topic"] = topic;
                ans["payload"] = recursiveParse(msg.asElement());
                if(++d_eventCount >= d_maxEvents) {
                    return Rcpp::wrap(ans);
                }
            }
        }
    }
    return R_NilValue;
}
