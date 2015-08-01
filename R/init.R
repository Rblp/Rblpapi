winDllPath <- function() {
    file.path(.libPaths()[1], "Rblpapi/inst/libs",
              ifelse(Sys.info()["machine"] == "x86-64",
                     "x64", "i386"))
}

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

    if (Sys.info()["sysname"] == "Windows") {
        fldrs <- strsplit(Sys.getenv("PATH"), ";")[[1]]
        # remove any existing blp locations
        # fldrs <- fldrs[!grepl("blp", fldrs)]

        pth <- paste(c(winDllPath(), fldrs), collapse = ";")

        # packageStartupMessage("Setting path to: ", pth)
        Sys.setenv(PATH=pth)
    }
}

.onDetach <- function(libpath) {
    if (Sys.info()["sysname"] == "Windows") {
        fldrs <- strsplit(Sys.getenv("PATH"), ";")[[1]]


        pth <- paste(fldrs[fldrs != winDllPath()], collapse = ";")

        # packageStartupMessage("Setting path to: ", pth)
        Sys.setenv(PATH=pth)
    }
}
