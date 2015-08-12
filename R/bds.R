
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


##' This function uses the Bloomberg API to retrieve 'bds' (Bloomberg
##' Data Set) queries
##'
##' @title Run 'Bloomberg Data Set' Queries
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character string with a single Bloomberg query field.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param overrides An optional named character vector with override
##' values. Each field must have both a name (designating the override
##' being set) as well as a value.
##' @param identity An optional identity object.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A list with as a entries as there are entries in
##' \code{securities}; each list contains a data.frame with one row
##' per observations and as many columns as entries in
##' \code{fields}. If the list is of length one, it is collapsed into
##' a single data frame.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   bds("GOOG US Equity", "TOP_20_HOLDERS_PUBLIC_FILINGS")
##' }
bds <- function(securities, fields, options=NULL,
                overrides=NULL, identity=NULL, con=defaultConnection()) {
    if (any(duplicated(securities))) stop("Duplicated securities submitted.", call.=FALSE)
    res <- bds_Impl(con, securities, fields, options, overrides, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

