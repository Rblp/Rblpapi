
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


##' This function searches for matching Bloomberg data fields given a
##' search term.
##'
##' @title Search for matching data fields
##'
##' @param searchterm A string with the term to search for
##' @param excludeterm A string with an expression for matches to
##' excludes, defaults to \dQuote{Static}
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##'
##' @return A \code{data.frame} with three columns of the id,
##' mnenemonic and description of each match.
##'
##' @author Dirk Eddelbuettel
##'
##' @examples
##' \dontrun{
##'   head(fieldSearch("vwap"), 20)
##' }
##'
fieldSearch <- function(searchterm, excludeterm="Static", con=defaultConnection()) {
    fieldSearch_Impl(con, searchterm, excludeterm)
}
