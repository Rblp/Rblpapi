
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


