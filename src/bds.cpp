// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  bds.cpp -- "Bloomberg Data Set" query function for the BLP API
//
//  Copyright (C) 2013  Whit Armstrong
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel
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

SEXP bulkArrayToDf(Element& fieldData) {
    if(fieldData.numValues()==0) {
        return R_NilValue;
    }

    LazyFrameT lazy_frame;
    for(size_t i = 0; i < fieldData.numValues(); ++i) {
        Element row = fieldData.getValueAsElement(i);
        for(size_t j = 0; j < row.numElements(); ++j) {
            Element e = row.getElement(j);
            LazyFrameIteratorT iter = assertColumnDefined(lazy_frame,e,fieldData.numValues());
            populateDfRow(iter->second,i,e);
        }
    }
    return buildDataFrame(lazy_frame,true);
}

SEXP BulkDataResponseToDF(Event& event, std::string& requested_field) {
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

    SEXP ans = PROTECT(Rf_allocVector(VECSXP, securityData.numValues()));
    std::vector<std::string> ans_names(securityData.numValues());

    for(size_t i = 0; i < securityData.numValues(); ++i) {
        Element this_security = securityData.getValueAsElement(i);
        ans_names[i] = this_security.getElementAsString("security");
        Element fieldData = this_security.getElement("fieldData");
        if(!fieldData.hasElement(requested_field.c_str())) {
            SET_VECTOR_ELT(ans,i,R_NilValue);
        } else {
            Element this_field = fieldData.getElement(requested_field.c_str());
            SET_VECTOR_ELT(ans,i,bulkArrayToDf(this_field));
        }
    }
    
    //FIXME: setListNames(ans,ans_names);
    UNPROTECT(1);
    return ans;
}

// only allow one field for bds in contrast to bdp
// [[Rcpp::export]]
SEXP bds_Impl(SEXP con_, std::vector<std::string> securities, 
              std::string field, SEXP options_, SEXP overrides_, SEXP identity_) {

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

    SEXP ans = R_NilValue;      // to keep -pedantic happy
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

    return ans;
}
