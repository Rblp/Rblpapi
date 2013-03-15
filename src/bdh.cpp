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

#include <stdexcept>
#include <iostream>
#include <string>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/local_time/local_time_types.hpp>

#include <blpapi_session.h>
#include <blpapi_eventdispatcher.h>

#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_name.h>
#include <blpapi_request.h>
#include <blpapi_subscriptionlist.h>
#include <blpapi_defs.h>
#include <blpapi_exception.h>

#include <Rcpp.h>
#include <finalizers.h>

using namespace BloombergLP;
using namespace blpapi;

static void sessionFinalizer(SEXP session_) {
  blpapi::Session* session = reinterpret_cast<blpapi::Session*>(R_ExternalPtrAddr(session_));
  if(session) {
    delete session;
    R_ClearExternalPtr(session_);
  }
}

void* checkExternalPointer(SEXP xp_, const char* valid_tag) {
  if(xp_ == R_NilValue) {
    throw std::logic_error("External pointer is NULL.");
  }
  if(TYPEOF(xp_) != EXTPTRSXP) {
    throw std::logic_error("Not an external pointer.");
  }

  if(R_ExternalPtrTag(xp_)==R_NilValue) {
    throw std::logic_error("External pointer tag is NULL.");
  }
  const char* xp_tag = CHAR(PRINTNAME(R_ExternalPtrTag(xp_)));
  if(!xp_tag) {
    throw std::logic_error("External pointer tag is blank.");
  }
  if(strcmp(xp_tag,valid_tag) != 0) {
    throw std::logic_error("External pointer tag does not match.");
  }
  if(R_ExternalPtrAddr(xp_)==NULL) {
    throw std::logic_error("External pointer address is null.");
  }
  return R_ExternalPtrAddr(xp_);
}


Rcpp::DataFrame HistoricalDataResponseToDF(blpapi::Event& event) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid HistoricalDataResponse.");
  }

  Message msg = msgIter.message();
  Element response = msg.asElement();
  std::cout << "name: " << response.name() << std::endl;
  std::cout << "response.datatype: " << response.datatype() << std::endl;
  std::cout << "response.numValues:" << response.numValues() << std::endl;
  std::cout << "response.numElements: " << response.numElements() << std::endl;
  std::cout << "response.isArray: " << response.isArray() << std::endl;

  Element securityData = response.getElement("securityData");
  std::cout << "name: " << securityData.name() << std::endl;
  std::cout << "securityData.datatype: " << securityData.datatype() << std::endl;
  std::cout << "securityData.numValues:" << securityData.numValues() << std::endl;
  std::cout << "securityData.numElements: " << securityData.numElements() << std::endl;
  std::cout << "securityData.isArray: " << securityData.isArray() << std::endl;

  Element fieldData = securityData.getElement("fieldData");
  std::cout << "name: " << fieldData.name() << std::endl;
  std::cout << "fieldData.datatype: " << fieldData.datatype() << std::endl;
  std::cout << "fieldData.numValues:" << fieldData.numValues() << std::endl;
  std::cout << "fieldData.numElements: " << fieldData.numElements() << std::endl;
  std::cout << "fieldData.isArray: " << fieldData.isArray() << std::endl;

  Rcpp::DatetimeVector dts(fieldData.numValues());
  Rcpp::NumericVector values(fieldData.numValues());
  for(size_t i = 0; i < fieldData.numValues(); i++) {
    Element this_fld = fieldData.getValueAsElement(i);
    blpapi::Datetime bbg_date =  this_fld.getElementAsDatetime("date");
    //std::cout << bbg_date.year() << "-" << bbg_date.month() << "-" << bbg_date.day() << std::endl;
    boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
    struct tm tm_time = to_tm(bbg_boost_date);
    dts[i] = static_cast<double>(mktime(&tm_time));
    values[i] = this_fld.getElement("PX_LAST").getValueAsFloat64();
  }
  return Rcpp::DataFrame::create( Rcpp::Named("asofdate")= dts, Rcpp::Named("values") = values);
}

extern "C" SEXP bdp_connect(SEXP host_, SEXP port_, SEXP log_level_) {
  SEXP conn;
  std::string host(Rcpp::as<std::string>(host_));
  int port(Rcpp::as<int>(port_));

  SessionOptions sessionOptions;
  sessionOptions.setServerHost(host.c_str());
  sessionOptions.setServerPort(port);

  std::cout << "Connecting to " <<  host << ":" << port << std::endl;
  Session* sp = new Session(sessionOptions);

  if (!sp->start()) {
    std::cerr << "Failed to start session." << std::endl;
    return R_NilValue;
  }
  return createExternalPointer<blpapi::Session>(sp,sessionFinalizer,"blpapi::Session*");
}

extern "C" SEXP bdh(SEXP conn_, SEXP securities_, SEXP fields_, SEXP start_date_, SEXP end_date_, SEXP options_) {
  blpapi::Session* session;
  try {
    session = reinterpret_cast<blpapi::Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return R_NilValue;
  }

  Rcpp::CharacterVector securities(securities_);
  Rcpp::CharacterVector fields(fields_);
  std::string start_date(Rcpp::as<std::string>(start_date_));
  std::string end_date;

  if (!session->openService("//blp/refdata")) {
    std::cerr << "Failed to open //blp/refdata" << std::endl;
    return R_NilValue;
  }

  Service refDataService = session->getService("//blp/refdata");
  Request request = refDataService.createRequest("HistoricalDataRequest");

  for(R_len_t i = 0; i < securities.length(); i++) {
    request.getElement("securities").appendValue(static_cast<std::string>(securities[i]).c_str());
  }
  
  for(R_len_t i = 0; i < fields.length(); i++) {
    request.getElement("fields").appendValue(static_cast<std::string>(fields[i]).c_str());
  }

  //request.set("periodicityAdjustment", "ACTUAL");
  //request.set("periodicitySelection", "MONTHLY");
  request.set("startDate", start_date.c_str());
  if(end_date_ != R_NilValue) {
    request.set("endDate", Rcpp::as<std::string>(end_date_).c_str());
  }

  std::cout << "Sending Request: " << request << std:: endl;
  session->sendRequest(request);

  Rcpp::List ans;

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans.push_back(HistoricalDataResponseToDF(event));
      break;
    default:
      MessageIterator msgIter(event);
      while (msgIter.next()) {
        Message msg = msgIter.message();
        msg.asElement().print(std::cout);
        std::cout << std::endl;
      }      
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  return Rcpp::wrap(ans);
}



