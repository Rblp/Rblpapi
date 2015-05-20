
##' This function searches for matching Bloomberg data fields given a search term.
##'
##' @title Search for matching data fields
##'
##' @param con A connection object as returned by a \code{blpConnect} call
##' @param searchterm A string with the term to search for
##' @param excludeterm A string with an expression for matches to excludes, defaults to \dQuote{Static}
##'
##' @return A \code{data.frame} with three columns of the id, mnenemonic and description of each match.
##'
##' @author Dirk Eddelbuettel
##'
##' @examples
##' \dontrun{
##'   con <- blpConnect()
##'   res <- fieldSearch("vwap")
##' }
##'
fieldSearch <- function(searchterm, excludeterm="Static", con=.pkgenv$con) {
    fieldSearch_Impl(con, searchterm, excludeterm)
}
