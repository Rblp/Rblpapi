
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


##' This function uses the Bloomberg API to retrieve fieldInfo
##'
##' @title Run 'Bloomberg Field Data' Queries
##' @param fields A character vector with Bloomberg query fields.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A data frame with as a many rows as entries in
##' \code{fields}
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   fieldInfo(c("PX_LAST", "VOLUME"))
##' }
fieldInfo <- function(fields, con=defaultConnection()) {
    if (any(duplicated(fields))) stop("Duplicated fields submitted.", call.=FALSE)
    fieldInfo_Impl(con, fields)
}
