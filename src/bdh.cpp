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
#include <blpapi.utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

std::string getSecurity(Event& event) {
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

Rcpp::DataFrame HistoricalDataResponseToDF(Event& event, const std::vector<std::string>& fields) {
  std::vector<Rcpp::NumericVector*> nvps;

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

  // we always have dates
  Rcpp::DatetimeVector dts(fieldData.numValues());

  // must build rownames as chars...
  Rcpp::CharacterVector rownames(fieldData.numValues());
  for(R_len_t i = 0; i < fieldData.numValues(); ++i) {
    std::ostringstream convert; convert << (i+1); rownames[i] = convert.str();
  }

  // create scratch space for output arrays
  for(size_t i = 0; i < fields.size(); ++i) {
    nvps.push_back(new Rcpp::NumericVector(fieldData.numValues()));
  }

  for(size_t i = 0; i < fieldData.numValues(); i++) {
    Element this_fld = fieldData.getValueAsElement(i);
    dts[i] = bbgDateToPOSIX(this_fld.getElementAsDatetime("date"));
    // walk through fields and test whether this tuple contains the field
    for(size_t j = 0; j < fields.size(); ++j) {
      nvps[j]->operator[](i) = this_fld.hasElement(fields[j].c_str()) ? this_fld.getElement(fields[j].c_str()).getValueAsFloat64() : NA_REAL;
    }
  }

  Rcpp::DataFrame ans;
  ans.push_back(dts,"asofdate");
  for(R_len_t i = 0; i < fields.size(); ++i) {
    ans.push_back(*nvps[i],fields[i]);
  }

  ans.attr("class") = "data.frame";
  ans.attr("row.names") = rownames;
  return ans;
}

extern "C" SEXP bdh(SEXP conn_, SEXP securities_, SEXP fields_, SEXP start_date_, SEXP end_date_, SEXP options_, SEXP identity_) {
  Rcpp::List ans;
  Session* session;
  Identity* ip;
  std::vector<std::string> fields_vec;

  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  Rcpp::CharacterVector securities(securities_);
  Rcpp::CharacterVector fields(fields_);
  std::string start_date(Rcpp::as<std::string>(start_date_));

  if (!session->openService("//blp/refdata")) {
    REprintf("Failed to open //blp/refdata\n");
    return R_NilValue;
  }

  Service refDataService = session->getService("//blp/refdata");
  Request request = refDataService.createRequest("HistoricalDataRequest");

  for(R_len_t i = 0; i < securities.length(); i++) {
    request.getElement("securities").appendValue(static_cast<std::string>(securities[i]).c_str());
  }

  for(R_len_t i = 0; i < fields.length(); i++) {
    request.getElement("fields").appendValue(static_cast<std::string>(fields[i]).c_str());
    fields_vec.push_back(static_cast<std::string>(fields[i]));
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

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans[ getSecurity(event) ] = HistoricalDataResponseToDF(event,fields_vec);
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
  return Rcpp::wrap(ans);
}
