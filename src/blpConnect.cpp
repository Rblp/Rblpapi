// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  blpConnect.cpp -- Function to establish Bloomberg connection
//
//  Copyright (C) 2013  Whit Armstrong
//  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel
//  Copyright (C) 2019  Whit Armstrong, Dirk Eddelbuettel and Alfred Kanzler
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
SEXP blpConnect_Impl(const std::string host, const int port, SEXP app_name_) {
    SessionOptions sessionOptions;
    sessionOptions.setServerHost(host.c_str());
    sessionOptions.setServerPort(port);

    if (app_name_ != R_NilValue) {
        std::string app_name = Rcpp::as<std::string>(app_name_);
        std::string authentication_string = APP_PREFIX + app_name;
        sessionOptions.setAuthenticationOptions(authentication_string.c_str());
    }
    Session* sp = new Session(sessionOptions);

    if (!sp->start()) {
        Rcpp::stop("Failed to start session.\n");
    }
    if (sp == NULL) {
        Rcpp::stop("Session pointer is NULL\n");
    }

    return createExternalPointer<Session>(sp, sessionFinalizer, "blpapi::Session*");
}
