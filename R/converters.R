
##  Copyright (C) 2017         Whit Armstrong and Dirk Eddelbuettel and John Laing
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

## not exported (yet) so no docs (yet)
asDataTable <- function(res, keepPOSIXct=TRUE) {

    if (!requireNamespace("data.table", quietly=TRUE)) stop("Need data.table to convert", call.=FALSE)

    pt <- time <- `:=` <- NULL		# silly R CMD check warning

    data.table::setNumericRounding(0)   # important to not truncate

    ## we assume res is a matrix with a POSIXct in column one
    dt <- data.table::data.table(pt=res[,1], 		             # keep POSIXct
                                 date=data.table::as.IDate(res[,1]), # just Date
                                 time=data.table::as.ITime(res[,1]), # just Time
                                 res[, -1])                          # remainder
    data.table::setkey(dt, pt, date, time)

    if (!keepPOSIXct) dt[, pt := NULL]

    dt
}
