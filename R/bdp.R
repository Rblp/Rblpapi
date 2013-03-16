blpConnect <- function(host = NULL, port = NULL, log.level = "warning") {
    .Call("bdp_connect", host, port, log.level, PACKAGE="bdp")
}

bdh <- function(conn, securities, fields, start.date, end.date=NULL, include.non.trading.days=FALSE, options=NULL) {

    start.date = format(start.date, format="%Y%m%d")
    if (!is.null(end.date)) {
        end.date = format(end.date, format="%Y%m%d")
    }

    if (include.non.trading.days) {
        options <- c(options,structure(c("ALL_CALENDAR_DAYS", "NIL_VALUE"),names=c("nonTradingDayFillOption", "nonTradingDayFillMethod")))
    }

    .Call("bdh", conn, securities, fields, start.date, end.date, options, PACKAGE="bdp")
}

## bdp <- function(conn, securities, fields, options=NULL) {
##     .Call("blp", securities, fields, PACKAGE="bdp")
## }
