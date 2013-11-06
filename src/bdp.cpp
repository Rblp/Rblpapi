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
    Element fieldData = this_security.getElement("fieldData");
    populateDfRow(ans, rownames_map[this_security_name], colnames_map, fieldData);
  }
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
  createStandardRequest(request, securities, fields, options_);

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
