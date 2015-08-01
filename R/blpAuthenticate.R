
##' This function authenticates against the the Bloomberg API
##'
##' @title Authenticate Bloomberg API access
##' @param uuid A character variable with a unique user id token
##' @param host A character variable with a hostname, defaults to 'localhost'
##' @param ip.address An optional character variable with an IP address
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return Not sure. May just be the side effect of having the
##' session authenticated.
##' @author Whit Armstrong and Dirk Eddelbuettel

blpAuthenticate <- function(uuid, host="localhost", ip.address, con=defaultConnection()) {
    if (missing(ip.address)) {
        ## Linux only ?
        cmd.res <- system(paste("host",host), intern=TRUE,
                          ignore.stdout=FALSE, ignore.stderr=FALSE,wait=TRUE)
        ip.address <- strsplit(cmd.res,"address ")[[1]][2]
    }
    authenticate_Impl(con, as.character(uuid), ip.address)
}

#### TODO: rename to just 'authenticate' ?

