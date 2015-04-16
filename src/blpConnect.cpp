// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  blpConnect.cpp -- Function to establish Bloomberg connection
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

#include <string>
#include <blpapi_session.h>
#include <Rcpp.h>
#include <finalizers.h>

using BloombergLP::blpapi::Session;
using BloombergLP::blpapi::SessionOptions;

static void sessionFinalizer(SEXP session_) {
    Session* session = reinterpret_cast<Session*>(R_ExternalPtrAddr(session_));
    if (session) {
        delete session;
        R_ClearExternalPtr(session_);
    }
}

// [[Rcpp::export]]
SEXP blpConnect_Impl(const std::string host, const int port) {
    SessionOptions sessionOptions;
    sessionOptions.setServerHost(host.c_str());
    sessionOptions.setServerPort(port);
    Session* sp = new Session(sessionOptions);

    if (!sp->start()) {
        Rcpp::stop("Failed to start session.\n");
    }
    
    return createExternalPointer<Session>(sp, sessionFinalizer, "blpapi::Session*");
}
