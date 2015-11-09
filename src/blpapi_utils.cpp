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
//#include <iostream>
#include <algorithm>
#define BOOST_NO_AUTO_PTR
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/local_time/local_time_types.hpp>
#include <blpapi_session.h>
#include <blpapi_request.h>
#include <blpapi_datetime.h>
#include <Rcpp.h>
#include <blpapi_utils.h>

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

const int bbgDateToJulianDate(const Datetime& bbg_date) {
  const boost::gregorian::date r_epoch(1970,1,1);
  boost::gregorian::date bbg_boost_date(bbg_date.year(),bbg_date.month(),bbg_date.day());
  boost::gregorian::date_period dp(r_epoch,bbg_boost_date);
  return static_cast<int>(dp.length().days());
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

// In case data already comes in localtime (cf TZDF<GO>) 
// we need to adjust back to UTC 
const double bbgDatetimeToUTC(const BloombergLP::blpapi::Datetime& dt) {
  boost::gregorian::date bbg_boost_date(dt.year(),dt.month(),dt.day());
  boost::posix_time::time_duration td =
    boost::posix_time::hours(dt.hours()) +
    boost::posix_time::minutes(dt.minutes()) +
    boost::posix_time::seconds(dt.seconds()) +
    boost::posix_time::milliseconds(dt.milliseconds());
  boost::posix_time::ptime bbg_ptime(bbg_boost_date,td);

  // cf http://stackoverflow.com/a/4462309/143305
  boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
  boost::posix_time::time_duration::sec_type x = (bbg_ptime - epoch).total_seconds();
  return x;
}

void addDateClass(SEXP x) {
  // create and add dates class to dates object
  SEXP r_dates_class;
  PROTECT(r_dates_class = Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(r_dates_class, 0, Rf_mkChar("Date"));
  Rf_classgets(x, r_dates_class);
  UNPROTECT(1); //r_dates_class
}

void addPosixClass(SEXP x) {
  // create and add dates class to dates object
  SEXP r_posix_class;
  PROTECT(r_posix_class = Rf_allocVector(STRSXP, 2));
  SET_STRING_ELT(r_posix_class, 0, Rf_mkChar("POSIXct"));
  SET_STRING_ELT(r_posix_class, 1, Rf_mkChar("POSIXt"));
  Rf_classgets(x, r_posix_class);
  UNPROTECT(1); //r_posix_class
}

void populateDfRow(SEXP ans, R_len_t row_index, Element& e) {
  if(e.isNull()) { return; }

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
    INTEGER(ans)[row_index] = bbgDateToJulianDate(e.getValueAsDatetime()); break;
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
    ans = PROTECT(Rf_allocVector(LGLSXP,n));
    std::fill(LOGICAL(ans),LOGICAL(ans)+n,NA_LOGICAL);
    break;
  case BLPAPI_DATATYPE_CHAR:
    ans = PROTECT(Rf_allocVector(STRSXP,n));
    break;
  case BLPAPI_DATATYPE_BYTE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTE.");
    break;
  case BLPAPI_DATATYPE_INT32:
  case BLPAPI_DATATYPE_INT64:
    ans = PROTECT(Rf_allocVector(INTSXP, n));
    std::fill(INTEGER(ans),INTEGER(ans)+n,NA_INTEGER);
    break;
  case BLPAPI_DATATYPE_FLOAT32:
  case BLPAPI_DATATYPE_FLOAT64:
    ans = PROTECT(Rf_allocVector(REALSXP,n));
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_STRING:
    ans = PROTECT(Rf_allocVector(STRSXP,n)); break;
  case BLPAPI_DATATYPE_BYTEARRAY:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_BYTEARRAY.");
    break;
  case BLPAPI_DATATYPE_DATE:
    ans = PROTECT(Rf_allocVector(INTSXP, n));
    addDateClass(ans);
    std::fill(INTEGER(ans),INTEGER(ans)+n,NA_INTEGER);
    break;
  case BLPAPI_DATATYPE_TIME:
    //FIXME: separate out time later
    ans = PROTECT(Rf_allocVector(REALSXP,n));
    addPosixClass(ans);
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_DECIMAL:
    ans = PROTECT(Rf_allocVector(REALSXP,n));
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_DATETIME:
    ans = PROTECT(Rf_allocVector(REALSXP,n));
    addPosixClass(ans);
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case BLPAPI_DATATYPE_ENUMERATION:
    ans = PROTECT(Rf_allocVector(STRSXP,n));
    break;
  case BLPAPI_DATATYPE_SEQUENCE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_SEQUENCE.");
    break;
  case BLPAPI_DATATYPE_CHOICE:
    throw std::logic_error("Unsupported datatype: BLPAPI_DATATYPE_CHOICE.");
    break;
  case BLPAPI_DATATYPE_CORRELATION_ID:
    ans = PROTECT(Rf_allocVector(INTSXP, n));
    break;
  default:
    throw std::logic_error("Unsupported datatype outside of api blpapi_DataType_t scope.");
  }
  return ans;
}

void addFakeRownames(SEXP x, R_len_t n) {
  SEXP rownames = PROTECT(Rf_allocVector(INTSXP,n));
  for(R_len_t i = 0; i < n; ++i) INTEGER(rownames)[i] = i+1;
  Rf_setAttrib(x,Rf_install("row.names"),rownames);
  UNPROTECT(1); // rownames
}

SEXP buildDataFrame(LazyFrameT& m, bool add_fake_rownames, bool date_column_first) {
  if(m.empty()) { return R_NilValue; }
  SEXP ans = PROTECT(Rf_allocVector(VECSXP, m.size()));

  SEXP klass = PROTECT(Rf_allocVector(STRSXP, 1));
  SET_STRING_ELT(klass, 0, Rf_mkChar("data.frame"));
  Rf_classgets(ans,klass); UNPROTECT(1); // klass

  if(add_fake_rownames) {
    addFakeRownames(ans,Rf_length(m.begin()->second));
  }

  SEXP colnames = PROTECT(Rf_allocVector(STRSXP, m.size()));

  // reset date_column_first to false if 'date' column not present
  if(date_column_first && m.find(std::string("date"))==m.end()) {
    date_column_first = false;
  }

  int i(0);
  if(date_column_first) {
    SET_STRING_ELT(colnames,i,Rf_mkChar("date"));
    SET_VECTOR_ELT(ans,i,m["date"]);
    ++i;

    for (const auto &v : m) {
      if(v.first != "date") {
        SET_STRING_ELT(colnames,i,Rf_mkChar(v.first.c_str()));
        SET_VECTOR_ELT(ans,i,v.second);
        ++i;
      }
    }

  } else {
    for (const auto &v : m) {
      SET_STRING_ELT(colnames,i,Rf_mkChar(v.first.c_str()));
      SET_VECTOR_ELT(ans,i,v.second);
      ++i;
    }
  }
  Rf_setAttrib(ans, R_NamesSymbol, colnames); UNPROTECT(1); // colnames

  // all columns are now safe
  UNPROTECT(m.size());

  UNPROTECT(1); // ans
  return ans;
}

SEXP buildDataFrame(std::vector<std::string>& rownames, LazyFrameT& m) {
  SEXP ans = PROTECT(buildDataFrame(m,false));
  SEXP rownames_ = PROTECT(Rf_allocVector(STRSXP, rownames.size()));
  int j(0);
  for(const auto &v : rownames) { SET_STRING_ELT(rownames_,j++,Rf_mkChar(v.c_str())); }
  Rf_setAttrib(ans, Rf_install("row.names"), rownames_); UNPROTECT(1); // rownames_

  UNPROTECT(1); // ans
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

void appendOverridesToRequest(Request& request, SEXP overrides_) {
  if(overrides_ == R_NilValue) { return; }
  Rcpp::CharacterVector overrides(overrides_);

  if(!overrides.hasAttribute("names") || overrides.attr("names") == R_NilValue) {
    throw std::logic_error("Request overrides must be named.");
  }

  Rcpp::CharacterVector overrides_names(overrides.attr("names"));

  if(overrides.length() && overrides_names.length()==0) {
    throw std::logic_error("Request overrides must be non empty and named.");
  }

  Element request_overrides = request.getElement("overrides");
  for(R_len_t i = 0; i < overrides.length(); i++) {
    Element this_override = request_overrides.appendElement();
    this_override.setElement("fieldId", static_cast<std::string>(overrides_names[i]).c_str());
    this_override.setElement("value", static_cast<std::string>(overrides[i]).c_str());
  }
}

void createStandardRequest(Request& request,
                           const std::vector<std::string>& securities,
                           const std::vector<std::string>& fields,
                           SEXP options_,
                           SEXP overrides_) {

  for(size_t i = 0; i < securities.size(); i++) {
    request.getElement("securities").appendValue(securities[i].c_str());
  }

  for(size_t i = 0; i < fields.size(); i++) {
    request.getElement("fields").appendValue(fields[i].c_str());
  }

  if(options_ != R_NilValue) { appendOptionsToRequest(request,options_); }
  if(overrides_ != R_NilValue) { appendOverridesToRequest(request,overrides_); }
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

LazyFrameIteratorT assertColumnDefined(LazyFrameT& lazy_frame, BloombergLP::blpapi::Element& e, size_t n) {
  LazyFrameIteratorT iter = lazy_frame.find(e.name().string());

  // insert only if not present
  if(iter == lazy_frame.end()) {
    // allocateDataFrameColumn calls PROTECT when SEXP is allocated
    SEXP column = allocateDataFrameColumn(e.datatype(), n);
    iter = lazy_frame.insert(lazy_frame.begin(),std::pair<std::string,SEXP>(e.name().string(),column));
  }

  return iter;
}

void setNames(SEXP x, std::vector<std::string>& names) {
  SEXP names_ = PROTECT(Rf_allocVector(STRSXP, names.size()));
  for(size_t i = 0; i < names.size(); ++i) {
    SET_STRING_ELT(names_,i,Rf_mkChar(names[i].c_str()));
  }
  Rf_setAttrib(x, R_NamesSymbol, names_); UNPROTECT(1);
}

Rcpp::NumericVector createPOSIXtVector(const std::vector<double> & ticks, 
                                       const std::string tz) {
    Rcpp::NumericVector pt(ticks.begin(), ticks.end());
    pt.attr("class") = Rcpp::CharacterVector::create("POSIXct", "POSIXt");
    pt.attr("tzone") = tz;
    return pt;
}

std::string vectorToCSVString(const std::vector<std::string>& vec) {
  if(vec.empty()) {
    return std::string();
  } else {
    std::ostringstream oss;
    std::copy(vec.begin(), vec.end()-1,std::ostream_iterator<std::string>(oss, ","));
    oss << vec.back();
    return oss.str();
  }
}
