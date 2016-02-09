///////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016  Whit Armstrong                                    //
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
#include <blpapi_session.h>
#include <blpapi_request.h>
#include <Rcpp.h>


class FieldInfo {
public:
  std::string id;
  std::string mnemonic;
  std::string datatype;
  std::string ftype;
};


// Bloomberg types
enum class DatatypeT {Bool,Datetime,Double,Int32,Int64,String};

// mapping string->DatatypeT
const std::map<std::string, DatatypeT> stringToDatatypeT {
  {"Bool",DatatypeT::Bool},{"Datetime",DatatypeT::Datetime},{"Double",DatatypeT::Double},{"Int32",DatatypeT::Int32},{"Int64",DatatypeT::Int64},{"String",DatatypeT::String}
};

// our types
enum class RblpapiT {Integer,Double,Logical,Date,Datetime,String,Unknown};


RblpapiT fieldInfoToRblpapiT(const std::string& datatype, const std::string& ftype);
SEXP allocateDataFrameColumn(RblpapiT rblpapitype, const size_t n);
FieldInfo getFieldType(BloombergLP::blpapi::Session *session, BloombergLP::blpapi::Service& fieldInfoService, const std::string& field);
std::vector<FieldInfo> getFieldTypes(BloombergLP::blpapi::Session *session,const std::vector<std::string> &fields);
Rcpp::List allocateDataFrame(const std::vector<std::string>& rownames, const std::vector<std::string>& colnames, std::vector<RblpapiT>& coltypes);
Rcpp::List allocateDataFrame(size_t nrows, const std::vector<std::string>& colnames, const std::vector<RblpapiT>& coltypes);
