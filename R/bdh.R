
##' This function uses the Bloomberg API to retrieve 'bdh' (Bloomberg
##' Data History) queries
##'
##' @title Run 'Bloomberg Data History' Queries
##' @param con A connection object as returned by a \code{blpConnect} call
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character vector with Bloomberg query fields.
##' @param start.date A Date variable with the query start date.
##' @param end.date An optional Date variable with the query end date;
##' if omitted the most recent available date is used.
##' @param include.non.trading.days An optional logical variable
##' indicating whether non-trading days should be included.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param overrides An optional named character vector with override
##' values. Each field must have both a name (designating the override
##' being set) as well as a value.
##' @param identity An optional idendity object.
##' @return A list with as a entries as there are entries in
##' \code{securities}; each list contains a data.frame with one row
##' per observations and as many columns as entries in
##' \code{fields}. If the list is of length one, it is collapsed into
##' a single data frame.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   con <- blpConnect()   # adjust as needed
##'   bdh(con, "SPY US Equity", c("PX_LAST", "VOLUME"), start.date=Sys.Date()-31)
##' }
bdh <- function(con, securities, fields, start.date, end.date=NULL,
                include.non.trading.days=FALSE, options=NULL, overrides=NULL, identity=NULL) {
    if (!class(start.date) == "Date") stop("start.date must be a Date object", call.=FALSE)
    start.date <- format(start.date, format="%Y%m%d")
    if (!is.null(end.date)) {
        end.date <- format(end.date, format="%Y%m%d")
    }

    if (include.non.trading.days) {
        options <- c(options,
                     structure(c("ALL_CALENDAR_DAYS", "NIL_VALUE"),
                               names=c("nonTradingDayFillOption", "nonTradingDayFillMethod")))
    }

    res <- bdh_Impl(con, securities, fields, start.date, end.date, options, overrides, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}
