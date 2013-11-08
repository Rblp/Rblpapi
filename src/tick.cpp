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

// to abide non c++11
const char* cnames[] = { "time", "type", "value", "size", "conditionCodes" };
const int ctypes[] = { BLPAPI_DATATYPE_DATETIME, BLPAPI_DATATYPE_ENUMERATION, BLPAPI_DATATYPE_FLOAT64, BLPAPI_DATATYPE_INT32, BLPAPI_DATATYPE_ENUMERATION };

Rcpp::List intradayTickDataToDF(Event& event) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }
  Element response(msgIter.message().asElement());
  if(std::strcmp(response.name().string(),"IntradayTickResponse")) {
    throw std::logic_error("Not a valid IntradayTickResponse.");
  }
  if(!response.hasElement("tickData")) {
    throw std::logic_error("Element has no tickData.");
  }

  Element tickData = response.getElement("tickData");

  if(!tickData.hasElement("tickData") || tickData.getElement("tickData").numValues() == 0) {
    throw std::logic_error("No data returned.");
  }

  Element tickDataArray = tickData.getElement("tickData");

  //FIXME: only append conditionCodes if it was a requested option
  std::vector<int> fieldTypes(ctypes, ctypes+5);
  std::vector<std::string> fields(cnames,cnames+5);
  std::vector<std::string> rownames(generateRownames(tickDataArray.numValues()));
  Rcpp::List ans(buildDataFrame(rownames,fields,fieldTypes));

  // build field map
  std::map<std::string, R_len_t> fields_map;
  for(R_len_t i = 0; i < fields.size(); ++i) { fields_map[fields[i]] = i; }

  for(size_t i = 0; i < tickDataArray.numValues(); ++i) {
    Element row_element = tickDataArray.getValueAsElement(i);
    populateDfRow(ans, i, fields_map, row_element);
  }
  return ans;
}


extern "C" SEXP tick(SEXP conn_, SEXP security_, SEXP event_types_, SEXP start_datetime_, SEXP end_datetime_, SEXP include_condition_codes_, SEXP options_, SEXP identity_) {
  Session* session;

  std::string security(Rcpp::as<std::string>(security_));
  std::vector<std::string> event_types(Rcpp::as<std::vector<std::string> >(event_types_));
  std::string start_datetime(Rcpp::as<std::string>(start_datetime_));
  std::string end_datetime(Rcpp::as<std::string>(end_datetime_));
  bool include_condition_codes(Rcpp::as<bool>(include_condition_codes_));

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
  Request request = refDataService.createRequest("IntradayTickRequest");
  request.set("security", security.c_str());
  Element eventTypes = request.getElement("eventTypes");
  for (size_t i = 0; i < event_types.size(); ++i) {
    eventTypes.appendValue(event_types[i].c_str());
  }
  request.set("startDateTime", start_datetime.c_str());
  request.set("endDateTime", end_datetime.c_str());
  request.set("includeConditionCodes",include_condition_codes);
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
        ans = intradayTickDataToDF(event);
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
