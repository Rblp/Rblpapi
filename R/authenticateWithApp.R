
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


##' This function authenticates against the the Bloomberg API
##'
##' @title Authenticate Bloomberg API access
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return The returned bloomberg identity object should be passed to subsequent data
##' calls via bdp(), bds(), etc.
##' @author Alfred Kanzler
##' @examples
##' \dontrun{
##' blpConnect(host=blpHost, port=blpPort, app_name=blpAppName)
##' blpid <- blpAuthenticate(con)
##' bdp("IBM US Equity", "NAME", identity=blpid)
##' }

authenticateWithApp <- function(con=defaultConnection()) {
    authenticateWithApp_Impl(con)
}
