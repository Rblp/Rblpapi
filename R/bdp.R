
##' This function uses the Bloomberg API to retrieve 'bdp' (Bloomberg
##' Data Point) queries
##'
##' @title Run 'Bloomberg Data Point' Queries
##' @param securities A character vector with security symbols in
##' Bloomberg notation.
##' @param fields A character vector with Bloomberg query fields.
##' @param options An optional named character vector with option
##' values. Each field must have both a name (designating the option
##' being set) as well as a value.
##' @param overrides An optional named character vector with override
##' values. Each field must have both a name (designating the override
##' being set) as well as a value.
##' @param identity An optional idendity object.
##' @param con A connection object as returned by a \code{blpConnect} call
##' @return A data frame with as a many rows as entries in
##' \code{securities} and columns as entries in \code{fields}.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   con <- blpConnect()   # adjust as needed
##'   bdp(con, c("ESA Index", "SPY US Equity"), c("PX_LAST", "VOLUME"))
##' }
bdp <- function(securities, fields, options=NULL, overrides=NULL, identity=NULL, con=.pkgenv$con) {
    if (any(duplicated(securities))) stop("Duplicated securities submitted.", call.=FALSE)
    bdp_Impl(con, securities, fields, options, overrides, identity)
}

