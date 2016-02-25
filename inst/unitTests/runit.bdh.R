#!/usr/bin/env r
# hey emacs, please make this use  -*- tab-width: 4 -*-
#
# Copyright (C) 2016   Dirk Eddelbuettel, Whit Armstrong and John Laing
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

.setUp <- function() {
    connectionParameterFile <- "~/.R/rblpapiOptions.R"
    if (file.exists(connectionParameterFile)) {
        source(connectionParameterFile)
        Rblpapi::blpConnect()
    } else {
        .runThisTest <- FALSE
    }
}

if (.runThisTest) {

    test.bdhColumnTypes <- function() {

        res <- bdh("TY1 Comdty",c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER"),Sys.Date()-10)

        checkTrue(inherits(res, "data.frame"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 5, msg = "check return of five rows")
        checkTrue(dim(res)[2] == 4, msg = "check return of four cols")

        checkTrue(all(c("PX_LAST","OPEN_INT","FUT_CUR_GEN_TICKER") %in% colnames(res)),
                  msg = "check column names")

    }

}
