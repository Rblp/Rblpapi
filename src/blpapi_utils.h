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

#ifndef BLPAPI_UTILS_H
#define BLPAPI_UTILS_H

#include <string>
#include <vector>
#include <Rcpp.h>

void* checkExternalPointer(SEXP xp_, const char* valid_tag);
const double bbgDateToPOSIX(const BloombergLP::blpapi::Datetime& bbg_date);
const double bbgDatetimeToPOSIX(const BloombergLP::blpapi::Datetime& dt);
void appendOptionsToRequest(BloombergLP::blpapi::Request& request, SEXP options_);
void populateDfRow(Rcpp::List& ans, R_len_t row_index, std::map<std::string,R_len_t>& fields_map, BloombergLP::blpapi::Element& e);
Rcpp::List buildDataFrame(const std::vector<int>& fieldTypes, size_t n);
Rcpp::List buildDataFrame(const std::vector<std::string>& fieldTypes, size_t n);
Rcpp::List buildDataFrame(const std::vector<std::string>& rownames,const std::vector<std::string>& colnames,const std::vector<std::string>& fieldTypes);
Rcpp::List buildDataFrame(const std::vector<std::string>& rownames,const std::vector<std::string>& colnames,const std::vector<int>& fieldTypes);
std::vector<std::string> generateRownames(size_t n);
void createStandardRequest(BloombergLP::blpapi::Request& request,const std::vector<std::string>& securities,const std::vector<std::string>& fields,SEXP options_);
void sendRequestWithIdentity(BloombergLP::blpapi::Session* session, BloombergLP::blpapi::Request& request, SEXP identity_);
std::vector<std::string> getNamesFromRow(const BloombergLP::blpapi::Element& row);
Rcpp::List buildDataFrameFromRow(const BloombergLP::blpapi::Element& row, size_t n);

#endif // BLPAPI_UTILS_H
