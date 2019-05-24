
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


##' This function returns the default authentication object from the
##' package environment.  If no default authentication has been
##' established yet, will return NULL.  If using a desktop/workstation
##' session, NULL will work fine.  If using SAPI/B-PIPE, you may need
##' to use blpAuthenticate() to create an authentication object.
##'
##' @title Return the default authentication object
##'
##' @details For the authentication object, one can authenticate via
##'     UUID/login-location or via Application-Name.
##'
##' For UUID authentication, the required arguments \code{uuid} and
##'     (\code{ip.address} or \code{host}) arguments can be set via
##'     \code{\link{options}} as \code{blpUUID}, \code{blpLoginIP} and
##'     \code{blpLoginHostname} .
##'
##' For Application-Name authentication, the \code{blpAppName} argument
##'     should have been passed to \code{blpConnect}, and no further
##'     arguments are needed to \code{blpAuthenticate}
##'
##' If an additional option \code{blpAutoAuthenticate} is set to
##'     \sQuote{TRUE}, an authentication is established in the
##'     \code{.onAttach()} function and stored in the package
##'     environment.
##'
##' In the \code{default=TRUE} case nothing is returned, and
##'     this identity object is automatically used for all future calls
##'     which omit the \code{identity} argument. Otherwise an identity
##'     object is returned which is required by all the accessor
##'     functions in the package.
##'
##' @return This helper function looks up the stored authentication
##'     object and returns it. In case no authentication has been
##'     established, NULL is returned.  (NULL is sufficient for Desktop
##'     API connections.)
##'
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   blpid <- defaultAuthentication()  
##' }
defaultAuthentication <- function() {
    blpAuth <- .pkgenv$blpAuth
    blpAuth
}
