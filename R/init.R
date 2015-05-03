
.pkgenv <- new.env(parent=emptyenv())

.onAttach <- function(libname, pkgname) {
    if (getOption("blpAutoConnect", FALSE)) {
        con <- blpConnect()
        assign("con", con, envir=.pkgenv)
        if (getOption("blpVerbose", FALSE)) {
            packageStartupMessage(paste0("Created and stored default connection object ",
                                         "for Rblpapi version ",
                                         packageDescription("Rblpapi")$Version, "."))
        }
    }
}
