// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  subscribe.cpp -- market data subscription function for the BLP API
//
//  Copyright (C) 2015-2025  Whit Armstrong
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
using BloombergLP::blpapi::DatetimeParts;

const std::map<Event::EventType,std::string> BlpapiEventToString { {Event::ADMIN,"ADMIN"},{Event::SESSION_STATUS,"SESSION_STATUS"},{Event::SUBSCRIPTION_STATUS,"SUBSCRIPTION_STATUS"},{Event::REQUEST_STATUS,"REQUEST_STATUS"},{Event::RESPONSE,"RESPONSE"},{Event::PARTIAL_RESPONSE,"PARTIAL_RESPONSE"},{Event::SUBSCRIPTION_DATA,"SUBSCRIPTION_DATA"},{Event::SERVICE_STATUS,"SERVICE_STATUS"},{Event::TIMEOUT,"TIMEOUT"},{Event::AUTHORIZATION_STATUS,"AUTHORIZATION_STATUS"},{Event::RESOLUTION_STATUS,"RESOLUTION_STATUS"},{Event::TOPIC_STATUS,"TOPIC_STATUS"},{Event::TOKEN_STATUS,"TOKEN_STATUS"},{Event::REQUEST,"REQUEST"},{Event::UNKNOWN,"UNKNOWN"} };

SEXP eleToLogical(const Element& e) {
    Rcpp::LogicalVector ans(e.numValues());
    for(size_t i = 0; i < e.numValues(); ++i) {
        ans[i] = e.getValueAsBool(i);
    }
    return Rcpp::wrap(ans);
}

SEXP eleToString(const Element& e) {
    Rcpp::StringVector ans(e.numValues());
    for(size_t i = 0; i < e.numValues(); ++i) {
        ans[i] = e.getValueAsString(i);
    }
    return Rcpp::wrap(ans);
}

SEXP eleToDouble(const Element& e) {
    Rcpp::NumericVector ans(e.numValues());
    for(size_t i = 0; i < e.numValues(); ++i) {
        ans[i] = e.getValueAsFloat64(i);
    }
    return Rcpp::wrap(ans);
}

SEXP eleToInt(const Element& e) {
    Rcpp::IntegerVector ans(e.numValues());
    for(size_t i = 0; i < e.numValues(); ++i) {
        ans[i] = e.getValueAsInt32(i);
    }
    return Rcpp::wrap(ans);
}

SEXP eleToDate(const Element& e) {
    Rcpp::DateVector ans(e.numValues());
    for(size_t i = 0; i < e.numValues(); ++i) {
        ans[i] = bbgDateToRDate(e.getValueAsDatetime(i));
    }
    return Rcpp::wrap(ans);
}

SEXP eleToDatetime(const Element& e) {

    // sometimes bbg uses BLPAPI_DATATYPE_DATETIME for timestamps w/ no day/month/year attribues
    if(e.getValueAsDatetime().hasParts(DatetimeParts::DATE)) {
        Rcpp::DatetimeVector ans(e.numValues());
        for(size_t i = 0; i < e.numValues(); ++i) {
            ans[i] = bbgDatetimeToPOSIX(e.getValueAsDatetime(i));
        }
        return Rcpp::wrap(ans);
    } else {
        return eleToString(e);
    }
}

SEXP eleToArray(const Element& e) {
    if(e.isNull()) { return R_NilValue; }
    switch(e.datatype()) {
    case BLPAPI_DATATYPE_BOOL:
        return eleToLogical(e);
    case BLPAPI_DATATYPE_CHAR:
        return eleToString(e);
    case BLPAPI_DATATYPE_BYTE:
        return R_NilValue;
    case BLPAPI_DATATYPE_INT32:
        return eleToInt(e);
    case BLPAPI_DATATYPE_INT64:
        return R_NilValue;
    case BLPAPI_DATATYPE_FLOAT32:
    case BLPAPI_DATATYPE_FLOAT64:
        return eleToDouble(e);
    case BLPAPI_DATATYPE_STRING:
        return eleToString(e);
    case BLPAPI_DATATYPE_BYTEARRAY:
        return R_NilValue;
    case BLPAPI_DATATYPE_DATE:
        return eleToDate(e);
    case BLPAPI_DATATYPE_TIME:
        return eleToString(e);
    case BLPAPI_DATATYPE_DECIMAL:
        return eleToDouble(e);
    case BLPAPI_DATATYPE_DATETIME:
        return eleToDatetime(e);
    case BLPAPI_DATATYPE_ENUMERATION:
        return eleToString(e);
    case BLPAPI_DATATYPE_SEQUENCE:
        return eleToString(e);
    case BLPAPI_DATATYPE_CHOICE:
        return eleToString(e);
    case BLPAPI_DATATYPE_CORRELATION_ID:
        return eleToString(e);
     default:
        return R_NilValue;
    }
}

SEXP recursiveParse(const Element& e) {
    if(e.numElements()) {
        Rcpp::List ans(e.numElements());
        Rcpp::StringVector names(e.numElements());
        for(size_t i = 0; i < e.numElements(); ++i) {
            names(i) = e.getElement(i).name().string();
            ans[i] = recursiveParse(e.getElement(i));
        }
        ans.attr("names") = names;
        return Rcpp::wrap(ans);
    } else {
        if(e.numValues()==0) {
            return R_NilValue;
        } else {
            return eleToArray(e);
        }
    }
}
#else
#include <Rcpp/Lightest>
#endif

// [[Rcpp::export]]
SEXP subscribe_Impl(SEXP con_, std::vector<std::string> securities, std::vector<std::string> fields,
                    Rcpp::Function fun, SEXP options_, SEXP identity_) {
#if defined(HaveBlp)
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

    for(size_t i = 0; i < securities.size(); ++i) {
        subscriptions.add(securities[i].c_str(),fields_collapsed.c_str(),options_collapsed.c_str(),CorrelationId(i));
    }

    // check if identity was passed, if so, use it
    if(identity_ != R_NilValue) {
        Identity* ip;
        ip = reinterpret_cast<Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
        session->subscribe(subscriptions,*ip);
    } else {
        session->subscribe(subscriptions);
    }

    try {
        while (true) {
            Event event = session->nextEvent();
            Rcpp::checkUserInterrupt();
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::SUBSCRIPTION_STATUS ||
                    event.eventType() == Event::SUBSCRIPTION_DATA) {

                    Rcpp::List ans;
                    auto it = BlpapiEventToString.find(event.eventType());
                    if(it==BlpapiEventToString.end()) {
                        Rcpp::stop("Unknown event type.");
                    }
                    ans["event.type"] = it->second;
                    size_t cid(msg.correlationId().asInteger());
                    if(cid >= 0 && cid < securities.size()) {
                        ans["topic"] = securities[cid];
                    }
                    ans["data"] = recursiveParse(msg.asElement());
                    // call user function
                    fun(ans);
                }
            }
        }
    } catch (const Rcpp::internal::InterruptedException& e) {
        session->unsubscribe(subscriptions);
    }
    return R_NilValue;
#else // ie no Blp
    return R_NilValue;
#endif
}
