
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


##' This function return the default connection object from the
##' package environment.  If no default connection has been
##' established yet, an error message is shown,
##'
##' @title Return the default connection object
##'
##' @details For the connection object, the required arguments
##' \code{host} and \code{port} argument can be set via
##' \code{\link{options}}. In addition, if an additional option
##' \code{blpAutoConnect} is set to \sQuote{TRUE}, a connection is
##' established in the \code{.onAttach()} function and stored in the
##' package environment. This effectively frees users from having to
##' explicitly create such an object. Of course, the user can also
##' call \code{blpConnect} explicitly and store the connection object.
##' This helper function looks up the stored connection object and
##' returns it. In case no connection has been established, and error
##' message is shown.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   con <- defaultConnection()  
##' }
defaultConnection <- function() {
    con <- .pkgenv$con
    if (is.null(con))
        stop("No connection object has been created. Use 'blpConnect()' first.",
             call.=FALSE)
    con
}
