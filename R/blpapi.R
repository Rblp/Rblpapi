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

bdh <- function(conn, securities, fields, start.date, end.date=NULL, include.non.trading.days=FALSE, options=NULL, identity=NULL) {
    start.date = format(start.date, format="%Y%m%d")
    if (!is.null(end.date)) {
        end.date = format(end.date, format="%Y%m%d")
    }

    if (include.non.trading.days) {
        options <- c(options,structure(c("ALL_CALENDAR_DAYS", "NIL_VALUE"),names=c("nonTradingDayFillOption", "nonTradingDayFillMethod")))
    }

    res <- .Call("bdh", conn, securities, fields, start.date, end.date, options, identity, PACKAGE="Rblpapi")
    if(typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

bdp <- function(conn, securities, fields, options=NULL, identity=NULL) {
    if(any(duplicated(securities))) stop("duplicated securities submitted.")
    .Call("bdp", conn, securities, fields, options, identity, PACKAGE="Rblpapi")
}

bds <- function(conn, securities, field, options=NULL, identity=NULL) {
    .Call("bds", conn, securities, field, options, identity, PACKAGE="Rblpapi")
}

bar <- function(conn, security, event.type, interval, start.datetime, end.datetime, options=NULL, identity=NULL) {
    .Call("bar", conn, security, event.type, interval, start.datetime, end.datetime, options, identity, PACKAGE="Rblpapi")
}

tick <- function(conn, security, event.types, start.datetime, end.datetime, include.condition.codes=FALSE, options=NULL, identity=NULL) {
    .Call("tick", conn, security, event.types, start.datetime, end.datetime, include.condition.codes, options, identity, PACKAGE="Rblpapi")
}
