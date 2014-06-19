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

std::string getSecurityName(Event& event) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }

  Message msg = msgIter.message();
  Element response = msg.asElement();
  if(std::strcmp(response.name().string(),"HistoricalDataResponse")) {
    throw std::logic_error("Not a valid HistoricalDataResponse.");
  }

  Element securityData = response.getElement("securityData");
  std::string ans(securityData.getElementAsString("security"));
  return ans;
}

SEXP HistoricalDataResponseToDF(Event& event) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }

  Message msg = msgIter.message();
  Element response = msg.asElement();
  if(std::strcmp(response.name().string(),"HistoricalDataResponse")) {
    throw std::logic_error("Not a valid HistoricalDataResponse.");
  }
  Element securityData = response.getElement("securityData");
  Element fieldData = securityData.getElement("fieldData");

  LazyFrameT lazy_frame;

  for(size_t i = 0; i < fieldData.numValues(); i++) {
    Element row = fieldData.getValueAsElement(i);
    for(size_t j = 0; j < row.numElements(); ++j) {
      Element e = row.getElement(j);
      LazyFrameIteratorT iter = assertColumnDefined(lazy_frame,e,fieldData.numValues());
      populateDfRow(iter->second,i,e);
    }
  }
  return buildDataFrame(lazy_frame,true);
}

extern "C" SEXP bdh(SEXP conn_, SEXP securities_, SEXP fields_, SEXP start_date_, SEXP end_date_, SEXP options_, SEXP identity_) {
  Session* session;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::vector<std::string> fields(Rcpp::as<std::vector<std::string> >(fields_));
  std::string start_date(Rcpp::as<std::string>(start_date_));

  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  if(!session->openService("//blp/refdata")) {
    REprintf("Failed to open //blp/refdata\n");
    return R_NilValue;
  }

  Service refDataService = session->getService("//blp/refdata");
  Request request = refDataService.createRequest("HistoricalDataRequest");
  try {
    createStandardRequest(request, securities, fields, options_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  request.set("startDate", start_date.c_str());
  if(end_date_ != R_NilValue) {
    request.set("endDate", Rcpp::as<std::string>(end_date_).c_str());
  }

  try {
    sendRequestWithIdentity(session, request, identity_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  SEXP ans = PROTECT(Rf_allocVector(VECSXP, securities.size()));
  R_len_t i = 0;

  // capture names in case they come back out of order
  std::vector<std::string> ans_names;

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans_names.push_back(getSecurityName(event));
      SET_VECTOR_ELT(ans,i++,HistoricalDataResponseToDF(event));
      break;
    default:
      MessageIterator msgIter(event);
      while (msgIter.next()) {
        Message msg = msgIter.message();
        // FIXME:: capture msg here for logging
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }

  setNames(ans,ans_names);
  UNPROTECT(1);
  return ans;
}
