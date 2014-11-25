
##' This function uses the Bloomberg API to retrieve bars for the requested security.
##'
##' @title Get Open/High/Low/Close/Volume Bars from Bloomberg
##' @param con A connection object as return by a \code{blpConnect} call
##' @param security A character variable describing a valid security ticker
##' @param eventType A character variable describing an event type;
##' default is \sQuote{TRADE}
##' @param barInterval A integer denoting the number of minutes for each bar
##' @param startTime A Datetime object with the start time, defaults
##' to one hour before current time
##' @param endTime A Datetime object with the end time, defaults
##' to current time
##' @param gapFillInitialBar A boolean indicating whether the initial bar is to be filled, defaults to \sQuote{FALSE}
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param asXts A boolean indicating whether an \code{xts} object
##' should be returned, defaults to \sQuote{TRUE}.
##' @return A numeric matrix with elements \sQuote{time},
##' \sQuote{open}, \sQuote{high}, \sQuote{low}, \sQuote{close},
##' \sQuote{numEvents}, \sQuote{volume}.  If \code{asXts} was set to
##' \sQuote{TRUE}, an \code{xts} object using the \sQuote{time} column
##' as index.
##' @author Dirk Eddelbuettel
getBars <- function(con,
                    security,
                    eventType = "TRADE",
                    barInterval=60,     		# in minutes
                    startTime = Sys.time()-60*60*6,
                    endTime = Sys.time(),
                    gapFillInitialBar = FALSE,
                    verbose = FALSE,
                    asXts = TRUE) {

    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getBars_Impl(con, security, eventType, barInterval,
                        startUTC, endUTC, gapFillInitialBar, verbose)
    if (asXts) {
        require(xts)
        res <- xts::xts(res[,-1], order.by=res[,1])
    }

    return(res)   # to return visibly
}
