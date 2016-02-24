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

    test.getBarsAsMatrix <- function() {

        res <- getBars("ES1 Index", returnAs="matrix")

        checkTrue(inherits(res, "data.frame"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 3, msg = "check return of at least three rows")
        checkTrue(dim(res)[2] == 8, msg = "check return of eight columns")

        checkTrue(all(c("times", "open", "high", "low", "close") %in% colnames(res)),
                  msg = "check column names")

    }

    test.getBarsAsXts <- function() {

        res <- getBars("ES1 Index", returnAs="xts")

        checkTrue(inherits(res, "xts"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] > 3, msg = "check return of at least three rows")
        checkTrue(dim(res)[2] == 7, msg = "check return of seven columns")

        checkTrue(all(c("open", "high", "low", "close") %in% colnames(res)),
                  msg = "check column names")

    }
}
