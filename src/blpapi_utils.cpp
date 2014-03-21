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
#include <algorithm>
#include <map>
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

const double bbgDateToJulianDate(const Datetime& bbg_date) {
  const boost::gregorian::date r_epoch(1970,1,1);
  boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
  boost::gregorian::date_period dp(r_epoch,bbg_boost_date);
  return static_cast<double>(dp.length().days());
}

const double bbgDateToPOSIX(const Datetime& bbg_date) {
  boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
  struct tm tm_time(to_tm(bbg_boost_date));
  return static_cast<double>(mktime(&tm_time));
}

const double bbgDatetimeToPOSIX(const Datetime& dt) {
  boost::gregorian::date bbg_boost_date(dt.year(),dt.month(),dt.day());
  boost::posix_time::time_duration td =
    boost::posix_time::hours(dt.hours()) +
    boost::posix_time::minutes(dt.minutes()) +
    boost::posix_time::seconds(dt.seconds()) +
    boost::posix_time::milliseconds(dt.milliseconds());

  boost::posix_time::ptime bbg_ptime(bbg_boost_date,td);
  struct tm tm_time(to_tm(bbg_ptime));
  return static_cast<double>(mktime(&tm_time));
}

void populateDfRow(SEXP ans, R_len_t row_index, Element& e) {
  switch(e.datatype()) {
  case BLPAPI_DATATYPE_BOOL:
    LOGICAL(ans)[row_index] = e.getValueAsBool(); break;
  case BLPAPI_DATATYPE_CHAR:
    SET_STRING_ELT(ans,row_index,Rf_mkChar(e.getValueAsString())); break;
  case BLPAPI_DATATYPE_BYTE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTE.");
    break;
  case BLPAPI_DATATYPE_INT32:
    INTEGER(ans)[row_index] = e.getValueAsInt32(); break;
  case BLPAPI_DATATYPE_INT64:
    if(static_cast<double>(e.getValueAsInt64()) > static_cast<double>(std::numeric_limits<int>::max())) {
      REprintf("BLPAPI_DATATYPE_INT64 exceeds max int value on this system (assigning std::numeric_limits<int>::max()).");
      INTEGER(ans)[row_index] = std::numeric_limits<int>::max();
    } else {
      INTEGER(ans)[row_index] = static_cast<int>(e.getValueAsInt64()); break;
    }
    break;
  case BLPAPI_DATATYPE_FLOAT32:
    REAL(ans)[row_index] = e.getValueAsFloat32(); break;
  case BLPAPI_DATATYPE_FLOAT64:
    REAL(ans)[row_index] = e.getValueAsFloat64(); break;
  case BLPAPI_DATATYPE_STRING:
    SET_STRING_ELT(ans,row_index,Rf_mkChar(e.getValueAsString())); break;
  case BLPAPI_DATATYPE_BYTEARRAY:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTEARRAY."); break;
  case BLPAPI_DATATYPE_DATE:
    REAL(ans)[row_index] = bbgDateToJulianDate(e.getValueAsDatetime()); break;
  case BLPAPI_DATATYPE_TIME:
    //FIXME: separate out time later
    REAL(ans)[row_index] = bbgDateToPOSIX(e.getValueAsDatetime()); break;
  case BLPAPI_DATATYPE_DECIMAL:
    REAL(ans)[row_index] = e.getValueAsFloat64(); break;
  case BLPAPI_DATATYPE_DATETIME:
    //REAL(ans)[row_index] = bbgDateToPOSIX(e.getValueAsDatetime()); break;
    REAL(ans)[row_index] = bbgDatetimeToPOSIX(e.getValueAsDatetime()); break;
  case BLPAPI_DATATYPE_ENUMERATION:
    //throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_ENUMERATION.");
    SET_STRING_ELT(ans,row_index,Rf_mkChar(e.getValueAsString())); break;
  case BLPAPI_DATATYPE_SEQUENCE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_SEQUENCE.");
  case BLPAPI_DATATYPE_CHOICE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_CHOICE.");
  case BLPAPI_DATATYPE_CORRELATION_ID:
    INTEGER(ans)[row_index] = e.getValueAsInt32(); break;
  default:
    throw std::logic_error("Unsupported datatype outside of api blpapi_DataType_t scope.");
  }
}

SEXP allocateDataFrameColumn(int fieldT, size_t n) {
  SEXP ans;

  switch(fieldT) {
  case BLPAPI_DATATYPE_BOOL:
    ans = Rcpp::LogicalVector(n); break;
  case BLPAPI_DATATYPE_CHAR:
    ans = Rcpp::CharacterVector(n); break;
  case BLPAPI_DATATYPE_BYTE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTE.");
    break;
  case BLPAPI_DATATYPE_INT32:
    ans = Rcpp::IntegerVector(n); break;
  case BLPAPI_DATATYPE_INT64:
    ans = Rcpp::IntegerVector(n); break;
    break;
  case BLPAPI_DATATYPE_FLOAT32:
    ans = Rcpp::NumericVector(n,NA_REAL); break;
  case BLPAPI_DATATYPE_FLOAT64:
    ans = Rcpp::NumericVector(n,NA_REAL); break;
  case BLPAPI_DATATYPE_STRING:
    ans = Rcpp::CharacterVector(n); break;
  case BLPAPI_DATATYPE_BYTEARRAY:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTEARRAY.");
  case BLPAPI_DATATYPE_DATE:
    ans = Rcpp::DateVector(n);
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_TIME:
    //FIXME: separate out time later
    ans = Rcpp::DatetimeVector(n);
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_DECIMAL:
    ans = Rcpp::NumericVector(n,NA_REAL); break;
  case BLPAPI_DATATYPE_DATETIME:
    ans = Rcpp::DatetimeVector(n);
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_ENUMERATION:
    //throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_ENUMERATION.");
    ans = Rcpp::CharacterVector(n); break;
  case BLPAPI_DATATYPE_SEQUENCE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_SEQUENCE.");
  case BLPAPI_DATATYPE_CHOICE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_CHOICE.");
  case BLPAPI_DATATYPE_CORRELATION_ID:
    ans = Rcpp::IntegerVector(n); break;
  default:
    throw std::logic_error("Unsupported datatype outside of api blpapi_DataType_t scope.");
  }
  return ans;
}

SEXP fakeRownames(size_t n) {
  SEXP ans = Rcpp::IntegerVector(n);
  for(size_t i = 0; i < n; ++i) INTEGER(ans)[i] = i+1;
  return ans;
}
/*
Rcpp::List buildDataFrame(std::map<std::string,SEXP>& m) {
  Rcpp::List ans(m.size());
  ans.attr("class") = "data.frame";
  std::vector<std::string> colnames;
  int i(0);
  for (const auto &v : m) { colnames.push_back(v.first); ans[i++] = v.second; }
  ans.attr("names") = colnames;
  ans.attr("row.names") = fakeRownames(Rf_length(m.begin()->second));
  return ans;
}
*/

SEXP buildDataFrame(std::vector<std::string>& rownames, std::map<std::string,SEXP>& m) {
  if(m.empty()) { return R_NilValue; }
  SEXP ans = PROTECT(Rf_allocVector(VECSXP, m.size()));

  SEXP klass = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(klass, 0, Rf_mkChar("data.frame"));
  Rf_classgets(ans,klass); UNPROTECT(1);

  SEXP colnames = PROTECT(Rf_allocVector(STRSXP, m.size()));
  int i(0);
  for (const auto &v : m) {
    SET_STRING_ELT(colnames,i,Rf_mkChar(v.first.c_str()));
    SET_VECTOR_ELT(ans,i,v.second);
    ++i;
  }
  Rf_setAttrib(ans, R_NamesSymbol, colnames); UNPROTECT(1);

  SEXP rownames_ = PROTECT(Rf_allocVector(STRSXP, rownames.size()));
  int j(0);
  for(const auto &v : rownames) { SET_STRING_ELT(rownames_,j++,Rf_mkChar(v.c_str())); }
  Rf_setAttrib(ans, Rf_install("row.names"), rownames_); UNPROTECT(1);

  // all columns are now safe
  UNPROTECT(m.size());
  UNPROTECT(1);
  return ans;
}

void appendOptionsToRequest(Request& request, SEXP options_) {
  if(options_== R_NilValue) { return; }
  Rcpp::CharacterVector options(options_);

  if(!options.hasAttribute("names")) {
    throw std::logic_error("Request options must be named.");
  }

  if(options.attr("names") == R_NilValue) {
    throw std::logic_error("Request optionnames must not be null.");
  }

  Rcpp::CharacterVector options_names(options.attr("names"));

  if(options.length() && options_names.length()==0) {
    throw std::logic_error("Request options must be non empty and named.");
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
