
# Copyright (C) 2016 - 2021  Dirk Eddelbuettel and Whit Armstrong
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

library(tinytest)

.runThisTest <- Sys.getenv("RunRblpapiUnitTests") == "yes"
if (!.runThisTest) exit_file("Skipping this file")

library(Rblpapi)

#test.getTicksAsDataFrame <- function() {
isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)
res <- getTicks("ESA Index", startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                endTime=Sys.time() - isweekend*48*60*60, returnAs="data.frame")
expect_true(inherits(res, "data.frame"), info = "checking return type")
expect_true(dim(res)[1] > 10, info = "check return of at least ten rows")
expect_true(dim(res)[2] == 5, info = "check return of five columns")
expect_true(all(c("times", "value", "size") %in% colnames(res)), info = "check column names")
#}

#test.getBarsAsXts <- function() {
isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)
res <- getTicks("ESA Index", startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                endTime=Sys.time() - isweekend*48*60*60, returnAs="xts")
expect_true(inherits(res, "xts"), info = "checking return type")
expect_true(dim(res)[1] > 10, info = "check return of at least ten rows")
expect_true(dim(res)[2] == 2, info = "check return of two columns")
expect_true(all(c("value", "size") %in% colnames(res)), info = "check column names")
#}

#test.getTicksAsDataTable <- function() {
isweekend <- as.POSIXlt(Sys.Date())$wday %in% c(0,6)
res <- getTicks("ESA Index", startTime=Sys.time() - isweekend*48*60*60 - 60*60,
                endTime=Sys.time() - isweekend*48*60*60, returnAs="data.table")
expect_true(inherits(res, "data.table"), info = "checking return type")
expect_true(dim(res)[1] > 10, info = "check return of at least ten rows")
expect_true(dim(res)[2] == 7, info = "check return of seven columns")
expect_true(all(c("pt", "date", "time", "type", "value", "size", "condcode") %in% colnames(res)),
            info = "check column names")
#}
