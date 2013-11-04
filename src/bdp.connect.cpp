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

#include <string>
#include <blpapi_session.h>
#include <Rcpp.h>
#include <finalizers.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::SessionOptions;

static void sessionFinalizer(SEXP session_) {
  Session* session = reinterpret_cast<Session*>(R_ExternalPtrAddr(session_));
  if(session) {
    delete session;
    R_ClearExternalPtr(session_);
  }
}

extern "C" SEXP bdp_connect(SEXP host_, SEXP port_) {
  std::string host(Rcpp::as<std::string>(host_));
  int port(Rcpp::as<int>(port_));

  SessionOptions sessionOptions;
  sessionOptions.setServerHost(host.c_str());
  sessionOptions.setServerPort(port);
  Session* sp = new Session(sessionOptions);

  if (!sp->start()) {
    REprintf("Failed to start session.\n");
    return R_NilValue;
  }

  return createExternalPointer<Session>(sp,sessionFinalizer,"blpapi::Session*");
}
