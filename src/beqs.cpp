// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  beqs.cpp -- "Bloomberg EQS" query function for the BLP API
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


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <array>
#include <iterator>


#include <sstream>
using namespace std;
using namespace Rcpp;

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Element;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;



string ToString(size_t sz) {

    stringstream ss;

    ss << sz;

    return ss.str();
}

CharacterMatrix processResponseEvent(Event event,SEXP PiTDate_)
{
    std::string PiTDate;
    if(PiTDate_ == R_NilValue) {
        PiTDate  = "NA";
    } else {
        PiTDate = Rcpp::as<std::string>(PiTDate_).c_str();
    }

    MessageIterator msgIter(event);
    CharacterMatrix ans;

    while (msgIter.next()) {

        Message msg = msgIter.message();
        Element response = msg.asElement();

        if(std::strcmp(response.name().string(),"BeqsResponse")) {
            throw std::logic_error("Not a valid EQSDataResponse.");
        }

        Element data = msg.getElement("data");
        Element fieldDisplayUnits = data.getElement("fieldDisplayUnits");
        Element securities = data.getElement("securityData");

        size_t iCols = fieldDisplayUnits.numElements();
        size_t iRows = securities.numValues();

        CharacterMatrix m( iRows, iCols+1 );
        CharacterVector rownames(iRows);
        CharacterVector colnames(iCols+1);

        std::string * fieldsArray;
        fieldsArray = new std::string [iCols];

        // Map fields
        colnames[0]="Date";

        // Headers
        for (size_t j = 0; j < iCols; ++j) {
            fieldsArray[j] = fieldDisplayUnits.getElement(j).name().string();
            colnames[j+1] = fieldDisplayUnits.getElement(j).name().string();
        }

        for (size_t i = 0; i < iRows ; ++i) {

            Element security = securities.getValueAsElement(i);
            Element fieldData = security.getElement("fieldData");

            // Set the first column as Date
            if(std::strcmp(PiTDate.c_str(),"NA")==0) {
                m(i,0) = NA_STRING;
            } else {
                m(i,0) = PiTDate.c_str();
            }

            // Set Rownames
            rownames[i]=ToString(i+1);

            //   For each returned security, check if data is available
            for (size_t j = 0; j < iCols;++j){

                if (fieldData.hasElement(fieldsArray[j].c_str()))
                {
                    Element datapoint = fieldData.getElement(fieldsArray[j].c_str());

                    if ( fieldsArray[j] == "Ticker")
                    {
                        std::string TickerValue =datapoint.getValueAsString();
                        TickerValue += " Equity";
                        m(i,j+1) = TickerValue;
                    }
                    else
                    {
                        std::string sValue = datapoint.getValueAsString();
                        m(i,j+1) = sValue;
                    }

                }
                else
                {
                    //   Insert alternative value here as below
                    m(i,j+1) = NA_STRING;
                }
            }
        }

        List dimnms = List::create(rownames,colnames);
        m.attr("dimnames") = dimnms;
        ans=m;

    }
    return wrap(ans);
}



// Simpler interface with std::vector<std::string> thanks to Rcpp::Attributes
// [[Rcpp::export]]
SEXP beqs_Impl(SEXP con_,
              std::string screenName,
              std::string screenType_,
              SEXP Group_,
              SEXP PiTDate_,
              SEXP languageId_) {

    //Rprintf("=======BEQS============ \n");

    Session* session = reinterpret_cast<Session*>(checkExternalPointer(con_,"blpapi::Session*"));

    const std::string rdsrv = "//blp/refdata";
    if (!session->openService(rdsrv.c_str())) {
        Rcpp::stop("Failed to open " + rdsrv);
    }

    Service refDataService = session->getService(rdsrv.c_str());
    Request request = refDataService.createRequest("BeqsRequest");


    request.set("screenName", screenName.c_str());
    request.set("screenType",screenType_.c_str());

    if (Group_ != R_NilValue) {
        request.set ("Group", Rcpp::as<std::string>(Group_).c_str());
    }

    if (languageId_ != R_NilValue) {
        request.set ("languageId", Rcpp::as<std::string>(languageId_).c_str());
    }


    Element overrides = request.getElement("overrides");
    Element override1 = overrides.appendElement();
    override1.setElement("fieldId", "PiTDate");
    if(PiTDate_ != R_NilValue) {
        override1.setElement("value", Rcpp::as<std::string>(PiTDate_).c_str());
    }

    session->sendRequest(request);

    CharacterMatrix ans;

    // Wait for events from Session
    bool done = false;
    while (!done) {
        Event event = session->nextEvent();
        if (event.eventType() == Event::PARTIAL_RESPONSE) {
            ans = processResponseEvent(event,PiTDate_);
        }
        else if (event.eventType() == Event::RESPONSE) {
            ans = processResponseEvent(event,PiTDate_);
            done = true;
        } else {
            MessageIterator msgIter(event);
            while (msgIter.next()) {
                Message msg = msgIter.message();
                if (event.eventType() == Event::SESSION_STATUS) {
                    if (msg.messageType() == "SessionTerminated" ||
                        msg.messageType() == "SessionStartupFailure") {
                        done = true;
                    }
                }
            }
        }
    }


return wrap(ans);

}
