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

    test.bdpColumnTypes <- function() {

        cols <- c("LAST_PRICE","PX_VOLUME","SECURITY_DES", "BID_YIELD")
        res <- bdp(c("TYA Comdty","ES1 Index"), cols)

        checkTrue(inherits(res, "data.frame"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] == 2, msg = "check return of two rows")
        checkTrue(dim(res)[2] == 4, msg = "check return of four cols")

        checkTrue(all(cols == colnames(res)), msg = "check column names")

        checkTrue(all(fieldInfo(cols)$datatype == c("Double", "Int32", "String", "Float")),
                  msg="check fieldInfo matches expectations")

        checkTrue(all(sapply(res, class) == c("numeric", "integer", "character", "numeric")),
                  msg="check column classes match fieldInfo")
    }

}
