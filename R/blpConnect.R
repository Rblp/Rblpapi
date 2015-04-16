blpConnect <- function(host=getOption("blpHost", "localhost"),
                       port=getOption("blpPort", 8194L)) {
    if (storage.mode(port) != "integer") port <- as.integer(port)
    if (storage.mode(host) != "character") stop("Host argument must be character.", call.=FALSE)
    blpConnect_Impl(host, port)
}
