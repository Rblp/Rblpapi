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
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

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
using std::cout;
using std::endl;

// global logger
static std::ofstream logger;

static void identityFinalizer(SEXP identity_) {
  blpapi::Identity* identity = reinterpret_cast<blpapi::Identity*>(R_ExternalPtrAddr(identity_));
  if(identity) {
    delete identity;
    R_ClearExternalPtr(identity_);
  }
}

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
  if(std::strcmp(xp_tag,valid_tag) != 0) {
    throw std::logic_error("External pointer tag does not match.");
  }
  if(R_ExternalPtrAddr(xp_)==NULL) {
    throw std::logic_error("External pointer address is null.");
  }
  return R_ExternalPtrAddr(xp_);
}

const double bbgDateToPOSIX(const blpapi::Datetime& bbg_date) {
  boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
  struct tm tm_time(to_tm(bbg_boost_date));
  return static_cast<double>(mktime(&tm_time));
}

// caller guarantees options_ not null
void appendOptionsToRequest(blpapi::Request& request, SEXP options_) {
  Rcpp::CharacterVector options(options_);
  Rcpp::CharacterVector options_names(options.attr("names"));

  if(options.length() && options_names.length()==0) {
    throw std::logic_error("Request options must be named.");
  }

  for(R_len_t i = 0; i < options.length(); i++) {
    request.set(static_cast<std::string>(options_names[i]).c_str(), static_cast<std::string>(options[i]).c_str());
  }
}

std::string getSecurity(blpapi::Event& event) {
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

Rcpp::List refDataElementToList(blpapi::Element e) {
  Rcpp::List ans;
  Element field_data = e.getElement("fieldData");

  for(size_t i = 0; i < field_data.numElements(); ++i) {
    Element this_e = field_data.getElement(i);
    std::string field_name(this_e.name().string());

    switch(this_e.datatype()) {
    case BLPAPI_DATATYPE_BOOL:
      ans.push_back(this_e.getValueAsBool(),field_name); break;
    case BLPAPI_DATATYPE_CHAR:
      ans.push_back(this_e.getValueAsString(),field_name); break;
    case BLPAPI_DATATYPE_BYTE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTE.");
      break;
    case BLPAPI_DATATYPE_INT32:
      ans.push_back(this_e.getValueAsInt32(),field_name); break;
    case BLPAPI_DATATYPE_INT64:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_INT64.");
      break;
    case BLPAPI_DATATYPE_FLOAT32:
      ans.push_back(this_e.getValueAsFloat32(),field_name); break;
    case BLPAPI_DATATYPE_FLOAT64:
      ans.push_back(this_e.getValueAsFloat64(),field_name); break;
    case BLPAPI_DATATYPE_STRING:
      ans.push_back(this_e.getValueAsString(),field_name); break;
    case BLPAPI_DATATYPE_BYTEARRAY:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTEARRAY.");
    case BLPAPI_DATATYPE_DATE:      
    case BLPAPI_DATATYPE_TIME:
      //FIXME: separate out time later
      ans.push_back(bbgDateToPOSIX(this_e.getValueAsDatetime()),field_name); break;
    case BLPAPI_DATATYPE_DECIMAL:
      ans.push_back(this_e.getValueAsFloat64(),field_name); break;
    case BLPAPI_DATATYPE_DATETIME:
      ans.push_back(bbgDateToPOSIX(this_e.getValueAsDatetime()),field_name); break;
    case BLPAPI_DATATYPE_ENUMERATION:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_ENUMERATION.");
    case BLPAPI_DATATYPE_SEQUENCE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_SEQUENCE.");
    case BLPAPI_DATATYPE_CHOICE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_CHOICE.");
    case BLPAPI_DATATYPE_CORRELATION_ID:
      ans.push_back(this_e.getValueAsInt32(),field_name); break;
    default:
      throw std::logic_error("Unsupported datatype outside of api blpapi_DataType_t scope.");
    }
  }
  return ans;
}


Rcpp::List responseToList(blpapi::Event& event, const std::vector<std::string>& fields) {
  Rcpp::List ans;
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
  for(size_t i = 0; i < securityData.numValues(); ++i) {
    Element this_security = securityData.getValueAsElement(i);
    ans.push_back(refDataElementToList(this_security),this_security.getElementAsString("security"));
  }
  return ans;
}

Rcpp::DataFrame HistoricalDataResponseToDF(blpapi::Event& event, const std::vector<std::string>& fields) {
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
    blpapi::Datetime bbg_date =  this_fld.getElementAsDatetime("date");
    dts[i] = bbgDateToPOSIX(bbg_date);
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

extern "C" SEXP bdp_authenticate(SEXP conn_, SEXP uuid_, SEXP ip_address_) {
  blpapi::Session* session;
  try {
    session = reinterpret_cast<blpapi::Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  if(uuid_ == R_NilValue || ip_address_ == R_NilValue) {
    REprintf("uuid or ip_address was null.");
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
        REprintf("Authorization request failed.");
        return R_NilValue;
      }
    default:
      while (msgIter.next()) {
        Message msg = msgIter.message();
        if(logger.is_open()) { msg.asElement().print(logger); }
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  if(logger.is_open()) { logger.flush(); }
  return createExternalPointer<blpapi::Identity>(identity_p,identityFinalizer,"blpapi::Identity*");
}

extern "C" SEXP bdp_connect(SEXP host_, SEXP port_, SEXP logfile_) {
  SEXP conn;
  std::string host(Rcpp::as<std::string>(host_));
  int port(Rcpp::as<int>(port_));

  SessionOptions sessionOptions;
  sessionOptions.setServerHost(host.c_str());
  sessionOptions.setServerPort(port);
  Session* sp = new Session(sessionOptions);

  if (!sp->start()) {
    REprintf("Failed to start session.\n");
    return R_NilValue;
  }

  if(logfile_ != R_NilValue && TYPEOF(logfile_)==STRSXP && CHAR(STRING_ELT(logfile_,0))) {
    logger.open(CHAR(STRING_ELT(logfile_,0)));
  }
  return createExternalPointer<blpapi::Session>(sp,sessionFinalizer,"blpapi::Session*");
}

extern "C" SEXP bdh(SEXP conn_, SEXP securities_, SEXP fields_, SEXP start_date_, SEXP end_date_, SEXP options_, SEXP identity_) {
  Rcpp::List ans;
  blpapi::Session* session;
  blpapi::Identity* ip;
  std::vector<std::string> fields_vec;

  try {
    session = reinterpret_cast<blpapi::Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  Rcpp::CharacterVector securities(securities_);
  Rcpp::CharacterVector fields(fields_);
  std::string start_date(Rcpp::as<std::string>(start_date_));
  std::string end_date;

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
      ip = reinterpret_cast<blpapi::Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
    } catch (std::exception& e) {
      REprintf(e.what());
      return R_NilValue;
    }
    session->sendRequest(request,*ip);
  } else {
    session->sendRequest(request);
  }

  while (true) {
    std::string security_name;
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
        if(logger.is_open()) { msg.asElement().print(logger); }
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  if(logger.is_open()) { logger.flush(); }
  return Rcpp::wrap(ans);
}


extern "C" SEXP bdp(SEXP conn_, SEXP securities_, SEXP fields_, SEXP options_, SEXP identity_) {
  Rcpp::List ans;
  blpapi::Session* session;
  blpapi::Identity* ip;
  std::vector<std::string> fields_vec;

  try {
    session = reinterpret_cast<blpapi::Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  Rcpp::CharacterVector securities(securities_);
  Rcpp::CharacterVector fields(fields_);

  if (!session->openService("//blp/refdata")) {
    REprintf("Failed to open //blp/refdata\n");
    return R_NilValue;
  }

  Service refDataService = session->getService("//blp/refdata");
  Request request = refDataService.createRequest("ReferenceDataRequest");

  for(R_len_t i = 0; i < securities.length(); i++) {
    request.getElement("securities").appendValue(static_cast<std::string>(securities[i]).c_str());
  }
  
  for(R_len_t i = 0; i < fields.length(); i++) {
    request.getElement("fields").appendValue(static_cast<std::string>(fields[i]).c_str());
    fields_vec.push_back(static_cast<std::string>(fields[i]));
  }

  if(options_ != R_NilValue) { appendOptionsToRequest(request,options_); }

  if(identity_ != R_NilValue) {
    try {
      ip = reinterpret_cast<blpapi::Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
    } catch (std::exception& e) {
      REprintf(e.what());
      return R_NilValue;
    }
    session->sendRequest(request,*ip);
  } else {
    session->sendRequest(request);
  }

  while (true) {
    std::string security_name;
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans = responseToList(event,fields_vec);
      break;
    default:
      MessageIterator msgIter(event);
      while (msgIter.next()) {
        Message msg = msgIter.message();
        if(logger.is_open()) { msg.asElement().print(logger); }
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  if(logger.is_open()) { logger.flush(); }
  return Rcpp::wrap(ans);
}
