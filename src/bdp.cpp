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

void populateDfRow(Rcpp::List& ans, R_len_t row_index, std::map<std::string,R_len_t>& fields_map, Element& e) {
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


void populateDF(Rcpp::List& ans, Event& event) {
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

extern "C" SEXP bdp(SEXP conn_, SEXP securities_, SEXP fields_, SEXP options_, SEXP identity_) {
  Session* session;
  Identity* ip;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::vector<std::string> fields(Rcpp::as<std::vector<std::string> >(fields_));
  std::vector<std::string> field_types;

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
    REprintf("Failed to open //blp/refdata.\n");
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
      ip = reinterpret_cast<Identity*>(checkExternalPointer(identity_,"blpapi::Identity*"));
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
        //FIXME:: capture messages here for debugging
      }
    }
    if (event.eventType() == Event::RESPONSE) { break; }
  }
  return Rcpp::wrap(ans);
}
