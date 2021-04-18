## Copyright (C) 2016 - 2021  Dirk Eddelbuettel and Whit Armstrong
## Based on earlier versions in Rcpp, RProtoBuf and other packages
##
## This file is part of Rblpapi
##
## Rblpapi is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 2 of the License, or
## (at your option) any later version.
##
## Rblpapi is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

## Set a seed to make the test deterministic (though currently to random data)
set.seed(42)

## We need to source an extra parameter file to support a Bloomberg connection
## For live sessions, we use ~/.Rprofile but that file is not read by R CMD check
## The file basically just calls options() and sets options as needed for blpHost,
## blpPort, blpAutoConnect (to ensure blpConnect() is called on package load) and,
## as tested for below, blpUnitTests.
connectionParameterFile <- "~/.R/rblpapiOptions.R"
if (file.exists(connectionParameterFile)) source(connectionParameterFile)

## if an option is set, we run tests. otherwise we don't.
## recall that we DO need a working Bloomberg connection...
if (getOption("blpUnitTests", FALSE)) {

    ## without this, we get (or used to get) unit test failures
    Sys.setenv("R_TESTS"="")

    ## set an option the test files check for
    Sys.setenv("RunRblpapiUnitTests" = "yes")

    ## standard test suite call provided tinytest is present
    if (requireNamespace("tinytest", quietly=TRUE)) {
        tinytest::test_package("Rblpapi")
    }
}
