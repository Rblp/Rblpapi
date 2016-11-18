// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  bdh.cpp -- "Bloomberg Data History" query function for the BLP API
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

std::string getSecurityName(Event& event) {
    MessageIterator msgIter(event);
    if (!msgIter.next()) {
        throw std::logic_error("Not a valid MessageIterator.");
    }

    Message msg = msgIter.message();
    Element response = msg.asElement();
    if(std::strcmp(response.name().string(),"HistoricalDataResponse")) {
        throw std::logic_error("Not a valid HistoricalDataResponse.");
    }

    Element securityData = response.getElement("securityData");
    std::string ans(securityData.getElementAsString("security"));
    return ans;
}

Rcpp::List HistoricalDataResponseToDF(Event& event, const std::vector<std::string>& fields,
                                      const std::vector<RblpapiT>& rtypes, bool verbose=FALSE) {
    MessageIterator msgIter(event);
    if (!msgIter.next()) {
        throw std::logic_error("Not a valid MessageIterator.");
    }

    Message msg = msgIter.message();
    Element response = msg.asElement();
    if (verbose) response.print(Rcpp::Rcout);
    if (std::strcmp(response.name().string(),"HistoricalDataResponse")) {
        throw std::logic_error("Not a valid HistoricalDataResponse.");
    }
    Element securityData = response.getElement("securityData");
    Element fieldData = securityData.getElement("fieldData");

    Rcpp::List res(allocateDataFrame(fieldData.numValues(), fields, rtypes));

    // i is the row iterator
    for(size_t i = 0; i < fieldData.numValues(); i++) {
        Element row = fieldData.getValueAsElement(i);
        for(size_t j = 0; j < row.numElements(); ++j) {
            Element e = row.getElement(j);
            auto it = std::find(fields.begin(),fields.end(),e.name().string());
            if(it==fields.end()) { throw std::logic_error("Unexpected field returned."); }
            int colindex = std::distance(fields.begin(),it);
            populateDfRow(res[colindex], i, e, rtypes[colindex]);
        }
    }
    return res;
}

// Simpler interface with std::vector<std::string> thanks to Rcpp::Attributes
// [[Rcpp::export]]
Rcpp::List bdh_Impl(SEXP con_,
                    std::vector<std::string> securities,
                    std::vector<std::string> fields,
                    std::string start_date_, SEXP end_date_,
                    SEXP options_, SEXP overrides_,
                    bool verbose, SEXP identity_,
                    bool int_as_double) {

    Session* session = 
        reinterpret_cast<Session*>(checkExternalPointer(con_,"blpapi::Session*"));


    // get the field info
    std::vector<FieldInfo> fldinfos(getFieldTypes(session, fields));
    std::vector<RblpapiT> rtypes;
    for(auto f : fldinfos) {
        rtypes.push_back(fieldInfoToRblpapiT(f.datatype,f.ftype));
    }

    // for bdh request all int fields as doubles b/c of implicit bbg conversion
    if (int_as_double) {
      std::transform(rtypes.begin(), rtypes.end(), rtypes.begin(),
                     [](RblpapiT x) { return x == RblpapiT::Integer || x == RblpapiT::Integer64 ? RblpapiT::Double : x; });
    }

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }

    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("HistoricalDataRequest");
    createStandardRequest(request, securities, fields, options_, overrides_);

    request.set("startDate", start_date_.c_str());
    if (end_date_ != R_NilValue) {
        request.set("endDate", Rcpp::as<std::string>(end_date_).c_str());
    }

    sendRequestWithIdentity(session, request, identity_);

    Rcpp::List ans(securities.size());
    R_len_t i = 0;

    // capture names in case they come back out of order
    std::vector<std::string> ans_names;

    // the date field was not part of the request
    // but we need to map it into the result
    // we always want it in the first position
    fields.insert(fields.begin(),"date");
    rtypes.insert(rtypes.begin(),RblpapiT::Date);

    while (true) {
        Event event = session->nextEvent();
        switch (event.eventType()) {
        case Event::RESPONSE:
        case Event::PARTIAL_RESPONSE:
            ans_names.push_back(getSecurityName(event));
            ans[i++] = HistoricalDataResponseToDF(event, fields, rtypes, verbose);
            break;
        default:
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                // FIXME:: capture msg here for logging
            }
        }
        if (event.eventType() == Event::RESPONSE) { break; }
    }
    ans.attr("names") = ans_names;
    return ans;
}
