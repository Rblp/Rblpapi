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
#include <getFieldInfo.h>
#include <Rcpp.h>
#include <blpapi_utils.h>

using std::vector;
using std::string;
using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;
using BloombergLP::blpapi::Element;

// map to RblpapiT using datatype and ftype
// both are needed b/c datatype does not distinguish between date and datetime
RblpapiT fieldInfoToRblpapiT(const std::string& datatype, const std::string& ftype) {
  auto iter = stringToDatatypeT.find(datatype);
  if(iter == stringToDatatypeT.end()) {
    std::ostringstream err;
    err << "datatype not found: " << datatype;
    // No throw, try to be graceful
  }

  switch(iter->second) {
  case DatatypeT::Bool:
    return RblpapiT::Logical;
    break;
  case DatatypeT::String:
    return RblpapiT::String;
    break;
  case DatatypeT::Int32:
  case DatatypeT::Int64:
    return RblpapiT::Integer;
    break;
  case DatatypeT::Double:
    return RblpapiT::Double;
    break;
  case DatatypeT::Datetime:
    if(ftype=="Date") {
      return RblpapiT::Date;
    } else if(ftype=="Time") {
      return RblpapiT::Datetime;
    }
    break;
  default:
    // just try to return it as a string
    return RblpapiT::String;
    break;
  }
  // never gets here
  return RblpapiT::String;
}

SEXP allocateDataFrameColumn(RblpapiT rblpapitype, const size_t n) {
  SEXP ans;
  switch(rblpapitype) {
  case RblpapiT::Logical:
    ans = PROTECT(Rf_allocVector(LGLSXP,n));
    std::fill(LOGICAL(ans),LOGICAL(ans)+n,NA_LOGICAL);
    break;
  case RblpapiT::Integer:
    ans = PROTECT(Rf_allocVector(INTSXP, n));
    std::fill(INTEGER(ans),INTEGER(ans)+n,NA_INTEGER);
    break;
  case RblpapiT::Double:
    ans = PROTECT(Rf_allocVector(REALSXP,n));
    std::fill(REAL(ans),REAL(ans)+n,NA_REAL);
    break;
  case RblpapiT::Date:
    ans = PROTECT(Rf_allocVector(INTSXP,n));
    std::fill(INTEGER(ans),INTEGER(ans)+n,NA_INTEGER);
    addDateClass(ans);
    break;
  case RblpapiT::Datetime:
    ans = PROTECT(Rf_allocVector(STRSXP,n));
    break;
  case RblpapiT::String:
    ans = PROTECT(Rf_allocVector(STRSXP,n));
    break;
  default: // try to convert it as a string
    ans = PROTECT(Rf_allocVector(STRSXP,n));
    break;
  }
  UNPROTECT(1);
  return ans;
}

FieldInfo getFieldType(Session *session, Service& fieldInfoService, const std::string& field) {

  Request request = fieldInfoService.createRequest("FieldInfoRequest");
  request.append("id", field.c_str());
  request.set("returnFieldDocumentation", false);
  session->sendRequest(request);

  FieldInfo ans;
  while (true) {
    Event event = session->nextEvent();
    if (event.eventType() != Event::RESPONSE &&
        event.eventType() != Event::PARTIAL_RESPONSE) {
      continue;
    }

    MessageIterator msgIter(event);
    while (msgIter.next()) {
      Message msg = msgIter.message();
      //msg.asElement().print(std::cout);
      Element fields = msg.getElement("fieldData");
      if(fields.numValues() > 1) {
        throw std::logic_error("getFieldType: too many fields returned.");
      }
      Element field = fields.getValueAsElement(0);
      if (!field.hasElement("id")) {
        throw std::logic_error("Did not find 'id' in repsonse.");
      }
      if (field.hasElement("fieldError")) {
        std::ostringstream err;
        err << "Bad field: " << field.getElementAsString("id") << std::endl;
        throw std::logic_error(err.str());
      }
      if (!field.hasElement("fieldInfo")) {
        throw std::logic_error("Did not find fieldInfo in repsonse.");
      }
      Element fieldInfo = field.getElement("fieldInfo");
      if (!fieldInfo.hasElement("mnemonic") ||
          !fieldInfo.hasElement("datatype") ||
          !fieldInfo.hasElement("ftype")) {
        throw std::logic_error(
                               "fieldInfo missing info mnemonic/datatype/ftype.");
      }
      ans.id = field.getElementAsString("id");
      ans.mnemonic = fieldInfo.getElementAsString("mnemonic");
      ans.datatype = fieldInfo.getElementAsString("datatype");
      ans.ftype = fieldInfo.getElementAsString("ftype");
    }
    if (event.eventType() == Event::RESPONSE) {
      break;
    }
  }
  return ans;
}


vector<FieldInfo> getFieldTypes(Session *session,
                                     const std::vector<std::string> &fields) {
  const std::string APIFLDS_SVC("//blp/apiflds");
  if (!session->openService(APIFLDS_SVC.c_str())) {
    throw std::logic_error(std::string("Failed to open " + APIFLDS_SVC));
  }
  Service fieldInfoService = session->getService(APIFLDS_SVC.c_str());
  std::vector<FieldInfo> ans;
  for(auto field : fields) {
    ans.push_back(getFieldType(session, fieldInfoService, field));
  }
  return ans;
}

Rcpp::List allocateDataFrame(const vector<string>& rownames, const vector<string>& colnames, vector<RblpapiT>& coltypes) {

  if(colnames.size() != coltypes.size()) {
    throw std::logic_error("colnames size inconsistent with column types size.");
  }

  Rcpp::List ans(colnames.size());
  ans.attr("class") = "data.frame";
  ans.attr("names") = colnames;
  ans.attr("row.names") = rownames;
  for(size_t i = 0; i < colnames.size(); ++i) {
    ans[i] = allocateDataFrameColumn(coltypes[i], rownames.size());
  }
  return ans;
}
