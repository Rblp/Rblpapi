
##' This function uses the Bloomberg API to retrieve 'bdp' (Bloomberg
##' Data Point) queries
##'
##' @title Run 'Bloomberg Data Point' Queries
##' @param con A connection object as returned by a \code{blpConnect} call
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character string with a single Bloomberg query field.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param overrides An optional override vector.
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
##'   bdp(con, c("ESA Index", "SPY US Equity"), c("PX_LAST", "VOLUME"))
##' }
bds <- function(con, securities, fields, options=NULL, overrides=NULL, identity=NULL) {
    if (any(duplicated(securities))) stop("duplicated securities submitted.")
    res <- bds_Impl(con, securities, fields, options, overrides, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

