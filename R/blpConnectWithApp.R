
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
##
## This file connects using an application name to authorize the connection
## See Bloomberg API section 6.3 for information on connecting to bpipe with an
## application name.


##' This function connects to the Bloomberg API
##'
##' @title Establish connection to Bloomberg service
##' @param port An integer variable with the connection port. Default
##' to \code{8194L}.
##' @param host A character option with either a machine name that is
##' resolvable by DNS, or an IP address. Defaults to
##' \sQuote{localhost}.
##' @param app_name the name of the application that is authorized
##' to connect to bpipe
##' @param default A logical indicating whether this connection should
##' be saved as the default, as opposed to returned to the
##' user. Default to \code{TRUE}.
##' @return In the \code{default=TRUE} case nothing is returned, and
##' this connection is automatically used for all future calls which
##' omit the \code{con} argument. Otherwise a connection object is
##' returned which is required by all the accessor functions in the
##' package.
##' @details For both \code{host} and \code{port} argument, default
##' values can also be specified via \code{\link{options}} using,
##' respectively, the named entries \code{blpHost} and
##' \code{blpConnectWithApp}.
##'
##' If an additional option \code{blpAutoConnect} is set to
##' \sQuote{TRUE}, a connection is established in the
##' \code{.onAttach()} function and stored in the package
##' environment. This effectively frees users from having to
##' explicitly create such an object.
##' @author Alfred Kanzler
##' @seealso bPipe connections using an application name require authentication
##' via \code{blpAuthenticateWithApp} after \code{blpConnectWithApp}.
##' @examples
##' \dontrun{
##'   con <- blpConnectWithApp()   # adjust as needed
##' }
blpConnectWithApp <- function(host=getOption("blpHost", "localhost"),
                       port=getOption("blpPort", 8194L),
                       app_name=getOption("blpAppName", "None"),
                       default=TRUE) {
    if (storage.mode(port) != "integer") port <- as.integer(port)
    if (storage.mode(host) != "character") stop("Host argument must be character.", call.=FALSE)
    if (storage.mode(app_name) != "character") stop("App name argument must be character.", call.=FALSE)
    con <- blpConnectWithApp_Impl(host, port, app_name)

    if (default) .pkgenv$con <- con else return(con)
}
