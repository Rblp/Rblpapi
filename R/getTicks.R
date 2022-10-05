
##
##  Copyright (C) 2015 - 2022  Whit Armstrong and Dirk Eddelbuettel and John Laing
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


##' This function uses the Bloomberg API to retrieve ticks for the requested security.
##'
##' @note Bloomberg returns condition codes as well, and may return \emph{multiple
##' observations for the same trade}. Eg for ES we can get \sQuote{AS} or
##' \sQuote{AB} for aggressor buy or sell, \sQuote{OR} for an order participating in the
##' matching event, or a \sQuote{TSUM} trade summary.  Note that this implies
##' double-counting.  There may be an option for this in the API.
##'
##' The Bloomberg API allows to retrieve up to 140 days of intra-day
##' history relative to the current date.
##'
##' @title Get Ticks from Bloomberg
##' @param security A character variable describing a valid security ticker
##' @param eventType A character variable describing an event, default
##' is \sQuote{TRADE}.
##' @param startTime A Datetime object with the start time, defaults
##' to one hour before current time
##' @param endTime A Datetime object with the end time, defaults
##' to current time
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param returnAs A character variable describing the type of return
##' object; currently supported are \sQuote{data.frame} (also the default),
##' \sQuote{data.table}, \sQuote{xts} and \sQuote{zoo}
##' @param tz A character variable with the desired local timezone,
##' defaulting to the value \sQuote{TZ} environment variable, and
##' \sQuote{UTC} if unset
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return Depending on the value of \sQuote{returnAs}, either a
##' \sQuote{data.frame} or \sQuote{data.table} object also containing
##' non-numerical information such as condition codes, or a time-indexed
##' container of type \sQuote{xts} or \sQuote{zoo} with
##' a numeric matrix containing only \sQuote{value} and \sQuote{size}.
##' @author Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   res <- getTicks("ES1 Index")
##'   str(res)
##'   head(res, 20)
##'   res <- getTicks("ES1 Index", returnAs="data.table")
##'   str(res)
##'   head(res, 20)
##' }
getTicks <- function(security,
                     eventType = "TRADE",
                     startTime = Sys.time()-60*60,
                     endTime = Sys.time(),
                     verbose = FALSE,
                     returnAs = getOption("blpType", "data.frame"),
                     tz = Sys.getenv("TZ", unset="UTC"),
                     con = defaultConnection()) {

    match.arg(returnAs, c("data.frame", "xts", "zoo", "data.table"))
    if (!inherits(startTime, "POSIXt") || !inherits(endTime, "POSIXt")) {
        stop("startTime and endTime must be Datetime objects", call.=FALSE)
    }
    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getTicks_Impl(con, security, eventType, startUTC, endUTC,
                         setCondCodes = returnAs %in% c("data.frame", "data.table"),
                         verbose)

    attr(res[,1], "tzone") <- tz


    ## return data, but omit event type which is character type
    res <- switch(returnAs,
                  data.frame = res,            # default is data.frame
                  xts        = xts::xts(res[,-c(1:2,5)], order.by=res[,1]),
                  zoo        = zoo::zoo(res[,-c(1:2,5)], order.by=res[,1]),
                  data.table = asDataTable(res),
                  res)                         # fallback also data.frame
    return(res)   # to return visibly

}

##' This function uses the Bloomberg API to retrieve multiple ticks
##' for the requested security.
##'
##' @title Get Multiple Ticks from Bloomberg
##' @param security A character variable describing a valid security ticker
##' @param eventType A character vector describing event
##' types, default is \code{c("TRADE", "BID", "ASK")}
##' @param startTime A Datetime object with the start time, defaults
##' to one hour before current time
##' @param endTime A Datetime object with the end time, defaults
##' to current time
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param returnAs A character variable describing the type of return
##' object; currently supported are \sQuote{data.frame} (also the default)
##' and \sQuote{data.table}
##' @param tz A character variable with the desired local timezone,
##' defaulting to the value \sQuote{TZ} environment variable, and
##' \sQuote{UTC} if unset
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A numeric matrix with elements \sQuote{time}, (as a
##' \sQuote{POSIXct} object), \sQuote{values} and \sQuote{sizes}, or
##' an object of the type selected in \code{returnAs}.
##' @author Dirk Eddelbuettel
getMultipleTicks <- function(security,
                             eventType = c("TRADE", "BID", "ASK"),
                             startTime = Sys.time()-60*60,
                             endTime = Sys.time(),
                             verbose = FALSE,
                             returnAs = getOption("blpType", "data.frame"),
                             tz = Sys.getenv("TZ", unset="UTC"),
                             con = defaultConnection()) {

    match.arg(returnAs, c("data.frame", "data.table"))
    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getTicks_Impl(con, security, eventType, startUTC, endUTC, TRUE, verbose)

    attr(res[,1], "tzone") <- tz

    if (returnAs == "data.table") {
        res <- asDataTable(res)
    }

    return(res)

}
