
# Copyright (C) 2016 - 2021  Dirk Eddelbuettel, Whit Armstrong and John Laing
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

#test.bdhColumnTypes <- function() {
res <- bdh("TY1 Comdty",c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER"),Sys.Date()-10)
expect_true(inherits(res, "data.frame"), info = "checking return type")
expect_true(dim(res)[1] >= 5, info = "check return of five rows")
expect_true(dim(res)[2] == 4, info = "check return of four cols")
expect_true(all(c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER") %in% colnames(res)), info = "check column names")
#}

#test.bdhReturnAsDT <- function() {
res <- bdh("TY1 Comdty",c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER"),Sys.Date()-10,returnAs="data.table")
expect_true(inherits(res, "data.table"), info = "checking return type - data.table")
expect_true(dim(res)[1] >= 5, info = "check return of five rows - data.table")
expect_true(dim(res)[2] == 4, info = "check return of four cols - data.table")
expect_true(all(c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER") %in% colnames(res)), info = "check column names - data.table")
#}

#test.bdhReturnAsTS <- function() {
for (retAs in c("fts", "xts", "zoo")) {
    res <- bdh("TY1 Comdty",c("PX_OPEN", "PX_HIGH", "PX_LOW", "PX_LAST"),Sys.Date()-10,returnAs=retAs)
    expect_true(inherits(res, retAs), info = paste("checking return type -", retAs))
    expect_true(dim(res)[1] >= 5, info = paste("check return of five rows -", retAs))
    expect_true(dim(res)[2] == 4, info = paste("check return of four cols -", retAs))
    expect_true(all(c("PX_OPEN", "PX_HIGH", "PX_LOW", "PX_LAST") %in% colnames(res)), info = paste("check column names -", retAs))
}
#}

#    test.bdhDateAsDouble <- function() {
res <- bdh("DOENUSCH Index","ECO_RELEASE_DT",start.date=as.Date('2016-02-01'),end.date=as.Date('2016-02-29'))
expect_true(inherits(res, "data.frame"), info = "checking return type")
expect_true(dim(res)[2] == 2, info = "check return of two cols")
expect_true(all(c("date","ECO_RELEASE_DT") %in% colnames(res)), info = "check column names")
col.types <- unique(unlist(lapply(res,class)))
expect_true(length(col.types)==1L && col.types=="Date", info = "check column types == 'Date'")
#}

#    test.bdhIntAsDouble <- function() {
expect_error(bdh("SPX Index", "PX_VOLUME", as.Date("2000-12-14"), as.Date("2000-12-15")), info = "check int overflow")
res <- bdh("SPX Index", "PX_VOLUME", as.Date("2000-12-14"), as.Date("2000-12-15"), int.as.double=TRUE)
expect_true(!is.integer(res$PX_VOLUME), info = "check volume is not integer")
expect_true(is.numeric(res$PX_VOLUME), info = "check volume is numeric")
#}
