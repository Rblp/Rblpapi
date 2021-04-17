#!/usr/bin/env r
#
# Copyright (C) 2016   Dirk Eddelbuettel and Whit Armstrong
#
# This file is part of Rblpapi.
#
# Rblpapi is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Rblpapi is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

.runThisTest <- Sys.getenv("RunRblpapiUnitTests") == "yes"

if (.runThisTest) {

    test.getTicksAsDataFrame <- function() {

        isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)

        res <- getTicks("ESA Index",
                        startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                        endTime=Sys.time() - isweekend*48*60*60,
                        returnAs="data.frame")

        checkTrue(inherits(res, "data.frame"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 10, msg = "check return of at least ten rows")
        checkTrue(dim(res)[2] == 5, msg = "check return of five columns")

        checkTrue(all(c("times", "value", "size") %in% colnames(res)),
                  msg = "check column names")
    }

    test.getBarsAsXts <- function() {

        isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)

        res <- getTicks("ESA Index",
                        startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                        endTime=Sys.time() - isweekend*48*60*60,
                        returnAs="xts")

        checkTrue(inherits(res, "xts"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 10, msg = "check return of at least ten rows")
        checkTrue(dim(res)[2] == 2, msg = "check return of two columns")

        checkTrue(all(c("value", "size") %in% colnames(res)),
                  msg = "check column names")

    }

    test.getTicksAsDataTable <- function() {

        isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)

        res <- getTicks("ESA Index",
                        startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                        endTime=Sys.time() - isweekend*48*60*60,
                        returnAs="data.table")

        checkTrue(inherits(res, "data.table"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 10, msg = "check return of at least ten rows")
        checkTrue(dim(res)[2] == 7, msg = "check return of seven columns")

        checkTrue(all(c("pt", "date", "time", "type", "value", "size", "condcode") %in% colnames(res)),
                  msg = "check column names")
    }

}
