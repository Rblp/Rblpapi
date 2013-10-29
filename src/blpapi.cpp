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

void populateDfRow(Rcpp::List& ans, R_len_t row_index, std::map<std::string,R_len_t>& fields_map, blpapi::Element& e) {
  Element field_data = e.getElement("fieldData");

  for(size_t i = 0; i < field_data.numElements(); ++i) {
    Element this_e = field_data.getElement(i);
    std::map<std::string,R_len_t>::iterator iter = fields_map.find(this_e.name().string());
    if(iter == fields_map.end()) {
      throw std::logic_error(std::string("Unexpected field encountered in response:") + this_e.name().string());
    }
    R_len_t col_index = iter->second;
    switch(this_e.datatype()) {
    case BLPAPI_DATATYPE_BOOL:
      INTEGER(ans[col_index])[row_index] = this_e.getValueAsBool(); break;
    case BLPAPI_DATATYPE_CHAR:
      SET_STRING_ELT(ans[col_index],row_index,Rf_mkChar(this_e.getValueAsString())); break;
    case BLPAPI_DATATYPE_BYTE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTE.");
      break;
    case BLPAPI_DATATYPE_INT32:
      INTEGER(ans[col_index])[row_index] = this_e.getValueAsInt32(); break;
    case BLPAPI_DATATYPE_INT64:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_INT64.");
      break;
    case BLPAPI_DATATYPE_FLOAT32:
      REAL(ans[col_index])[row_index] = this_e.getValueAsFloat32(); break;
    case BLPAPI_DATATYPE_FLOAT64:
      REAL(ans[col_index])[row_index] = this_e.getValueAsFloat64(); break;
    case BLPAPI_DATATYPE_STRING:
      SET_STRING_ELT(ans[col_index],row_index,Rf_mkChar(this_e.getValueAsString())); break;
    case BLPAPI_DATATYPE_BYTEARRAY:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTEARRAY.");
    case BLPAPI_DATATYPE_DATE:
    case BLPAPI_DATATYPE_TIME:
      //FIXME: separate out time later
      REAL(ans[col_index])[row_index] = bbgDateToPOSIX(this_e.getValueAsDatetime()); break;
    case BLPAPI_DATATYPE_DECIMAL:
      REAL(ans[col_index])[row_index] = this_e.getValueAsFloat64(); break;
    case BLPAPI_DATATYPE_DATETIME:
      REAL(ans[col_index])[row_index] = bbgDateToPOSIX(this_e.getValueAsDatetime()); break;
    case BLPAPI_DATATYPE_ENUMERATION:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_ENUMERATION.");
    case BLPAPI_DATATYPE_SEQUENCE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_SEQUENCE.");
    case BLPAPI_DATATYPE_CHOICE:
      throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_CHOICE.");
    case BLPAPI_DATATYPE_CORRELATION_ID:
      INTEGER(ans[col_index])[row_index] = this_e.getValueAsInt32(); break;
    default:
      throw std::logic_error("Unsupported datatype outside of api blpapi_DataType_t scope.");
    }
  }
}


void populateDF(Rcpp::List& ans, blpapi::Event& event) {
  std::vector<std::string> rownames = Rcpp::as<std::vector<std::string> > (ans.attr("row.names"));
  std::vector<std::string> colnames = Rcpp::as<std::vector<std::string> > (ans.attr("names"));
  std::map<std::string, R_len_t> rownames_map; for(R_len_t i = 0; i < rownames.size(); ++i) { rownames_map[rownames[i]] = i; }
  std::map<std::string, R_len_t> colnames_map; for(R_len_t i = 0; i < colnames.size(); ++i) { colnames_map[colnames[i]] = i; }

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
    std::string this_security_name(this_security.getElementAsString("security"));
    populateDfRow(ans, rownames_map[this_security_name], colnames_map, this_security);
  }
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

Rcpp::List buildDataFrame(std::vector<std::string>& rownames,
                          std::vector<std::string>& colnames,
                          std::vector<std::string> fieldTypes) {

  if(colnames.size() != fieldTypes.size()) {
    throw std::logic_error("buildDataFrame: Colnames not the same length as fieldTypes.");
  }

  Rcpp::List ans(colnames.size());
  for(R_len_t i = 0; i < fieldTypes.size(); ++i) {
    if(fieldTypes[i] == "Double") {
      ans[i] = Rcpp::NumericVector(rownames.size(),NA_REAL);
    } else if(fieldTypes[i] == "String") {
      ans[i] = Rcpp::CharacterVector(rownames.size());
    } else if(fieldTypes[i] == "Datetime") {
      ans[i] = Rcpp::DatetimeVector(rownames.size());
    } else {
      throw std::logic_error(std::string("buildDataFrame: unexpected type encountered: ") + fieldTypes[i]); 
    }
  }

  ans.attr("class") = "data.frame";
  ans.attr("names") = colnames;
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

std::string getFieldType(blpapi::Session* session, blpapi::Service fieldInfoService, const std::string& field) {
  Request request = fieldInfoService.createRequest("FieldInfoRequest");
  request.append("id", field.c_str());
  request.set("returnFieldDocumentation", false);
  session->sendRequest(request);
  while (true) {
    Event event = session->nextEvent();
    if (event.eventType() != Event::RESPONSE &&
        event.eventType() != Event::PARTIAL_RESPONSE) {
      continue;
    }

    MessageIterator msgIter(event);
    while (msgIter.next()) {
      Message msg = msgIter.message();
      //msg.asElement().print(std::cout);
      Element fields = msg.getElement("fieldData");
      if(fields.numValues() > 1) {
        throw std::logic_error("Only one field requested.");
      }
      Element field = fields.getValueAsElement(0);
      if(field.hasElement("fieldError")) {
        std::ostringstream err;
        err << "Bad field: " << field.getElementAsString("id") << std::endl;
        throw std::logic_error(err.str());
      }
      if(!field.hasElement("fieldInfo") || !field.getElement("fieldInfo").hasElement("datatype")) {
        throw std::logic_error("Did not find datatype in fieldInfo request.");
      }
      return field.getElement("fieldInfo").getElementAsString("datatype");
    }
    if (event.eventType() == Event::RESPONSE) {
      break;
    }
  }
}

void getFieldTypes(std::vector<std::string>& ans, blpapi::Session* session, std::vector<std::string>& fields) {
  const std::string APIFLDS_SVC("//blp/apiflds");
  if (!session->openService(APIFLDS_SVC.c_str())) {
    throw std::logic_error(std::string("Failed to open " + APIFLDS_SVC));
  }
  Service fieldInfoService = session->getService(APIFLDS_SVC.c_str());
  for(R_len_t i = 0; i < fields.size(); i++) {
    ans.push_back(getFieldType(session,fieldInfoService,fields[i]));
  }
}

extern "C" SEXP bdp(SEXP conn_, SEXP securities_, SEXP fields_, SEXP options_, SEXP identity_) {
  blpapi::Session* session;
  blpapi::Identity* ip;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::vector<std::string> fields(Rcpp::as<std::vector<std::string> >(fields_));
  std::vector<std::string> field_types;

  try {
    session = reinterpret_cast<blpapi::Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
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
  Request request = refDataService.createRequest("ReferenceDataRequest");

  for(R_len_t i = 0; i < securities.size(); i++) {
    request.getElement("securities").appendValue(securities[i].c_str());
  }
  
  for(R_len_t i = 0; i < fields.size(); i++) {
    request.getElement("fields").appendValue(fields[i].c_str());
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

  Rcpp::List ans = buildDataFrame(securities,fields,field_types);

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      populateDF(ans,event);
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
