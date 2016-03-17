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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <blpapi_session.h>
#include <blpapi_service.h>
#include <blpapi_request.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <Rcpp.h>
#include <Rblpapi_types.h>

void* checkExternalPointer(SEXP xp_, const char* valid_tag);
const int bbgDateToRDate(const BloombergLP::blpapi::Datetime& bbg_date);
const int bbgDateToRDate(const double yyyymmdd_date);
const double bbgDateToPOSIX(const BloombergLP::blpapi::Datetime& bbg_date);
const double bbgDatetimeToPOSIX(const BloombergLP::blpapi::Datetime& dt);
const double bbgDatetimeToUTC(const BloombergLP::blpapi::Datetime& dt);
void appendOptionsToRequest(BloombergLP::blpapi::Request& request, SEXP options_);
void appendOverridesToRequest(BloombergLP::blpapi::Request& request, SEXP overrides_);
void createStandardRequest(BloombergLP::blpapi::Request& request,const std::vector<std::string>& securities,const std::vector<std::string>& fields,SEXP options_,SEXP overrides_);
void sendRequestWithIdentity(BloombergLP::blpapi::Session* session, BloombergLP::blpapi::Request& request, SEXP identity_);

void populateDfRow(SEXP ans, R_len_t row_index, const BloombergLP::blpapi::Element& e, RblpapiT rblpapitype);
void addDateClass(SEXP x);
void addPosixClass(SEXP x);

Rcpp::NumericVector createPOSIXtVector(const std::vector<double> & ticks, const std::string tz="UTC");
std::string vectorToCSVString(const std::vector<std::string>& vec);

RblpapiT fieldInfoToRblpapiT(const std::string& datatype, const std::string& ftype);
SEXP allocateDataFrameColumn(RblpapiT rblpapitype, const size_t n);
FieldInfo getFieldType(BloombergLP::blpapi::Session *session, BloombergLP::blpapi::Service& fieldInfoService, const std::string& field);
std::vector<FieldInfo> getFieldTypes(BloombergLP::blpapi::Session *session,const std::vector<std::string> &fields);
Rcpp::List allocateDataFrame(const std::vector<std::string>& rownames, const std::vector<std::string>& colnames, std::vector<RblpapiT>& coltypes);
Rcpp::List allocateDataFrame(size_t nrows, const std::vector<std::string>& colnames, const std::vector<RblpapiT>& coltypes);
