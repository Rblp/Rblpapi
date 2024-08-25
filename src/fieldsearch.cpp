//
//  fieldsearch.cpp -- a simple field search function for the BLP API
//
//  Copyright (C) 2013         Whit Armstrong
//  Copyright (C) 2014 - 2024  Whit Armstrong and Dirk Eddelbuettel
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

//  Derived from SimpleFieldSearchExample.cpp in the blpapi examples.
//  It contained the following header

/* Copyright 2012. Bloomberg Finance L.P.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:  The above
 * copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */


#include <blpapi_session.h>
#include <blpapi_utils.h>

namespace bbg = BloombergLP::blpapi;	// shortcut to not globally import both namespace

namespace {
    const bbg::Name FIELD_ID("id");
    const bbg::Name FIELD_MNEMONIC("mnemonic");
    const bbg::Name FIELD_DATA("fieldData");
    const bbg::Name FIELD_DESC("description");
    const bbg::Name FIELD_INFO("fieldInfo");
    const bbg::Name FIELD_ERROR("fieldError");
    const bbg::Name FIELD_MSG("message");
}

// [[Rcpp::export]]
Rcpp::DataFrame fieldSearch_Impl(SEXP con, std::string searchterm) {

    // via Rcpp Attributes we get a try/catch block with error propagation to R "for free"
    bbg::Session* session =
        reinterpret_cast<bbg::Session*>(checkExternalPointer(con,"blpapi::Session*"));

    const char *APIFLDS_SVC = "//blp/apiflds";
    if (!session->openService(APIFLDS_SVC)) {
        Rcpp::stop("Failed to open service");
    }

    bbg::Service fieldInfoService = session->getService(APIFLDS_SVC);
    bbg::Request request = fieldInfoService.createRequest("FieldSearchRequest");
    request.set(bbg::Name{"searchSpec"}, searchterm.c_str());
    request.set(bbg::Name{"returnFieldDocumentation"}, false);
    session->sendRequest(request);

    std::vector<std::string> fieldId, fieldMnen, fieldDesc;
    while (true) {
        bbg::Event event = session->nextEvent();

        if (event.eventType() != bbg::Event::RESPONSE &&
            event.eventType() != bbg::Event::PARTIAL_RESPONSE) {
            continue;
        }

        bbg::MessageIterator msgIter(event);
        while (msgIter.next()) {
            bbg::Message msg = msgIter.message();
            bbg::Element fields = msg.getElement(bbg::Name{"fieldData"});
            int numElements = fields.numValues();
            //Rprintf("Seeing %d elements\n", numElements);
            for (int i=0; i < numElements; i++) {
                const bbg::Element fld = fields.getValueAsElement(i);
                std::string  fldId = fld.getElementAsString(FIELD_ID);
                if (fld.hasElement(FIELD_INFO)) {
                    bbg::Element fldInfo     = fld.getElement (FIELD_INFO) ;
                    std::string  fldMnemonic = fldInfo.getElementAsString(FIELD_MNEMONIC);
                    std::string  fldDesc     = fldInfo.getElementAsString(FIELD_DESC);
                    fieldId.push_back(fldId);
                    fieldMnen.push_back(fldMnemonic);
                    fieldDesc.push_back(fldDesc);
                }
                else {
                    bbg::Element fldError = fld.getElement(FIELD_ERROR) ;
                    std::string  errorMsg = fldError.getElementAsString(FIELD_MSG) ;
                    Rcpp::stop(errorMsg);
                }
            }

        }
        if (event.eventType() == bbg::Event::RESPONSE) {
            break;
        }
    }
    return Rcpp::DataFrame::create(Rcpp::Named("Id")          = fieldId,
                                   Rcpp::Named("Mnemonic")    = fieldMnen,
                                   Rcpp::Named("Description") = fieldDesc);
}
