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

#include <Rcpp.h>
#include <blpapi_utils.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;
using BloombergLP::blpapi::Element;

// [[Rcpp::export]]
Rcpp::List fieldInfo_Impl(SEXP con_, std::vector<std::string> fields) {

  Session* session = 
    reinterpret_cast<Session*>(checkExternalPointer(con_, "blpapi::Session*"));

  // get the field info
  std::vector<FieldInfo> fldinfos(getFieldTypes(session, fields));
  std::vector<std::string> colnames {"id","mnemonic","datatype","ftype"};
  std::vector<RblpapiT> res_types(4,RblpapiT::String);
  Rcpp::List res(allocateDataFrame(fields, colnames, res_types));
  R_len_t i(0);
  for(auto f : fldinfos) {
    SET_STRING_ELT(res[0],i,Rf_mkCharCE(f.id.c_str(), CE_UTF8));
    SET_STRING_ELT(res[1],i,Rf_mkCharCE(f.mnemonic.c_str(), CE_UTF8));
    SET_STRING_ELT(res[2],i,Rf_mkCharCE(f.datatype.c_str(), CE_UTF8));
    SET_STRING_ELT(res[3],i,Rf_mkCharCE(f.ftype.c_str(), CE_UTF8));
    ++i;
  }
  return res;
}
