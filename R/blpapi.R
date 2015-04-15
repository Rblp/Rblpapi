blpConnect <- function(host=getOption("blpHost", "localhost"),
                       port=getOption("blpPort", 8194L),
                       logfile) {
    if (missing(logfile)) {
        logfile <- file.path("/tmp/",
                             paste0("blpapi_", format(Sys.time(), "%Y%m%d_%H%M%S"),
                                    "_", Sys.getpid(), ".log"))
    }
    if (storage.mode(port) != "integer") port <- as.integer(port)
    if (storage.mode(host) != "character") stop("Host argument must be character.", call.=FALSE)
    .Call("bdp_connect", host, port, logfile, PACKAGE="Rblpapi")
}

blpDisconnect <- function(conn) {
    # do nothing, just return a simple test
    invisible(object.size(conn) < 1000)
}

blpAuthenticate <- function(conn,uuid,host="localhost",ip.address) {
    if(missing(ip.address)) {
        cmd.res <- system(paste("host",host),intern=TRUE,ignore.stdout=FALSE,ignore.stderr=FALSE,wait=TRUE)
        ip.address <- strsplit(cmd.res,"address ")[[1]][2]
    }
    .Call("bdp_authenticate", conn, as.character(uuid), ip.address, PACKAGE="Rblpapi")
}


