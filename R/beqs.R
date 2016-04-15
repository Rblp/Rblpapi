
##
##  Copyright (C) 2015 - 2016  Whit Armstrong and Dirk Eddelbuettel and John Laing
##
##  This file is part of Rblpapi
##
##  Rblpapi is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 2 of the License, or
##  (at your option) any later version.
##
##  Rblpapi is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.


##' This function uses the Bloomberg API to retrieve 'beqs' (Bloomberg
##' EQS Data) queries
##'
##' @title Run 'Bloomberg EQS' Queries
##' @param screenName A character string with the name of the screen
##' to execute.  It can be a user defined EQS screen or one of the
##' Bloomberg Example screens on EQS
##' @param screenType A character string of value PRIVATE or GLOBAL
##' Use PRIVATE for user-defined EQS screen.
##' Use GLOBAL for Bloomberg EQS screen.
##' @param language An optional character string with the EQS language
##' @param group An optional character string with the Screen folder
##' name as defined in EQS
##' @param date An optional Date object with the \sQuote{point in time} date
##' of the screen to execute.
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}.
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A data frame object with the date in the first column and
##' and the requested EQS data in the remaining columns.
##' @author Rademeyer Vermaak and Dirk Eddelbuettel
##' @examples
##' \dontrun{
##' head(beqs("Global Oil Companies YTD Return"), 20)
##' head(beqs("Global Oil Companies YTD Return", "GLOBAL"), 20)
##' head(beqs("Global Oil Companies YTD Return", "GLOBAL", "GERMAN"), 20)
##' head(beqs("Global Oil Companies YTD Return", "GLOBAL", "GERMAN", "GENERAL"), 20)
##' head(beqs("Global Oil Companies YTD Return", "GLOBAL", "ENGLISH", "GENERAL",
##'           as.Date("2015-09-30")), 20)
##' }
beqs <- function(screenName,
                 screenType="GLOBAL",
                 language="",
                 group="",
                 date=NULL,
                 verbose=FALSE,
                 con=defaultConnection()) {

    datestr <- if (is.null(date)) "" else format(date, "%Y%m%d")
    res <- beqs_Impl(con, screenName, screenType, group,
                     datestr, language, verbose)
    if (is.null(date)) date <- Sys.Date()
    res <- data.frame(date=date, res)
    res
}
