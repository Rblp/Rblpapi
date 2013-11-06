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
#include <get_field_types.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Identity;
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

Rcpp::List HistoricalDataResponseToDF(Event& event,
                                      const std::vector<std::string>& fields,
                                      const std::vector<std::string>& field_types) {
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

  // must build rownames as chars...
  std::vector<std::string> rownames(generateRownames(fieldData.numValues()));
  Rcpp::List ans = buildDataFrame(rownames,fields,field_types);
  std::map<std::string, R_len_t> fields_map;
  for(R_len_t i = 0; i < fields.size(); ++i) { fields_map[fields[i]] = i; }

  for(size_t i = 0; i < fieldData.numValues(); i++) {
    Element row_element = fieldData.getValueAsElement(i);
    populateDfRow(ans, i, fields_map, row_element);
  }
  return ans;
}

extern "C" SEXP bdh(SEXP conn_, SEXP securities_, SEXP fields_, SEXP start_date_, SEXP end_date_, SEXP options_, SEXP identity_) {
  Session* session;
  Identity* ip;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::vector<std::string> fields(Rcpp::as<std::vector<std::string> >(fields_));
  std::vector<std::string> field_types;
  std::string start_date(Rcpp::as<std::string>(start_date_));

  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  try {
    getFieldTypes(field_types, session, fields);
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

  for(R_len_t i = 0; i < securities.size(); i++) {
    request.getElement("securities").appendValue(securities[i].c_str());
  }

  for(R_len_t i = 0; i < fields.size(); i++) {
    request.getElement("fields").appendValue(fields[i].c_str());
  }

  if(options_ != R_NilValue) { appendOptionsToRequest(request,options_); }

  request.set("startDate", start_date.c_str());
  if(end_date_ != R_NilValue) {
    request.set("endDate", Rcpp::as<std::string>(end_date_).c_str());
  }

  if(identity_ != R_NilValue) {
    try {
      ip = reinterpret_cast<Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
    } catch (std::exception& e) {
      REprintf(e.what());
      return R_NilValue;
    }
    session->sendRequest(request,*ip);
  } else {
    session->sendRequest(request);
  }

  // historical request will always have dates, so prepend to types expected
  fields.insert(fields.begin(),"date");
  field_types.insert(field_types.begin(),"Datetime");

  Rcpp::List ans(securities.size());
  R_len_t i = 0;

  // capture names in case they come back out of order
  std::vector<std::string> ans_names;

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans_names.push_back(getSecurityName(event));
      ans[i++] = HistoricalDataResponseToDF(event,fields,field_types);
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

  ans.attr("names") = ans_names;
  return Rcpp::wrap(ans);
}
