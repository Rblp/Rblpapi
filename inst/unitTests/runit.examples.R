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

if (.runThisTest) {
    ## don't mess with connections lest we break everything else
    #  example(blpConnect, package="Rblpapi", run.dontrun=TRUE)
    #  example(blpDisconnect, package="Rblpapi", run.dontrun=TRUE)

    ## also, don't touch subscription because it never quits:
    #  example(subscribe, package="Rblpapi", run.dontrun=TRUE)

    test.bdhExamples <- function()
        example(bdh, package="Rblpapi", run.dontrun=TRUE)

    test.bdpExamples <- function()
        example(bdp, package="Rblpapi", run.dontrun=TRUE)

    test.bdsExamples <- function()
        example(bds, package="Rblpapi", run.dontrun=TRUE)

    test.beqsExamples <- function()
        example(beqs, package="Rblpapi", run.dontrun=TRUE)

    test.bsrchExamples <- function()
        example(bsrch, package="Rblpapi", run.dontrun=TRUE)

    test.defaultConnectionExamples <- function()
        example(defaultConnection, package="Rblpapi", run.dontrun=TRUE)

    test.fieldInfoExamples <- function()
        example(fieldInfo, package="Rblpapi", run.dontrun=TRUE)

    test.fieldSearchExamples <- function()
        example(fieldSearch, package="Rblpapi", run.dontrun=TRUE)

    test.getBarsExamples <- function()
        example(getBars, package="Rblpapi", run.dontrun=TRUE)

    test.getTicksExamples <- function()
        example(getTicks, package="Rblpapi", run.dontrun=TRUE)
}
