
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
##' @param gapFillInitialBar A boolean indicating whether the initial bar is to be filled, defaults to \sQuote{FALSE}
##' @param adjustmentFollowDPDF A boolean indicating whether DPDF settings will be applied, defaults to \sQuote{TRUE}
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param returnAs A character variable describing the type of return
##' object; currently supported are \sQuote{matrix} (also the default),
##' \sQuote{fts}, \sQuote{xts} and \sQuote{zoo}
##' @param tz A character variable with the desired local timezone,
##' defaulting to the value \sQuote{TZ} environment variable, and
##' \sQuote{UTC} if unset
##' @param con A connection object as returned by a \code{blpConnect} call
##' @return A numeric matrix with elements \sQuote{time} (as a
##' \sQuote{POSIXct} object), \sQuote{open}, \sQuote{high},
##' \sQuote{low}, \sQuote{close}, \sQuote{numEvents}, \sQuote{volume},
##' or an object of the type selected in \code{returnAs}. Note that
##' the \sQuote{time} value is adjusted: Bloomberg returns the
##' \emph{opening} time of the bar interval, whereas financial studies
##' typically refer to the most recent timestamp. For this reason we
##' add the length of the bar interval to time value from Bloomberg to
##' obtain the time at the end of the interval.
##' @author Dirk Eddelbuettel
getBars <- function(security,
                    eventType = "TRADE",
                    barInterval=60,     		# in minutes
                    startTime = Sys.time()-60*60*6,
                    endTime = Sys.time(),
                    gapFillInitialBar = FALSE,
                    adjustmentFollowDPDF = TRUE,
                    verbose = FALSE,
                    returnAs = getOption("blpType", "matrix"),
                    tz = Sys.getenv("TZ", unset="UTC"),
                    con = .pkgenv$con) {

    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getBars_Impl(con, security, eventType, barInterval,
                        startUTC, endUTC, gapFillInitialBar, adjustmentFollowDPDF, verbose)

    attr(res[,1], "tzone") <- tz

    res <- switch(returnAs,
                  matrix = res,                # default is matrix
                  fts    = fts::fts(res[,1], res[,-1]),
                  xts    = xts::xts(res[,-1], order.by=res[,1]),
                  zoo    = zoo::zoo(res[,-1], order.by=res[,1]),
                  res)                         # fallback is also matrix
    return(res)   # to return visibly
}
