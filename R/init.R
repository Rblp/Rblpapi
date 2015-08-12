
##
##  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel and John Laing
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
    if (getOption("blpAutoConnect", FALSE)) {
        con <- blpConnect()
        if (getOption("blpVerbose", FALSE)) {
            packageStartupMessage(paste0("Created and stored default connection object ",
                                         "for Rblpapi version ",
                                         packageDescription("Rblpapi")$Version, "."))
        }
    } else {
        con <- NULL
    }
    assign("con", con, envir=.pkgenv)
}
