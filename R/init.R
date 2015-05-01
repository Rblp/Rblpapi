
.pkgenv <- new.env(parent=emptyenv())

.onAttach <- function(libname, pkgname) {
    if (isTRUE(nchar(getOption("blpHost")) > 0)  &&
        isTRUE(nchar(getOption("blpPort")) > 0)) {
        con <- blpConnect()
        assign("con", con, envir=.pkgenv)
        if (getOption("blpVerbose", FALSE)) {
            packageStartupMessage(paste0("Created and stored default connection object ",
                                         "for Rblpapi version ",
                                         packageDescription("Rblpapi")$Version, "."))
        }
    }
}
