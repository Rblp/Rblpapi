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

#include <iostream>
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

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

SEXP getBDPResult(Event& event) {
  LazyFrameT lazy_frame;
  std::vector<std::string> rownames;
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }
  Message msg = msgIter.message();
  Element response = msg.asElement();
    if(std::strcmp(response.name().string(),"ReferenceDataResponse")) {
    throw std::logic_error("Not a valid ReferenceDataResponse.");
  }
  Element securityData = response.getElement("securityData");

  // i is the row index
  //REprintf("securityData.numValues(): %d\n",securityData.numValues());
  for(size_t i = 0; i < securityData.numValues(); ++i) {
    //REprintf("i %d\n",i);
    Element this_security = securityData.getValueAsElement(i);
    rownames.push_back(this_security.getElementAsString("security"));
    Element fieldData = this_security.getElement("fieldData");
    //REprintf("fieldData.numElements(): %d\n",fieldData.numElements());
    for(size_t j = 0; j < fieldData.numElements(); ++j) {
      //REprintf("j %d\n",j);
      Element e = fieldData.getElement(j);
      std::map<std::string,SEXP>::iterator iter = lazy_frame.find(e.name().string());
      // insert only if not present
      if(iter == lazy_frame.end()) {
        iter = lazy_frame.insert(lazy_frame.begin(),std::pair<std::string,SEXP>(e.name().string(),PROTECT(allocateDataFrameColumn(e.datatype(), securityData.numValues()))));
      }
      populateDfRow(iter->second,i,e);
    }
  }
  return buildDataFrame(rownames,lazy_frame);
}

extern "C" SEXP bdp(SEXP conn_, SEXP securities_, SEXP fields_, SEXP options_, SEXP identity_) {
  Session* session;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::vector<std::string> fields(Rcpp::as<std::vector<std::string> >(fields_));
  std::vector<std::string> field_types;

  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  if(!session->openService("//blp/refdata")) {
    REprintf("Failed to open //blp/refdata.\n");
    return R_NilValue;
  }

  Service refDataService = session->getService("//blp/refdata");
  Request request = refDataService.createRequest("ReferenceDataRequest");
  try {
    createStandardRequest(request, securities, fields, options_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  try {
    sendRequestWithIdentity(session, request, identity_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  SEXP ans= R_NilValue;

  while (true) {
    Event event = session->nextEvent();
    //REprintf("%d\n",event.eventType());
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans = getBDPResult(event);
      break;
    default:
      MessageIterator msgIter(event);
      while (msgIter.next()) {
        Message msg = msgIter.message();
        //FIXME:: capture messages here for debugging
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  return ans;
}
