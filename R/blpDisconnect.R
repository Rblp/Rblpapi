
##
##  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel and John Laing
##
##  This file is part of Rblpapi
##
##  Rblpapi is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 2 of the License, or
##  (at your option) any later version.
##
##  Rblpapi is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.


##' This function provides an empty stub and does not really disconnect.
##'
##' @title Placeholder function for disconnection from Bloomberg
##' @param con A connection object
##' @return A boolean is returned; it simply states whether the
##' connection object was small or large relative to an arbitrary
##' cutoff of 1000 bytes.
##' @details The internal connection object is managed via
##' finalizers. As such the connection is only destroyed, and the
##' connection removed, once the packaged is unloaded or the session
##' is otherwise terminated.
##' @author Whit Armstrong and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##'   blpDisconnect(con)
##' }
blpDisconnect <- function(con) {
    # do nothing, just return a simple test
    invisible(object.size(con) < 1000)
}
