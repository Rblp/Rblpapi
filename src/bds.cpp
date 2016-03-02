// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  bds.cpp -- "Bloomberg Data Set" query function for the BLP API
//
//  Copyright (C) 2013         Whit Armstrong
//  Copyright (C) 2015 - 2016  Whit Armstrong and Dirk Eddelbuettel
//
//  This file is part of Rblpapi
//
//  Rblpapi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  Rblpapi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

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

void populateDfRowBDS(SEXP ans, R_len_t row_index, Element& e) {
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
    INTEGER(ans)[row_index] = bbgDateToRDate(e.getValueAsDatetime()); break;
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

// deprecated -- still neede by BDS
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


Rcpp::List buildDataFrame(LazyFrameT& m) {
    Rcpp::List ans(m.size());
    ans.attr("class") = "data.frame";
    if(m.empty()) { return ans; }

    size_t nrows(Rf_length(m.begin()->second));
    Rcpp::IntegerVector rnms(nrows); std::iota(rnms.begin(), rnms.end(), 1);
    ans.attr("row.names") = rnms;

    R_len_t i(0);
    std::vector<std::string> colnames(m.size());
    for(const auto &v : m) {
        colnames[i] = v.first;
        ans[i] = v.second;
        ++i;
    }
    ans.attr("names") = colnames;
    // all columns are now safe
    UNPROTECT(m.size());

    return ans;
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

Rcpp::List bulkArrayToDf(Element& fieldData) {
    if(fieldData.numValues()==0) {
        return R_NilValue;
    }
    LazyFrameT lazy_frame;
    for(size_t i = 0; i < fieldData.numValues(); ++i) {
        Element row = fieldData.getValueAsElement(i);
        for(size_t j = 0; j < row.numElements(); ++j) {
            Element e = row.getElement(j);
            LazyFrameIteratorT iter = assertColumnDefined(lazy_frame,e,fieldData.numValues());
            populateDfRowBDS(iter->second,i,e);
        }
    }
    return buildDataFrame(lazy_frame);
}

Rcpp::List BulkDataResponseToDF(Event& event, std::string& requested_field, bool verbose) {
    MessageIterator msgIter(event);
    if(!msgIter.next()) {
        throw std::logic_error("Not a valid MessageIterator.");
    }

    Message msg = msgIter.message();
    Element response = msg.asElement();
    if (verbose) response.print(Rcpp::Rcout);
    if(std::strcmp(response.name().string(),"ReferenceDataResponse")) {
        throw std::logic_error("Not a valid ReferenceDataResponse.");
    }
    Element securityData = response.getElement("securityData");

    Rcpp::List ans(securityData.numValues());
    std::vector<std::string> ans_names(securityData.numValues());

    for(size_t i = 0; i < securityData.numValues(); ++i) {
        Element this_security = securityData.getValueAsElement(i);
        ans_names[i] = this_security.getElementAsString("security");
        Element fieldData = this_security.getElement("fieldData");
        if(!fieldData.hasElement(requested_field.c_str())) {
            ans[i] = R_NilValue;
        } else {
            Element e = fieldData.getElement(requested_field.c_str());
            ans[i] = bulkArrayToDf(e);
        }
    }
    ans.attr("names") = ans_names;
    return ans;
}

// only allow one field for bds in contrast to bdp
// [[Rcpp::export]]
Rcpp::List bds_Impl(SEXP con_, std::vector<std::string> securities, 
                    std::string field, SEXP options_, SEXP overrides_,
                    bool verbose, SEXP identity_) {

    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    std::vector<std::string> field_types;

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }

    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("ReferenceDataRequest");
    for (size_t i = 0; i < securities.size(); i++) {
        request.getElement("securities").appendValue(securities[i].c_str());
    }
    request.getElement("fields").appendValue(field.c_str());
    appendOptionsToRequest(request,options_);
    appendOverridesToRequest(request,overrides_);
    
    sendRequestWithIdentity(session, request, identity_);

    while (true) {
        Event event = session->nextEvent();
        switch (event.eventType()) {
        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            return BulkDataResponseToDF(event, field, verbose);
            break;
        default:
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                //FIXME:: capture messages here for debugging
            }
        }
        if (event.eventType() == Event::RESPONSE) { break; }
    }
    return R_NilValue;
}
