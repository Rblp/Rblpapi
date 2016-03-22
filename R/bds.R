
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


##' This function uses the Bloomberg API to retrieve 'bds' (Bloomberg
##' Data Set) queries
##'
##' @title Run 'Bloomberg Data Set' Queries
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
##' @param identity An optional identity object.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A list with as many entries as there are entries in
##' \code{securities}; each list contains a data.frame with one row
##' per observations and as many columns as entries in
##' \code{fields}. If the list is of length one, it is collapsed into
##' a single data frame.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   ## simple query
##'   bds("GOOG US Equity", "TOP_20_HOLDERS_PUBLIC_FILINGS")
##'   ## example of using overrides
##'   overrd <- c("START_DT"="20150101", "END_DT"="20160101")
##'   bds("CPI YOY Index","ECO_RELEASE_DT_LIST", overrides = overrd)
##' }
bds <- function(security, field, options=NULL,
                overrides=NULL, verbose=FALSE,
                identity=NULL, con=defaultConnection()) {
    if (length(security) != 1L)
        stop("more than one security submitted.", call.=FALSE)
    if (length(field) != 1L)
        stop("more than one field submitted.", call.=FALSE)
    res <- bds_Impl(con, security, field, options, overrides, verbose, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

