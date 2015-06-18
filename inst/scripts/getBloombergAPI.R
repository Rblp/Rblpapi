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
} else if (Sys.info()["sysname"] == "Windows") {
    unlink("inst/win")

    tmpdir = tempdir()

    blpfile <- "http://static.bloomberglabs.com/api/cpp/blpapi_cpp_3.7.9.1-windows.zip"
    basefile <- basename(blpfile)

    tmpfile <- file.path(tmpdir, basefile)
    download.file(blpfile, tmpfile)
    unzip(tmpfile, exdir = tmpdir)

    dirname <- gsub("-windows\\.zip", "", basefile)
    libname <- paste0("blpapi3_",
                      ifelse(Sys.info()["machine"] == "x86-64", "64", "32"),
                      ".dll")
    libfile <- file.path(tmpdir, dirname, "lib", libname)

    if (!file.exists("inst/win"))
        dir.create("inst/win")
    file.copy(libfile, "inst/win")

    inclfolder <- file.path(tmpdir, dirname, "include")
    inclfiles <- list.files(inclfolder, full.names = TRUE)

    if (!file.exists("inst/win/include"))
        dir.create("inst/win/include")
    file.copy(inclfiles, "inst/win/include")
}
