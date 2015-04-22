#!/usr/bin/Rscript

if (Sys.info()["sysname"] == "Linux") {
    setwd("/tmp")
    
    blpfile <- "http://static.bloomberglabs.com/api/cpp/blpapi_cpp_3.8.8.1-linux.tar.gz"
    download.file(blpfile)

    basefile <- basename(blpfile)
    untar(basefile)

    dirname <- gsub("-linux\\.tar\\.gz", "", basefile)
    libname <- paste0("libblpapi3_",
                      ifelse(Sys.info()["machine"] == "x86_64", "64", "32"),
                      ".so")
    libfile <- file.path(dirname, "Linux", libname)

    file.copy(libfile, "/usr/local/lib")

    system("ldconfig")
}    
