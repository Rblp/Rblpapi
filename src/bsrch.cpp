//
//  bsrch.cpp -- "Bloomberg SRCH" query function for the BLP API
//
//  Copyright (C) 2015 - 2024  Whit Armstrong and Dirk Eddelbuettel and John Laing
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
using BloombergLP::blpapi::Name;

Rcpp::DataFrame processBsrchResponse(Event event, const bool verbose) {

    MessageIterator msgIter(event); 			// create message iterator
    if (!msgIter.next()) throw std::logic_error("Not a valid MessageIterator.");

    Message msg = msgIter.message(); 			// get message
    if (verbose) msg.print(Rcpp::Rcout);

    Element response = msg.asElement(); 		// view as element
    if (std::strncmp(response.name().string(), "GridResponse", std::strlen("GridResponse")) != 0)
        throw std::logic_error("Not a valid GridResponse.");

    // exrsvc provides a grid in the form of DataRecords
    // Extract the dimensions and key attributes before processing
    Element dataRecords = msg.getElement(Name{"DataRecords"});

    int numRows = msg.getElementAsInt64(Name{"NumOfRecords"});
    if (verbose) Rcpp::Rcout << numRows << " records returned" << std::endl;

    Element columnTitles = msg.getElement(Name{"ColumnTitles"});
    int numCols = columnTitles.numValues();
    if (verbose) Rcpp::Rcout << "Returned columns:" << std::endl;
    if (verbose) columnTitles.print(Rcpp::Rcout);

    std::vector<std::string> colnames(numCols);
    for (int i=0; i<numCols; i++) {
        colnames[i] = columnTitles.getValueAsString(i);
    }
    if (verbose) Rcpp::Rcout << "Columns converted to C++ vector" << std::endl;

    // Rcpp 'List' container of given number of columns
    Rcpp::List lst(numCols);
    Rcpp::LogicalVector chk(numCols, false);

    if (verbose) Rcpp::Rcout << numRows << " Rows expected" << std::endl;

    // Grid response structure
    // vvvvv
    // DataRecordArray
        // DataRecord
            // DataFieldArray
                // DataField --> value
    // TODO - this step could made simpler for exrsvc queries as they always
    // return values as a Choice
    for (int i = 0; i < min(numRows, 25); i++) { 		                 // look at up to 25 rows to infer types
        Element dataRecord = dataRecords.getValueAsElement(i); 	         // pick i-th element to infer DataRecord type
        Element dataFields = dataRecord.getElement(Name{"DataFields"});  // get data payload of first element

        for (int j=0; j< numCols; j++) { 			                     // loop over first data set, and infer types
            if (!chk(j)) {                                               // column has not been set yet
                Element dataField = dataFields.getValueAsElement(j);     // pick j-th column - cannot access value by name
                Element dataValue = dataField.getChoice();

                if (verbose) dataValue.print(Rcpp::Rcout);
                if (verbose) Rcpp::Rcout << dataValue.datatype() << std::endl;

                switch (dataValue.datatype()) {
                case BLPAPI_DATATYPE_STRING:
                    lst[j] = Rcpp::CharacterVector(numRows, R_NaString);
                    chk[j] = true;
                    break;
                case BLPAPI_DATATYPE_DATETIME:
                    lst[j] = Rcpp::DateVector(numRows);
                    chk[j] = true;
                    break;
                // TODO - how to implement this for ints
//                 case BLPAPI_DATATYPE_INT32:
//                 case BLPAPI_DATATYPE_INT64:
//                     lst[j] = Rcpp::NumericVector(numRows, NA_INTEGER);
//                     chk[j] = true;
//                     break;
                case BLPAPI_DATATYPE_FLOAT32:
                case BLPAPI_DATATYPE_FLOAT64:
                    lst[j] = Rcpp::NumericVector(numRows, NA_REAL);
                    chk[j] = true;
                    break;
                default:
                    lst[j] = Rcpp::CharacterVector(numRows, R_NaString);
                    chk[j] = true;
                }
            }
        }
    }

    // Process data into grid structure defined above
    for (int i = 0; i < numRows; i++) {

        Element dataRecord = dataRecords.getValueAsElement(i);
        Element dataFields = dataRecord.getElement(Name{"DataFields"});
        if (verbose) dataFields.print(Rcpp::Rcout);

        for (int j = 0; j < numCols; j++) {
            Element dataField = dataFields.getValueAsElement(j);
            Element dataValue = dataField.getChoice();

            if (verbose) dataValue.print(Rcpp::Rcout);

            if (dataValue.datatype() == BLPAPI_DATATYPE_STRING) {
                Rcpp::CharacterVector v = lst[j];
                std::string sValue = dataValue.getValueAsString();
                v[i] = sValue;
                lst[j] = v;
            } else if (dataValue.datatype() == BLPAPI_DATATYPE_FLOAT64) {
                Rcpp::NumericVector v = lst[j];
                std::double_t fValue = dataValue.getValueAsFloat64();
                v[i] = fValue;
                lst[j] = v;
            } else {
                Rcpp::CharacterVector v = lst[j];
                std::string sValue = dataValue.getValueAsString();
                v[i] = sValue;
                lst[j] = v;
            }
        }
    }

    lst.attr("names") = colnames;
    Rcpp::DataFrame df(lst);
    return df;
}


// [[Rcpp::export]]
DataFrame bsrch_Impl(SEXP con,
                    std::string domain,
                    std::string limit,
                    bool verbose=false) {

    Session* session = reinterpret_cast<Session*>(checkExternalPointer(con, "blpapi::Session*"));

    const std::string exrsrv = "//blp/exrsvc";
    if (!session->openService(exrsrv.c_str())) {
        Rcpp::stop("Failed to open " + exrsrv);
    }

    Service exrService = session->getService(exrsrv.c_str());
    Request request = exrService.createRequest("ExcelGetGridRequest");

    request.getElement(Name{"Domain"}).setValue(domain.c_str());

    // TODO - implement limit and other overrides

    if (verbose) Rcpp::Rcout <<"Sending Request: " << request << std::endl;
    session->sendRequest(request);

    DataFrame ans;

    // Wait for events from Session
    bool done = false;
    while (!done) {
        Event event = session->nextEvent();
        if (event.eventType() == Event::PARTIAL_RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Partial Response" << std::endl;
            ans = processBsrchResponse(event, verbose);
        } else if (event.eventType() == Event::RESPONSE) {
            if (verbose) Rcpp::Rcout << "Processing Response" << std::endl;
            ans = processBsrchResponse(event, verbose);
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

    return ans;

}
