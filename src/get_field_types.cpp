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

#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include <blpapi_session.h>
#include <blpapi_service.h>
#include <blpapi_event.h>
#include <blpapi_message.h>
#include <blpapi_element.h>
#include <blpapi_request.h>
#include <Rcpp.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::Service;
using BloombergLP::blpapi::Request;
using BloombergLP::blpapi::Event;
using BloombergLP::blpapi::Message;
using BloombergLP::blpapi::MessageIterator;
using BloombergLP::blpapi::Element;

std::string getFieldType(Session* session, Service& fieldInfoService, const std::string& field) {
  Request request = fieldInfoService.createRequest("FieldInfoRequest");
  request.append("id", field.c_str());
  request.set("returnFieldDocumentation", false);
  session->sendRequest(request);
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
        throw std::logic_error("Only one field requested.");
      }
      Element field = fields.getValueAsElement(0);
      if(field.hasElement("fieldError")) {
        std::ostringstream err;
        err << "Bad field: " << field.getElementAsString("id") << std::endl;
        throw std::logic_error(err.str());
      }
      if(!field.hasElement("fieldInfo") || !field.getElement("fieldInfo").hasElement("datatype")) {
        throw std::logic_error("Did not find datatype in fieldInfo request.");
      }
      return field.getElement("fieldInfo").getElementAsString("datatype");
    }
    if (event.eventType() == Event::RESPONSE) {
      break;
    }
  }
}

void getFieldTypes(std::vector<std::string>& ans, Session* session, std::vector<std::string>& fields) {
  const std::string APIFLDS_SVC("//blp/apiflds");
  if (!session->openService(APIFLDS_SVC.c_str())) {
    throw std::logic_error(std::string("Failed to open " + APIFLDS_SVC));
  }
  Service fieldInfoService = session->getService(APIFLDS_SVC.c_str());
  for(R_len_t i = 0; i < fields.size(); i++) {
    ans.push_back(getFieldType(session,fieldInfoService,fields[i]));
  }
}
