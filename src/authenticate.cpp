// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  authenticate.cpp -- Function to authenticate to Bloomberg backend
//
//  Copyright (C) 2013  Whit Armstrong
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel
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


#include <string>
#include <blpapi_session.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <Rcpp.h>
#include <finalizers.h>
#include <blpapi_utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

static void identityFinalizer(SEXP identity_) {
    Identity* identity = reinterpret_cast<Identity*>(R_ExternalPtrAddr(identity_));
    if (identity) {
        delete identity;
        R_ClearExternalPtr(identity_);
    }
}

// Simpler interface 
//
// [[Rcpp::export]]
SEXP authenticate_Impl(SEXP con_, SEXP uuid_, SEXP ip_address_) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    if (uuid_ == R_NilValue || ip_address_ == R_NilValue) {
        Rcpp::stop("Either uuid or ip_address was null.");
    }

    std::string uuid = Rcpp::as<std::string>(uuid_);
    std::string ip_address = Rcpp::as<std::string>(ip_address_);

    const std::string authsrv = "//blp/apiauth";
    if (!session->openService(authsrv.c_str())) {
        Rcpp::stop("Failed to open " + authsrv);
    }

    Service apiAuthSvc = session->getService(authsrv.c_str());
    Request authorizationRequest = apiAuthSvc.createAuthorizationRequest();
    authorizationRequest.set("uuid", uuid.c_str());
    authorizationRequest.set("ipAddress", ip_address.c_str());
    Identity* identity_p = new Identity(session->createIdentity());
    session->sendAuthorizationRequest(authorizationRequest, identity_p);

    while (true) {
        Event event = session->nextEvent();
        MessageIterator msgIter(event);

        switch (event.eventType()) {
        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            msgIter.next();
            if (std::strcmp(msgIter.message().asElement().name().string(),
                            "AuthorizationSuccess")!=0) {
                Rcpp::stop("Authorization request failed.\n");
            }
        default:
            while (msgIter.next()) {
                Message msg = msgIter.message();
                //FIXME:: capture error msg here
            }
        }
        if (event.eventType() == Event::RESPONSE) { break; }
    }
    return createExternalPointer<Identity>(identity_p,identityFinalizer,"blpapi::Identity*");
}
