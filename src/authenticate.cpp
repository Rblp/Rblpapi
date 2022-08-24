// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  authenticate.cpp -- Function to authenticate to Bloomberg backend
//
//  Copyright (C) 2013  Whit Armstrong
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettelp
//  Copyright (C) 2019  Whit Armstrong, Dirk Eddelbuettel and Alfred Kanzler
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
#include <blpapi_defs.h>
#include <blpapi_element.h>
#include <blpapi_eventdispatcher.h>
#include <blpapi_exception.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
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
using BloombergLP::blpapi::CorrelationId;
using BloombergLP::blpapi::EventQueue;

static void identityFinalizer(SEXP identity_) {
    Identity* identity = reinterpret_cast<Identity*>(R_ExternalPtrAddr(identity_));
    if (identity) {
        delete identity;
        R_ClearExternalPtr(identity_);
    }
}

Identity* authenticateWithId(SEXP con_, SEXP uuid_, SEXP ip_address_, SEXP isEmrsId_) {
    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session =
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    if (uuid_ == R_NilValue || ip_address_ == R_NilValue) {
        Rcpp::stop("Either uuid or ip_address was null.");
    }

    std::string uuid = Rcpp::as<std::string>(uuid_);
    std::string ip_address = Rcpp::as<std::string>(ip_address_);
    bool isEmrsId = Rcpp::as<bool>(isEmrsId_);

    const std::string authsrv = "//blp/apiauth";
    if (!session->openService(authsrv.c_str())) {
        Rcpp::stop("Failed to open " + authsrv);
    }

    Service apiAuthSvc = session->getService(authsrv.c_str());
    Request authorizationRequest = apiAuthSvc.createAuthorizationRequest();
    // if isEmrsId is true, we are actually assuming a BPIPE scenario
    // where we still want to auth by user id & ip address
    if (isEmrsId) {
        authorizationRequest.set("emrsId", uuid.c_str());
    } else {
        authorizationRequest.set("uuid", uuid.c_str());
    }
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
    return identity_p;
}

Identity* authenticateWithApp(SEXP con_) {
    Identity* identity_p = 0;

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session =
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    // setup authorization service
    std::string service("//blp/apiauth");

    // authorize
    if (session->openService(service.c_str())) {
        Service authService = session->getService(service.c_str());
        CorrelationId correlation_id(10);
        std::string token;
        EventQueue tokenEventQueue;
        session->generateToken(correlation_id, &tokenEventQueue);
        Event event = tokenEventQueue.nextEvent();
        //
        // get token for session
        //
        if(event.eventType() == Event::TOKEN_STATUS ||
           event.eventType() == Event::REQUEST_STATUS) {
            MessageIterator msgIter(event);
            while(msgIter.next()) {
                Message msg = msgIter.message();
                if (msg.messageType() == "TokenGenerationSuccess") {
                    token = msg.getElementAsString("token");
                } else if(msg.messageType() == "TokenGenerationFailure") {
                    Rcpp::stop("Failed to generate token");
                }
            }
        }

        //
        // begin authorization
        //
        if(!token.empty()) {
            Request authRequest = authService.createAuthorizationRequest();
            authRequest.set("token", token.c_str());
            identity_p = new Identity(session->createIdentity());
            session->sendAuthorizationRequest(authRequest, identity_p);
            // parse messages
            bool message_found = false;
            while(!message_found) {
                Event event = session->nextEvent(100000);
                if (event.eventType() == Event::RESPONSE ||
                   event.eventType() == Event::REQUEST_STATUS ||
                   event.eventType() == Event::PARTIAL_RESPONSE) {
                    MessageIterator msgIter(event);
                    while (msgIter.next()) {
                        Message msg = msgIter.message();
                        if (msg.messageType() == "AuthorizationSuccess") {
                            message_found = true;
                        } else {
                            Rcpp::stop(">>> Failed to Authorize");
                        }
                    }
                } else if(event.eventType() == Event::TIMEOUT) {
                    Rcpp::stop("Timed out trying to authorize");
                }
            }
        } else {
            Rcpp::stop("Generated token was empty");
        }
    }
    return identity_p;
}

// Simpler interface
//
// [[Rcpp::export]]
SEXP authenticate_Impl(SEXP con_, SEXP uuid_, SEXP ip_address_, SEXP isEmrsId_) {
    Identity* identity_p = NULL;
    if (uuid_ == R_NilValue) {
        identity_p = authenticateWithApp(con_);
    } else {
        identity_p = authenticateWithId(con_, uuid_, ip_address_, isEmrsId_);
    }
    if(identity_p == NULL) { Rcpp::stop("Identity pointer is null\n"); }
    return createExternalPointer<Identity>(identity_p, identityFinalizer, "blpapi::Identity*");
}
