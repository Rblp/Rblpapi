// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  blpConnectWithApp.cpp -- Function to establish Bloomberg connection to bpipe with
//                           an app name.
//
//  Copyright (C) 2013  Whit Armstrong
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel
//
//  This file is part of Rblpapi
//
//  Rblpapi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  Rblpapi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.
//
// Added 2019 to allow server authentication with an application name.
// Authorization by application is described on page 63 of the guide
// "Bloomberg API"

#include <string>
#include <blpapi_session.h>
#include <Rcpp.h>
#include <finalizers.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::SessionOptions;

const std::string APP_PREFIX("AuthenticationMode=APPLICATION_ONLY;"
			     "ApplicationAuthenticationType=APPNAME_AND_KEY;"
			     "ApplicationName=");

static void sessionFinalizer(SEXP session_) {
    Session* session = reinterpret_cast<Session*>(R_ExternalPtrAddr(session_));
    if (session) {
        delete session;
        R_ClearExternalPtr(session_);
    }
}

// [[Rcpp::export]]
SEXP blpConnectWithApp_Impl(const std::string host, const int port, const std::string app_name) {
    SessionOptions sessionOptions;
    sessionOptions.setServerHost(host.c_str());
    sessionOptions.setServerPort(port);
    std::string app(app_name);
    std::string authentication_string = APP_PREFIX + app;
    sessionOptions.setAuthenticationOptions(authentication_string.c_str());
    Session* sp = new Session(sessionOptions);

    if (!sp->start()) {
        Rcpp::stop("Failed to start session.\n");
    }

    return createExternalPointer<Session>(sp, sessionFinalizer, "blpapi::Session*");
}
