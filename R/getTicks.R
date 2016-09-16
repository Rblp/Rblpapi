
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


##' This function uses the Bloomberg API to retrieve ticks for the requested security.
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
##' object; currently supported are \sQuote{matrix} (also the default),
##' \sQuote{fts}, \sQuote{xts} and \sQuote{zoo}
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
##' @examples
##' \dontrun{
##'   res <- getTicks("ES1 Index")
##'   str(res)
##'   head(res, 20)
##' }
getTicks <- function(security,
                     eventType = "TRADE",
                     startTime = Sys.time()-60*60,
                     endTime = Sys.time(),
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
    res <- getTicks_Impl(con, security, eventType, startUTC, endUTC, verbose)

    attr(res[,1], "tzone") <- tz

    ## return data, but omit event type which is character type
    res <- switch(returnAs,
                  matrix = res[,-2],           # default is matrix
                  fts    = fts::fts(res[,1], res[,-(1:2)]),
                  xts    = xts::xts(res[,-(1:2)], order.by=res[,1]),
                  zoo    = zoo::zoo(res[,-(1:2)], order.by=res[,1]),
                  res)                         # fallback is also matrix
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
##' object; the default is return a matrix with results as received;
##' optionally a \sQuote{wide} \code{xts} object with merged data can
##' be returned
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
                             returnAs = getOption("blpType", "matrix"),
                             tz = Sys.getenv("TZ", unset="UTC"),
                             con = defaultConnection()) {

    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getTicks_Impl(con, security, eventType, startUTC, endUTC, verbose)

    attr(res[,1], "tzone") <- tz

    if (returnAs == "xts") {

        ## Make timestamps unique
        res[,1] <- xts::make.index.unique(res[,1], eps=2e-6)

        ## Subset into blocks for each event type, creating xts
        rl <- lapply(eventType,
                 function(s) {
                     x <- subset(res, res$type==s)
                     colnames(x)[3] <- tolower(s)
                     colnames(x)[4] <- paste0(tolower(s), "sz")
                     xts::xts(x[,3:4], order.by=x[,1])
                 })
        x <- do.call(merge, rl)

        ## Use na.locf to carry bid, ask, .. forward, but do not use trade column
        ind <- !grepl("trade", colnames(x))
        x[,ind] <- zoo::na.locf(x[,ind])

        zoo::index(x) <- trunc(zoo::index(x)) # truncated time stamp down to seconds
        return(x)
    }

    return(res)

}
