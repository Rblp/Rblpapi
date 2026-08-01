
## See e.g. github package 'eddelbuettel/tellme' at r-universe for output on
## various platform; there we aim at windows-arm64 for which we have Blp library
if (R.version$platform == "aarch64" && R.version$os == "mingw32") {
    file.copy("src/Makevars.no_nlp", "src/Makevars.win")
} else {
    file.copy("src/Makevars.win.in", "src/Makevars.win")

}
