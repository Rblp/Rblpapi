
##' This function authenticates against the the Bloomberg API
##'
##' @title Authenticate Bloomberg API access
##' @param con A connection object as returned by a \code{blpConnect} call
##' @param uuid A character variable with a unique user id token
##' @param host A character variable with a hostname, defaults to 'localhost'
##' @param ip.address An optional character variable with an IP address
##' @return Not sure. May just be the side effect of having the
##' session authenticated.
##' @author Whit Armstrong and Dirk Eddelbuettel

blpAuthenticate <- function(con, uuid, host="localhost", ip.address) {
    if (missing(ip.address)) {
        ## Linux only ?
        cmd.res <- system(paste("host",host), intern=TRUE,
                          ignore.stdout=FALSE, ignore.stderr=FALSE,wait=TRUE)
        ip.address <- strsplit(cmd.res,"address ")[[1]][2]
    }
    authenticate_Impl(con, as.character(uuid), ip.address)
}

#### TODO: rename to just 'authenticate' ?

