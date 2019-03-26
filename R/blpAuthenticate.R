
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
##' @param uuid A character variable with a unique user id token. If this
##' is missing the function will attempt to connect to bpipe using the connection. It
##' is assumed that an app_name was set. See blpConnect() for app_name information
##' @param host A character variable with a hostname, defaults to 'localhost'
##' @param ip.address An optional character variable with an IP address
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function. This is the only required
##' argument to authenticate a bpipe connection with a appName.
##' \code{defaultConnection}.
##' @return The returned object should be passed to subsequent data
##' calls via bdp(), bds(), etc.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##' blpConnect(host=blpHost, port=blpPort)
##' blpid <- blpAuthenticate(uuid=blpUUID, ip=blpIP_address)
##' bdp("IBM US Equity", "NAME", identity=blpid)
##' }

blpAuthenticate <- function(uuid, host="localhost", ip.address, con=defaultConnection()) {
    if(missing(uuid)) {
        ## no UUID, assume BPIPE
        authenticate_Impl(con, NULL, NULL)
    } else {
        ## have UUID, assume SAPI
        if (missing(ip.address)) {
            ## Linux only ?
            cmd.res <- system(paste("host",host), intern=TRUE,
                              ignore.stdout=FALSE, ignore.stderr=FALSE,wait=TRUE)
            ip.address <- strsplit(cmd.res,"address ")[[1]][2]
        }
        authenticate_Impl(con, as.character(uuid), ip.address)
    }
}
