// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  bdp.cpp -- "Bloomberg Data Point" query function for the BLP API
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

void getBDPResult(Event& event, Rcpp::List& res, const std::vector<std::string>& securities, const std::vector<std::string>& colnames, const std::vector<RblpapiT>& rtypes, bool verbose) {
    MessageIterator msgIter(event);
    if (!msgIter.next()) {
        throw std::logic_error("Not a valid MessageIterator.");
    }
    Message msg = msgIter.message();
    Element response = msg.asElement();
    if (verbose) response.print(Rcpp::Rcout);
    if (std::strcmp(response.name().string(),"ReferenceDataResponse")) {
        throw std::logic_error("Not a valid ReferenceDataResponse.");
    }
    Element securityData = response.getElement("securityData");

    for (size_t i = 0; i < securityData.numValues(); ++i) {
        Element this_security = securityData.getValueAsElement(i);
        size_t row_index = this_security.getElement("sequenceNumber").getValueAsInt32();

        // check that the seqNum matches the order of the securities vector (it's a grave error to screw this up)
        if(securities[row_index].compare(this_security.getElementAsString("security"))!=0) {
            throw std::logic_error("mismatched Security sequence, please report a bug.");
        }
        Element fieldData = this_security.getElement("fieldData");
        for(size_t j = 0; j < fieldData.numElements(); ++j) {
            Element e = fieldData.getElement(j);
            auto col_iter = std::find(colnames.begin(), colnames.end(), e.name().string());
            if (col_iter == colnames.end()) {
                throw std::logic_error(std::string("column is not expected: ") + e.name().string());
            }
            size_t col_index = std::distance(colnames.begin(),col_iter);
            populateDfRow(res[col_index],row_index,e,rtypes[col_index]);
        }
    }
}

// Simpler interface with std::vector<std::string> thanks to Rcpp::Attributes
//
// [[Rcpp::export]]
Rcpp::List bdp_Impl(SEXP con_, std::vector<std::string> securities, std::vector<std::string> fields,
                    SEXP options_, SEXP overrides_, bool verbose, SEXP identity_) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

    // get the field info
    std::vector<FieldInfo> fldinfos(getFieldTypes(session, fields));
    std::vector<RblpapiT> rtypes;
    for(auto f : fldinfos) {
        rtypes.push_back(fieldInfoToRblpapiT(f.datatype,f.ftype));
        //std::cout << f.id << ":" << f.mnemonic << ":" << f.datatype << ":" << f.ftype << std::endl;
    }
    Rcpp::List res(allocateDataFrame(securities, fields, rtypes));

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }
    
    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("ReferenceDataRequest");
    createStandardRequest(request, securities, fields, options_, overrides_);
    sendRequestWithIdentity(session, request, identity_);

    while (true) {
        Event event = session->nextEvent();
        switch (event.eventType()) {
        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            getBDPResult(event, res, securities, fields, rtypes, verbose);
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
    return res;
}
