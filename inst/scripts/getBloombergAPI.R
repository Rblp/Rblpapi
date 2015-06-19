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
    tmpdir = tempdir()

    blpfile <- "http://static.bloomberglabs.com/api/cpp/blpapi_cpp_3.7.9.1-windows.zip"
    basefile <- basename(blpfile)

    tmpfile <- file.path(tmpdir, basefile)
    download.file(blpfile, tmpfile)
    unzip(tmpfile, exdir = tmpdir)

    dirname <- gsub("-windows\\.zip", "", basefile)
    
    for (x in list(list(dll = "32", path = "i386"),
                   list(dll = "64", path = "x64"))) {
      libname <- paste0("blpapi3_", x$dll, ".dll")
      libfile <- file.path(tmpdir, dirname, "lib", libname)
  
      dllpath <- file.path("inst/lib", x$path)
      if (!file.exists(dllpath))
        dir.create(dllpath)
      file.copy(libfile, dllpath)
    }
    
    inclfolder <- file.path(tmpdir, dirname, "include")
    inclfiles <- list.files(inclfolder, full.names = TRUE)

    unlink("inst/win")
    if (!file.exists("inst/win/include"))
        dir.create("inst/win/include")
    file.copy(inclfiles, "inst/win/include")
}
