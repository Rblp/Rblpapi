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
#include <map>
#include <Rcpp.h>

typedef std::map<std::string,SEXP> LazyFrameT;
typedef std::map<std::string,SEXP>::iterator LazyFrameIteratorT;

void* checkExternalPointer(SEXP xp_, const char* valid_tag);
const int bbgDateToJulianDate(const BloombergLP::blpapi::Datetime& bbg_date);
const double bbgDateToPOSIX(const BloombergLP::blpapi::Datetime& bbg_date);
const double bbgDatetimeToPOSIX(const BloombergLP::blpapi::Datetime& dt);
const double bbgDatetimeToUTC(const BloombergLP::blpapi::Datetime& dt);
void appendOptionsToRequest(BloombergLP::blpapi::Request& request, SEXP options_);
void appendOverridesToRequest(BloombergLP::blpapi::Request& request, SEXP overrides_);
void createStandardRequest(BloombergLP::blpapi::Request& request,const std::vector<std::string>& securities,const std::vector<std::string>& fields,SEXP options_,SEXP overrides_);
void sendRequestWithIdentity(BloombergLP::blpapi::Session* session, BloombergLP::blpapi::Request& request, SEXP identity_);

void populateDfRow(SEXP ans, R_len_t row_index, BloombergLP::blpapi::Element& e);
SEXP allocateDataFrameColumn(int fieldT, size_t n);
SEXP buildDataFrame(LazyFrameT& m, bool add_fake_rownames = false, bool date_column_first = false);
SEXP buildDataFrame(std::vector<std::string>& rownames, LazyFrameT& m);
LazyFrameIteratorT assertColumnDefined(LazyFrameT& lazy_frame, BloombergLP::blpapi::Element& e, size_t n);
void addDateClass(SEXP x);
void addPosixClass(SEXP x);
void setNames(SEXP x, std::vector<std::string>& names);
void addFakeRownames(SEXP x, R_len_t n);

Rcpp::NumericVector createPOSIXtVector(const std::vector<double> & ticks, const std::string tz="UTC");
std::string vectorToCSVString(const std::vector<std::string>& vec);

#endif // BLPAPI_UTILS_H
