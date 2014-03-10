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
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;

Rcpp::List bulkArrayToDf(Element& bulk_field) {
  if(bulk_field.numValues()==0) {
    return Rcpp::List();
  }

  // construct scratch space for dataframe based on first row of response
  // (since we don't know the colnames/types ahead of time)
  Rcpp::List ans(buildDataFrameFromRow(bulk_field.getValueAsElement(0),bulk_field.numValues()));

  std::map<std::string, R_len_t> fields_map;
  std::vector<std::string> fields(getNamesFromRow(bulk_field.getValueAsElement(0)));
  for(R_len_t i = 0; i < fields.size(); ++i) { fields_map[fields[i]] = i; }

  for(size_t i = 0; i < bulk_field.numValues(); ++i) {
    Element row_element = bulk_field.getValueAsElement(i);
    populateDfRow(ans, i, fields_map, row_element);
  }
  return ans;
}

Rcpp::List BulkDataResponseToDF(Event& event, std::string& requested_field) {
  MessageIterator msgIter(event);
  if(!msgIter.next()) {
    throw std::logic_error("Not a valid MessageIterator.");
  }

  Message msg = msgIter.message();
  Element response = msg.asElement();
  //response.print(std::cout);
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
    if(fieldData.hasElement(requested_field.c_str())) {
      Element this_field = fieldData.getElement(requested_field.c_str());
      //ans[i] = bulkArrayToDf(this_field,requested_field);
      ans[i] = bulkArrayToDf(this_field);
    } else {
      ans[i] = R_NilValue;
    }
  }
 
  ans.attr("names") = ans_names;
  return ans;
}

// only allow one field for bds in contrast to bdp
extern "C" SEXP bds(SEXP conn_, SEXP securities_, SEXP field_, SEXP options_, SEXP identity_) {
  Session* session;

  std::vector<std::string> securities(Rcpp::as<std::vector<std::string> >(securities_));
  std::string field(Rcpp::as<std::string>(field_));
  std::vector<std::string> field_types;

  try {
    session = reinterpret_cast<Session*>(checkExternalPointer(conn_,"blpapi::Session*"));
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
  request.getElement("fields").appendValue(field.c_str());
  appendOptionsToRequest(request,options_);

  try {
    sendRequestWithIdentity(session, request, identity_);
  } catch (std::exception& e) {
    REprintf(e.what());
    return R_NilValue;
  }

  Rcpp::List ans;

  while (true) {
    Event event = session->nextEvent();
    switch (event.eventType()) {
    case Event::RESPONSE:
    case Event::PARTIAL_RESPONSE:
      ans = BulkDataResponseToDF(event,field);
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
