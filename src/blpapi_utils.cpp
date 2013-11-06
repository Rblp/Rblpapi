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
#include <string>
#include <sstream>
#include <iostream>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/local_time/local_time_types.hpp>
#include <blpapi_session.h>
#include <blpapi_request.h>
#include <blpapi_datetime.h>
#include <Rcpp.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Identity;
using BloombergLP::blpapi::Datetime;
using BloombergLP::blpapi::Element;

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

const double bbgDateToPOSIX(const Datetime& bbg_date) {
  boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
  struct tm tm_time(to_tm(bbg_boost_date));
  return static_cast<double>(mktime(&tm_time));
}

void populateDfRow(Rcpp::List& ans, R_len_t row_index, std::map<std::string,R_len_t>& fields_map, Element& field_data) {

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

Rcpp::List buildDataFrame(const std::vector<std::string>& rownames,
                          const std::vector<std::string>& colnames,
                          const std::vector<std::string>& fieldTypes) {

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

std::vector<std::string> generateRownames(size_t n) {
  std::vector<std::string> ans(n);
  for(size_t i = 0; i < n; ++i) {
    std::ostringstream convert;
    convert << (i+1);
    ans[i] = convert.str();
  }
  return ans;
}

// caller guarantees options_ not null
void appendOptionsToRequest(Request& request, SEXP options_) {
  Rcpp::CharacterVector options(options_);
  Rcpp::CharacterVector options_names(options.attr("names"));

  if(options.length() && options_names.length()==0) {
    throw std::logic_error("Request options must be named.");
  }

  for(R_len_t i = 0; i < options.length(); i++) {
    request.set(static_cast<std::string>(options_names[i]).c_str(), static_cast<std::string>(options[i]).c_str());
  }
}

void createStandardRequest(Request& request,
                           const std::vector<std::string>& securities,
                           const std::vector<std::string>& fields,
                           SEXP options_) {

  for(R_len_t i = 0; i < securities.size(); i++) {
    request.getElement("securities").appendValue(securities[i].c_str());
  }

  for(R_len_t i = 0; i < fields.size(); i++) {
    request.getElement("fields").appendValue(fields[i].c_str());
  }

  if(options_ != R_NilValue) { appendOptionsToRequest(request,options_); }
}

void sendRequestWithIdentity(Session* session, Request& request, SEXP identity_) {
  Identity* ip;
  if(identity_ != R_NilValue) {
    ip = reinterpret_cast<Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
    session->sendRequest(request,*ip);
  } else {
    session->sendRequest(request);
  }
}
