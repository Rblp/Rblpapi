
##
##  Copyright (C) 2016  Whit Armstrong and Dirk Eddelbuettel and John Laing
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


##' This function uses the Bloomberg API to retrieve 'portfolio' queries
##'
##' @title Run 'Portfolio Data' Queries
##' @param security A character value with a single security symbol in
##' Bloomberg notation.
##' @param field A character string with a single Bloomberg query field.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param overrides An optional named character vector with override
##' values. Each field must have both a name (designating the override
##' being set) as well as a value.
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param identity An optional identity object as created by a
##' \code{blpAuthenticate} call, and retrived via the internal function
##' \code{defaultAuthentication}.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A list with as many entries as there are entries in
##' \code{securities}; each list contains a data.frame with one row
##' per observations and as many columns as entries in
##' \code{fields}. If the list is of length one, it is collapsed into
##' a single data frame.
##' @author John Laing
## TODO: examples. Do global portfolios exist so that examples will
##       work for everyone? Otherwise I don't know how to do this.
getPortfolio <- function(security, field, options=NULL, overrides=NULL,
                      verbose=FALSE, identity=defaultAuthentication(),
                      con=defaultConnection()) {
    if (length(security) != 1L)
        stop("more than one security submitted.", call.=FALSE)
    if (length(field) != 1L)
        stop("more than one field submitted.", call.=FALSE)
    res <- getPortfolio_Impl(con, security, field, options, overrides, verbose, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

