
## See e.g. github package 'eddelbuettel/tellme' at r-universe for output on
## various platform; there we aim at windows-arm64 for which we have Blp library
if (R.version$arch == "aarch64" && R.version$os == "mingw32") {
    ## windows-arm does not have a Blp library so we use the 'no blp' Makevars
    rc <- file.copy("src/Makevars.no_nlp", "src/Makevars.win")
} else {
    ## else use the (old) default build (still accounting for 32 bit)
    rc <- file.copy("src/Makevars.win.in", "src/Makevars.win")
}
