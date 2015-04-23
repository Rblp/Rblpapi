// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  bdp.cpp -- "Bloomberg Data Point" query function for the BLP API
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


// compare to RefDataExample.cpp


#include <iostream>
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

void getBDPResult(Event& event, LazyFrameT& lazy_frame, std::vector<std::string>& securities) {
    MessageIterator msgIter(event);
    if (!msgIter.next()) {
        throw std::logic_error("Not a valid MessageIterator.");
    }
    Message msg = msgIter.message();
    Element response = msg.asElement();
    if (std::strcmp(response.name().string(),"ReferenceDataResponse")) {
        throw std::logic_error("Not a valid ReferenceDataResponse.");
    }
    Element securityData = response.getElement("securityData");

    // i is the row index
    //REprintf("securityData.numValues(): %d\n",securityData.numValues());
    for (size_t i = 0; i < securityData.numValues(); ++i) {
        //REprintf("i %d\n",i);
        Element this_security = securityData.getValueAsElement(i);
        // row index is position in securities vector
        auto row_iter = std::find(securities.begin(), securities.end(), 
                                  this_security.getElementAsString("security"));
        if (row_iter == securities.end()) {
            throw std::logic_error(std::string("this security is not expected: ") + 
                                   this_security.getElementAsString("security"));
        }
        size_t row_index = std::distance(securities.begin(),row_iter);
        Element fieldData = this_security.getElement("fieldData");
        //REprintf("fieldData.numElements(): %d\n",fieldData.numElements());
        for(size_t j = 0; j < fieldData.numElements(); ++j) {
            Element e = fieldData.getElement(j);
            LazyFrameIteratorT iter = assertColumnDefined(lazy_frame,e,securities.size());
            populateDfRow(iter->second,row_index,e);
        }
    }
}

// Simpler interface with std::vector<std::string> thanks to Rcpp::Attributes
//
// [[Rcpp::export]]
SEXP bdp_Impl(SEXP con_, std::vector<std::string> securities, std::vector<std::string> fields, 
              SEXP options_, SEXP overrides_, SEXP identity_) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }
    
    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("ReferenceDataRequest");
    createStandardRequest(request, securities, fields, options_, overrides_);
    sendRequestWithIdentity(session, request, identity_);

    LazyFrameT lazy_frame;

    while (true) {
        Event event = session->nextEvent();
        //REprintf("%d\n",event.eventType());
        switch (event.eventType()) {
        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            getBDPResult(event, lazy_frame, securities);
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
    return buildDataFrame(securities,lazy_frame);
}
