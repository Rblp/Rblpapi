
##
##  Copyright (C) 2015 - 2016  Whit Armstrong and Dirk Eddelbuettel and John Laing
##
##  This file is part of Rblpapi
##
##  Rblpapi is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 2 of the License, or
##  (at your option) any later version.
##
##  Rblpapi is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.


.pkgenv <- new.env(parent=emptyenv())

.onAttach <- function(libname, pkgname) {
    packageStartupMessage(paste0("Rblpapi version ", packageVersion("Rblpapi"),
                                 " using Blpapi headers ", getHeaderVersion(),
                                 " and run-time ", getRuntimeVersion(), "."))
    packageStartupMessage(paste0("Please respect the Bloomberg licensing agreement ",
                                 "and terms of service."))
    
    if (getOption("blpAutoConnect", FALSE)) {
        con <- blpConnect()
        if (getOption("blpVerbose", FALSE)) {
            packageStartupMessage("Created and stored default connection object.")
        }
    } else {
        con <- NULL
    }
    assign("con", con, envir=.pkgenv)
}
