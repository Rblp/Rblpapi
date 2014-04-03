blpConnect <- function(host="localhost", port=8194L, logfile=paste("/tmp/","blpapi_",format(Sys.time(),"%Y%m%d_%H%M%S"),"_",Sys.getpid(),".log",sep="")) {
    stopifnot(storage.mode(hat)!="character" || storage.mode(port) != "integer")
    .Call("bdp_connect", host, port, logfile, PACKAGE="Rblpapi")
}

blpAuthenticate <- function(conn,uuid,host="localhost",ip.address) {
    if(missing(ip.address)) {
        cmd.res <- system(paste("host",host),intern=TRUE,ignore.stdout=FALSE,ignore.stderr=FALSE,wait=TRUE)
        ip.address <- strsplit(cmd.res,"address ")[[1]][2]
    }
    .Call("bdp_authenticate", conn, as.character(uuid), ip.address, PACKAGE="Rblpapi")
}

bdp <- function(conn, securities, fields, options=NULL, identity=NULL) {
    if(any(duplicated(securities))) stop("duplicated securities submitted.")
    .Call("bdp", conn, securities, fields, options, identity, PACKAGE="Rblpapi")
}
