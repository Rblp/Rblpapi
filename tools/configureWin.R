
## See e.g. github package 'eddelbuettel/tellme' at r-universe for output on
## various platform; there we aim at windows-arm64 for which we have Blp library
cat("+++ hello from configureWin.R\n")
print(R.version)
if (R.version$platform == "aarch64" && R.version$os == "mingw32") {
    cat("+++ hello from configureWin.R -- aarch/arm64\n")
    rc <- file.copy("src/Makevars.no_nlp", "src/Makevars.win")
} else {
    cat("+++ hello from configureWin.R -- amd64\n")
    rc <- file.copy("src/Makevars.win.in", "src/Makevars.win")
}
