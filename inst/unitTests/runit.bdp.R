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

        res <- bdp(c("TYA Comdty","ES1 Index"), c("LAST_PRICE","PX_VOLUME","SECURITY_DES"))

        checkTrue(inherits(res, "data.frame"),
                  msg = "checking return type")

        checkTrue(dim(res)[1] == 2, msg = "check return of two rows")
        checkTrue(dim(res)[2] == 3, msg = "check return of two cols")

        checkTrue(all(c("LAST_PRICE","PX_VOLUME","SECURITY_DES") %in% colnames(res)),
                  msg = "check column names")
    }

}
