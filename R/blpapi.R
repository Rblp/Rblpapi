blpConnect <- function(host="localhost", port=8194L, logfile=paste("/tmp/","blpapi_",format(Sys.time(),"%Y%m%d_%H%M%S"),"_",Sys.getpid(),".log",sep="")) {
    stopifnot(storage.mode(hat)!="character" || storage.mode(port) != "integer")
    .Call("bdp_connect", host, port, logfile, PACKAGE="Rblpapi")
}

blpAuthenticate <- function(conn,uuid,host="localhost",ip.address) {
    if(missing(ip.address)) {
        cmd.res <- system(paste("host",host),intern=TRUE,ignore.stdout=FALSE,ignore.stderr=FALSE,wait=TRUE)
        ip.address <- strsplit(cmd.res,"address ")[[1]][2]
    }
    .Call("bdp_authenticate", conn, uuid, ip.address, PACKAGE="Rblpapi")
}

bdh <- function(conn, securities, fields, start.date, end.date=NULL, include.non.trading.days=FALSE, options=NULL, identity=NULL) {
    start.date = format(start.date, format="%Y%m%d")
    if (!is.null(end.date)) {
        end.date = format(end.date, format="%Y%m%d")
    }

    if (include.non.trading.days) {
        options <- c(options,structure(c("ALL_CALENDAR_DAYS", "NIL_VALUE"),names=c("nonTradingDayFillOption", "nonTradingDayFillMethod")))
    }

    .Call("bdh", conn, securities, fields, start.date, end.date, options, identity, PACKAGE="Rblpapi")
}

bdp <- function(conn, securities, fields, options=NULL, identity=NULL) {
    .Call("bdp", conn, securities, fields, options, identity, PACKAGE="Rblpapi")
}
