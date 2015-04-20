##' This function provides an empty stub and does not really disconnect.
##'
##' @title Placeholder function for disconnection from Bloomberg
##' @param conn A connection object
##' @return A boolean is returned; it simply states whether the
##' connection object was small or large relative to an arbitrary
##' cutoff of 1000 bytes.
##' @details The internal connection object is managed via
##' finalizers. As such the connection is only destroyed, and the
##' connection removed, once the packaged is unloaded or the session
##' is otherwise terminated.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   blpDiscnnect(con)  
##' }
blpDisconnect <- function(conn) {
    # do nothing, just return a simple test
    invisible(object.size(conn) < 1000)
}
