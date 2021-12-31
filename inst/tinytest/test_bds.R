
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

res <- bds("DAX Index", "INDX_MEMBERS", simplify=FALSE)
expect_true(inherits(res, "list"), info = "checking return type")
expect_true(inherits(res[[1]], "data.frame"), info = "checking return type of first element")
expect_error(bds(c("DAX Index", "SPX Index"), "INDX_MEMBERS"), info = "more than one security")
expect_error(bds(c("DAX Index", "SPX Index"), c("INDX_MEMBERS", "IVOL_SURFACE")), info = "more than one security and more than one field")
expect_error(bds("DAX Index", c("INDX_MEMBERS", "IVOL_SURFACE")), info = "more than one field")

res <- bds("DAX Index", "INDX_MEMBERS")
expect_true(inherits(res, "data.frame"), info = "checking return type under simplify")
