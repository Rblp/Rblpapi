.cons <- new.env()
setCon <- function(con, name) {
    assign(as.character(name), con, envir=.cons)
}
getCon <- function(name="default") {
    get(name, envir=.cons)
}
