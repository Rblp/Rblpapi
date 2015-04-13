blpConnect <- function(host=getOption("blpHost", "localhost"),
                       port=getOption("blpPort", 8194L),
                       logfile) {
    if (missing(logfile)) {
        logfile <- file.path("/tmp/",
                             paste0("blpapi_", format(Sys.time(), "%Y%m%d_%H%M%S"),
                                    "_", Sys.getpid(), ".log"))
    }
    if (storage.mode(port) != "integer") port <- as.integer(port)
    stopifnot(storage.mode(host)=="character")
    stopifnot(storage.mode(port)=="integer")
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

bdh <- function(conn, securities, fields, start.date, end.date=NULL, include.non.trading.days=FALSE, options=NULL, identity=NULL) {
    start.date = format(start.date, format="%Y%m%d")
    if (!is.null(end.date)) {
        end.date = format(end.date, format="%Y%m%d")
    }

    if (include.non.trading.days) {
        options <- c(options,structure(c("ALL_CALENDAR_DAYS", "NIL_VALUE"),names=c("nonTradingDayFillOption", "nonTradingDayFillMethod")))
    }

    res <- bdh_Impl(conn, securities, fields, start.date, end.date, options, identity)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}

bds <- function(conn, securities, fields, options=NULL, overrides=NULL, identity=NULL) {
    if(any(duplicated(securities))) stop("duplicated securities submitted.")
    res <- .Call("bds", conn, securities, fields, options, overrides, identity, PACKAGE="Rblpapi")
    if(typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}
