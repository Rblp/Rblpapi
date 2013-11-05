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
void appendOptionsToRequest(BloombergLP::blpapi::Request& request, SEXP options_);
void populateDfRow(Rcpp::List& ans, R_len_t row_index, std::map<std::string,R_len_t>& fields_map, BloombergLP::blpapi::Element& e);
Rcpp::List buildDataFrame(std::vector<std::string>& rownames,std::vector<std::string>& colnames,std::vector<std::string> fieldTypes);

#endif // BLPAPI_UTILS_H
