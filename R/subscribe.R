
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


##' This function uses the Bloomberg API to stream live market data
##'
##' @title Subscribe to streaming market data
##' @details
##' The subscribe function allows one to subscribe to streaming market
##' quotes.
##'
##' Full detials of the subscription string can be found in the header
##' file
##' \href{http://bloomberg.github.io/blpapi-docs/cpp/3.8/blpapi__subscriptionlist_8h.html}{blpapi_subscriptionlist.h}.
##' 
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character vector with Bloomberg query fields.
##' @param fun An R function to be called on the subscription data.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param identity An optional identity object.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return This function always returns NULL.
##' @references \url{http://bloomberg.github.io/blpapi-docs/cpp/3.8}
##' @author Whit Armstrong
##' @examples
##' \dontrun{
##'   subscribe(securities=c("TYZ5 Comdty","/cusip/912810RE0@BGN"),
##'             fields=c("LAST_PRICE","BID","ASK"),
##'             fun=function(x) print(str(x$data)))
##' }
subscribe <- function(securities, fields, fun, options=NULL, identity=NULL, con=defaultConnection()) {
    if (any(duplicated(securities))) stop("Duplicated securities submitted.", call.=FALSE)
    subscribe_Impl(con, securities, fields, fun, options, identity)
}

