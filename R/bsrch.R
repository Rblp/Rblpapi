
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


##' This function uses the Bloomberg API to retrieve 'bsrcb' (Bloomberg
##' SRCH Data) queries
##'
##' @title Run 'Bloomberg SRCH' Queries
##' @param domain A character string with the name of the domain
##' to execute.  It can be a user defined SRCH screen, commodity
##' screen or one of the variety of Bloomberg examples. All domains
##' are in the format <domain>:<search_name>.
##' @param limit A character string containing a value by which to
##' limit the search length -- NOT YET IMPLEMENTED
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A data frame object with the requested SRCH data.
##' @author Morgan Williams and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##' head(bsrch("COMDTY:NGFLOW"), 20)
##' head(bsrch("COMDTY:VESSELS"), 20)
##' }
bsrch <- function(domain,
                 limit="",
                 verbose=FALSE,
                 con=defaultConnection()) {

    res <- bsrch_Impl(con, domain, limit, verbose)
    res
}
