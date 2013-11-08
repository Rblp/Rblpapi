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

Rcpp::List intradayBarDataToDF(Event& event) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }
  Element response(msgIter.message().asElement());
  if(std::strcmp(response.name().string(),"IntradayBarResponse")) {
    throw std::logic_error("Not a valid IntradayBarResponse.");
  }
  if(!response.hasElement("barData")) {
    throw std::logic_error("Element has no barData.");
  }

  Element barData = response.getElement("barData");
  //barData.print(std::cout);
  if(!barData.hasElement("barTickData") || barData.getElement("barTickData").numValues() == 0) {
    throw std::logic_error("No data returned.");
  }

  Element barTickData = barData.getElement("barTickData");

  // construct scratch space for dataframe based on first row of response
  // (since we don't know the colnames/types ahead of time)
  Rcpp::List ans(buildDataFrameFromRow(barTickData.getValueAsElement(0),barTickData.numValues()));

  // build field map
  std::map<std::string, R_len_t> fields_map;
  std::vector<std::string> fields(getNamesFromRow(barTickData.getValueAsElement(0)));
  for(R_len_t i = 0; i < fields.size(); ++i) { fields_map[fields[i]] = i; }

  for(size_t i = 0; i < barTickData.numValues(); ++i) {
    Element row_element = barTickData.getValueAsElement(i);
    populateDfRow(ans, i, fields_map, row_element);
  }
  return ans;
}

extern "C" SEXP bar(SEXP conn_, SEXP security_, SEXP event_type_, SEXP interval_, SEXP start_datetime_, SEXP end_datetime_, SEXP options_, SEXP identity_) {
  Session* session;

  std::string security(Rcpp::as<std::string>(security_));
  std::string event_type(Rcpp::as<std::string>(event_type_));
  int interval(Rcpp::as<int>(interval_));
  std::string start_datetime(Rcpp::as<std::string>(start_datetime_));
  std::string end_datetime(Rcpp::as<std::string>(end_datetime_));

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
  Request request = refDataService.createRequest("IntradayBarRequest");
  request.set("security", security.c_str());
  request.set("eventType", event_type.c_str());
  request.set("interval", interval);
  request.set("startDateTime", start_datetime.c_str());
  request.set("endDateTime", end_datetime.c_str());
  appendOptionsToRequest(request, options_);

  try {
    sendRequestWithIdentity(session, request, identity_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  Rcpp::List ans;

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      try {
        ans = intradayBarDataToDF(event);
      } catch (std::exception& e) {
        REprintf(e.what());
      }
      break;
    default:
      MessageIterator msgIter(event);
      while (msgIter.next()) {
        Message msg = msgIter.message();
        //msg.asElement().print(std::cout);
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  return Rcpp::wrap(ans);
}
