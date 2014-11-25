
##' This function uses the Bloomberg API to retrieve ticks for the requested security.
##'
##' @title Get Ticks from Bloomberg
##' @param con A connection object as return by a \code{blpConnect} call
##' @param security A character variable describing a valid security ticker
##' @param eventType A character variable describing an event type;
##' default is \sQuote{TRADE}
##' @param startTime A Datetime object with the start time, defaults
##' to one hour before current time
##' @param endTime A Datetime object with the end time, defaults
##' to current time
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param asXts A boolean indicating whether an \code{xts} object
##' should be returned, defaults to \sQuote{TRUE}.
##' @return A numeric matrix with elements \sQuote{time},
##' \sQuote{values} and \sQuote{sizes}.  If \code{asXts} was set to
##' \sQuote{TRUE}, an \code{xts} object using the \sQuote{time} column
##' as index.
##' @author Dirk Eddelbuettel
getTicks <- function(con,
                     security,
                     eventType = "TRADE",
                     startTime = Sys.time()-60*60,
                     endTime = Sys.time(),
                     verbose = FALSE,
                     asXts = TRUE) {

    fmt <- "%Y-%m-%dT%H:%M:%S"
    startUTC <- format(startTime, fmt, tz="UTC")
    endUTC <- format(endTime, fmt, tz="UTC")
    res <- getTicks_Impl(con, security, eventType, startUTC, endUTC, verbose)
    if (asXts) {
        require(xts)
        res <- xts::xts(res[,-1], order.by=res[,1])
    }

    return(res)   # to return visibly
}
