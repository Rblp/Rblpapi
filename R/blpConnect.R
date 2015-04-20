##' This function connects to the Bloomberg API
##'
##' @title Establish connection to Bloomberg service
##' @param host A character option with either a machine name that is resolvable by DNS, or an IP address. Defaults to \sQuote{localhost}.
##' @param port An integer variable with the connection port. Default to \code{8194L}.
##' @return A connection object is returned. It is required by all the
##' accessor functions in th epackage.
##' @details For both \code{host} and \code{port} argument, default
##' values can also be specified via \code{\link{options}} using,
##' respectively, the named entries \code{blpHost} and
##' \code{blpConnect}.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   con <- blpConnect()   # adjust as needed
##' }
blpConnect <- function(host=getOption("blpHost", "localhost"),
                       port=getOption("blpPort", 8194L)) {
    if (storage.mode(port) != "integer") port <- as.integer(port)
    if (storage.mode(host) != "character") stop("Host argument must be character.", call.=FALSE)
    blpConnect_Impl(host, port)
}
