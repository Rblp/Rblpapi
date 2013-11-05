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

#ifndef GET_FIELD_TYPES_H
#define GET_FIELD_TYPES_H

#include <string>
#include <vector>
#include <blpapi_session.h>
#include <blpapi_service.h>

std::string getFieldType(BloombergLP::blpapi::Session* session, BloombergLP::blpapi::Service fieldInfoService, const std::string& field);
void getFieldTypes(std::vector<std::string>& ans, BloombergLP::blpapi::Session* session, std::vector<std::string>& fields);

#endif // GET_FIELD_TYPES_H
