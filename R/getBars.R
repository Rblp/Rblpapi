
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


##' This function uses the Bloomberg API to retrieve bars for the requested security.
##'
##' @title Get Open/High/Low/Close/Volume Bars from Bloomberg
##' @param security A character variable describing a valid security ticker
##' @param eventType A character variable describing an event type;
##' default is \sQuote{TRADE}
##' @param barInterval A integer denoting the number of minutes for each bar
##' @param startTime A Datetime object with the start time, defaults
##' to one hour before current time
##' @param endTime A Datetime object with the end time, defaults
##' to current time
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param returnAs A character variable describing the type of return
##' object; currently supported are \sQuote{matrix} (also the default),
##' \sQuote{fts}, \sQuote{xts} and \sQuote{zoo}
##' @param tz A character variable with the desired local timezone,
##' defaulting to the value \sQuote{TZ} environment variable, and
##' \sQuote{UTC} if unset
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A numeric matrix with elements \sQuote{time} (as a
##' \sQuote{POSIXct} object), \sQuote{open}, \sQuote{high},
##' \sQuote{low}, \sQuote{close}, \sQuote{numEvents}, \sQuote{volume},
##' \sQuote{value} or an object of the type selected in \code{returnAs}.
##' Note that the \sQuote{time} value is adjusted: Bloomberg returns the
##' \emph{opening} time of the bar interval, whereas financial studies
##' typically refer to the most recent timestamp. For this reason we
##' add the length of the bar interval to time value from Bloomberg to
##' obtain the time at the end of the interval.
##' @author Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   getBars("ES1 Index")
##' }
getBars <- function(security,
                    eventType = "TRADE",
                    barInterval=60,     		# in minutes
                    startTime = Sys.time()-60*60*6,
                    endTime = Sys.time(),
                    options = NULL,
                    verbose = FALSE,
                    returnAs = getOption("blpType", "matrix"),
                    tz = Sys.getenv("TZ", unset="UTC"),
                    con = defaultConnection()) {

    if (!inherits(startTime, "POSIXt") || !inherits(endTime, "POSIXt")) {
        stop("startTime and endTime must be Datetime objects", call.=FALSE)
    }
    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getBars_Impl(con, security, eventType, barInterval,
                        startUTC, endUTC, options, verbose)

    attr(res[,1], "tzone") <- tz

    res <- switch(returnAs,
                  matrix = res,                # default is matrix
                  fts    = fts::fts(res[,1], res[,-1]),
                  xts    = xts::xts(res[,-1], order.by=res[,1]),
                  zoo    = zoo::zoo(res[,-1], order.by=res[,1]),
                  res)                         # fallback is also matrix
    return(res)   # to return visibly
}
