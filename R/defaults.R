
##
##  Copyright (C) 2019  Whit Armstrong and Dirk Eddelbuettel and John Laing
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


##' These functions return the default connection/authentication objects
##' from the package environment.  If no default connection/authentication
##' has been established yet, will return NULL.
##' In the case of authentication, if using a desktop/workstation
##' session, NULL will work fine.  If using SAPI/Bpipe, you may need
##' to use blpAuthenticate() to create an authentication object.
##'
##' @title Return the default connection/authentication objects
##'
##' @details Required arguments can be set via \code{\link{options}}. See
##' \code{\link{blpConnect}} and \code{\link{blpAuthenticate}} for details.
##' In addition, if options \code{blpAutoConnect} and/or
##' \code{blpAutoAuthenticate} are set to \sQuote{TRUE}, a connection and/or
##' authentication is established in the \code{.onAttach()} function and
##' stored in the package environment. This effectively frees users from
##' having to explicitly create such objects. Of course, the user can also
##' call \code{blpConnect} and/or \code{blpAuthenticate} explicitly and
##' store the connection/authentication objects. These helper functions
##' look up the stored connection/authentications object and return them.
##' In case no connection has been established, an error message is shown.
##' In case no authentication has been established, NULL is returned.
##' (NULL is sufficent for Desktop API connections.)
##'
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   con <- defaultConnection()
##'   blpid <- defaultAuthentication()
##' }
##' @rdname defaults
defaultConnection <- function() {
    con <- .pkgenv$con
    if (is.null(con))
        stop("No connection object has been created. Use 'blpConnect()' first.",
             call.=FALSE)
    con
}
##' @rdname defaults
defaultAuthentication <- function() {
    blpAuth <- .pkgenv$blpAuth
    blpAuth
}
