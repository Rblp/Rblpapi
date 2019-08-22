
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
##' @param uuid An optional character variable with a unique user id
##'     token. If this is missing the function will attempt to connect
##'     to B-PIPE or SAPI using the connection. It is assumed that an
##'     app_name was set. See \code{blpConnect()} for app_name
##'     information.  Defaults to \code{getOption("blpUUID")} or NULL
##' @param host An optional character variable with a hostname.  This is
##'     the hostname of the machine where the user last authenticated.
##'     Either host or ip.address should be provided for user/uuid
##'     authentication. Note this is likely not the same 'host' used in
##'     \code{blpConnect()}.  Defaults to
##'     \code{getOption("blpLoginHostname")} or "localhost"
##' @param ip.address An optional character variable with an IP address
##'     for authentication.  Usually the IP address where the uuid/user
##'     last logged into the Bloomberg Terminal appication.  Defaults to
##'     \code{getOption("blpLoginIP")} or NULL, which will then lookup
##'     the IP of the "host" option. 
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function. This is the only required
##' argument to authenticate a B-PIPE connection with a appName.
##' Defaults to \code{defaultConnection}.
##' @param default A logical indicating whether this authentication should
##' be saved as the default, as opposed to returned to the
##' user. Default to \code{TRUE}.
##' @return In the \code{default=TRUE} case nothing is returned, and
##' this authentication is automatically used for all future calls which
##' omit the \code{identity} argument. Otherwise an authentication object is
##' returned which is required by all the accessor functions in the
##' package. (e.g. \code{bdp()} \code{bds()} \code{getPortfolio()} 
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##' blpConnect(host=blpHost, port=blpPort)
##' blpAuthenticate(uuid=blpUUID, ip=blpIP_address)
##' bdp("IBM US Equity", "NAME")
##'
##' blpid <- blpAuthenticate(uuid=blpUUID, ip=blpIP_address)
##' bdp("IBM US Equity", "NAME", identity=blpid)
##' }

blpAuthenticate <- function(uuid=getOption("blpUUID", NULL),
                            host=getOption("blpLoginHostname", "localhost"),
                            ip.address=getOption("blpLoginIP", NULL),
                            con=defaultConnection(),
                            default=TRUE) {
    if(is.null(uuid)) {
        ## no UUID, assume B-PIPE or SAPI with application ID
        blpAuth <- authenticate_Impl(con, NULL, NULL)
    } else {
        if ( (!is.null(ip.address)) && (!identical(host,"localhost")) ) {
            warning("Both ip.address and host are set.  Using ip.address.") }
        
        ## have UUID, assume SAPI
        if (is.null(ip.address)) {
            ## Linux only ?
            cmd.res <- system(paste("host",host), intern=TRUE,
                              ignore.stdout=FALSE, ignore.stderr=FALSE,wait=TRUE)
            ip.address <- strsplit(cmd.res,"address ")[[1]][2]
        }
        blpAuth <- authenticate_Impl(con, as.character(uuid), ip.address)
    }
    ## if we're setting the silent/hidden default object, return nothing
    ## else, return the object (keeps old behavior)
    if (default)
        .pkgenv$blpAuth <- blpAuth
    else
        return(blpAuth)
}
