///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013  Whit Armstrong                                    //
//                                                                       //
// This program is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by  //
// the Free Software Foundation, either version 3 of the License, or     //
// (at your option) any later version.                                   //
//                                                                       //
// This program is distributed in the hope that it will be useful,       //
// but WITHOUT ANY WARRANTY; without even the implied warranty of        //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
// GNU General Public License for more details.                          //
//                                                                       //
// You should have received a copy of the GNU General Public License     //
// along with this program.  If not, see <http://www.gnu.org/licenses/>. //
///////////////////////////////////////////////////////////////////////////

#include <string>
#include <blpapi_session.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <Rcpp.h>
#include <finalizers.h>
#include <blpapi.utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

static void identityFinalizer(SEXP identity_) {
  Identity* identity = reinterpret_cast<Identity*>(R_ExternalPtrAddr(identity_));
  if(identity) {
    delete identity;
    R_ClearExternalPtr(identity_);
  }
}

extern "C" SEXP bdp_authenticate(SEXP conn_, SEXP uuid_, SEXP ip_address_) {
  Session* session;
  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  if(uuid_ == R_NilValue || ip_address_ == R_NilValue) {
    REprintf("uuid or ip_address was null.\n");
    return R_NilValue;
  }
  std::string uuid = Rcpp::as<std::string>(uuid_);
  std::string ip_address = Rcpp::as<std::string>(ip_address_);

  if(!session->openService("//blp/apiauth")) {
    REprintf("Failed to open //blp/apiauth\n");
    return R_NilValue;
  }

  Service apiAuthSvc = session->getService("//blp/apiauth");
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
      if(std::strcmp(msgIter.message().asElement().name().string(),"AuthorizationSuccess")!=0) {
        REprintf("Authorization request failed.\n");
        return R_NilValue;
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
