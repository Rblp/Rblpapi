// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  authenticateWithApp.cpp -- Function to authenticate to bpipe with an application name
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
//
// Added 2019 to allow server authentication with an application name.
// Authorization by application is described on page 63 of the guide
// "Bloomberg API"

#include <string>
#include <blpapi_defs.h>
#include <blpapi_element.h>
#include <blpapi_eventdispatcher.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_session.h>
#include <blpapi_event.h>
#include <Rcpp.h>
#include <finalizers.h>
#include <blpapi_utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::EventQueue;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;
using BloombergLP::blpapi::CorrelationId;

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
SEXP authenticateWithApp_Impl(SEXP con_) {
    Identity* identity_p;

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
                if(msg.messageType() == "TokenGenerationSuccess") {
                    token = msg.getElementAsString("token");
                }
                else if(msg.messageType() == "TokenGenerationFailure") {
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
                if(event.eventType() == Event::RESPONSE ||
                   event.eventType() == Event::REQUEST_STATUS ||
                   event.eventType() == Event::PARTIAL_RESPONSE) {
                    MessageIterator msgIter(event);
                    while(msgIter.next()) {
                        Message msg = msgIter.message();
                        if(msg.messageType() == "AuthorizationSuccess") {
                            message_found = true;
                        }
                        else {
                            Rcpp::stop(">>> Failed to Authorize");
                        }
                    }
                }
                else if(event.eventType() == Event::TIMEOUT) {
                    Rcpp::stop("Timed out trying to authorize");
                }
            }
        }
        else {
            Rcpp::stop("Generated token was empty");
        }
    }
    return createExternalPointer<Identity>(identity_p,identityFinalizer,"blpapi::Identity*");
}

