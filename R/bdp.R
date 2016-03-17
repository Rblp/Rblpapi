
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


##' This function uses the Bloomberg API to retrieve 'bdp' (Bloomberg
##' Data Point) queries
##'
##' @title Run 'Bloomberg Data Point' Queries
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character vector with Bloomberg query fields.
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
##' @return A data frame with as a many rows as entries in
##' \code{securities} and columns as entries in \code{fields}.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   bdp(c("ESA Index", "SPY US Equity"), c("PX_LAST", "VOLUME"))
##'
##'   ##  using overrides (cf https://github.com/Rblp/Rblpapi/issues/67)
##'   bdp("EN00 Index", "MLI_OAS", overrides=c(MLI_DATE="20150831"))
##' }
bdp <- function(securities, fields, options=NULL, overrides=NULL,
                verbose=FALSE, identity=NULL, con=defaultConnection()) {
    if (any(duplicated(securities))) stop("Duplicated securities submitted.", call.=FALSE)
    bdp_Impl(con, securities, fields, options, overrides, verbose, identity)
}

