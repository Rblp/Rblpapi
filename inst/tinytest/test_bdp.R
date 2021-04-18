
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

#test.bdpColumnTypes <- function() {
cols <- c("LAST_PRICE","PX_VOLUME","SECURITY_DES", "BID_YIELD")
res <- bdp(c("TYA Comdty","ES1 Index"), cols)
expect_true(inherits(res, "data.frame"), info = "checking return type")
expect_true(dim(res)[1] == 2, info = "check return of two rows")
expect_true(dim(res)[2] == 4, info = "check return of four cols")
expect_true(all(cols == colnames(res)), info = "check column names")
expect_true(all(fieldInfo(cols)$datatype == c("Double", "Int32", "String", "Float")),
            info = "check fieldInfo matches expectations")
expect_true(all(sapply(res, class) == c("numeric", "integer", "character", "numeric")),
            info = "check column classes match fieldInfo")
#    }

#test.naDate <- function() {
res <- bdp("BBG006YQMFQ5", "ISSUE_DT")
expect_true(is.na(res$ISSUE_DT), info = "checking NA date value")
#}
