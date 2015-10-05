
##
##  Copyright (C) 2015  Whit Armstrong and Dirk Eddelbuettel and John Laing
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
##' @param screenName A character string with the name of the screen to execute.
##'  It can be a user defined EQS screen or one of the Bloomberg Example screens on EQS
##' @param screenType A character string of value PRIVATE or GLOBAL
##' Use PRIVATE for user-defined EQS screen.
##' Use GLOBAL for Bloomberg EQS screen.
##' @param languageID An optional character string with the EQS language
##' @param Group An optional character string with the Screen folder name as defined in EQS
##' @param PiTDate A character string with the Point in Time Date of the screen to execute.
##' Format = "YYYYMMDD"
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A matrix with requested EQS data
##' @author Whit Armstrong, Dirk Eddelbuettel
##' @examples
##' \dontrun{
##' beqs("Global Oil Companies YTD Return","GLOBAL")
##' beqs("Global Oil Companies YTD Return","GLOBAL","GERMAN")
##' beqs("Global Oil Companies YTD Return","GLOBAL","GERMAN","GENERAL")
##' beqs("Global Oil Companies YTD Return","GLOBAL","ENGLISH","GENERAL","20150930")
##' }
beqs <- function(screenName, screenType="GLOBAL", languageID=NULL,
                 Group=NULL, PiTDate=NULL, verbose=FALSE, con=defaultConnection()) {


    res <- beqs_Impl(con, screenName, screenType, Group, PiTDate, languageID, verbose)
    if (typeof(res)=="list" && length(res)==1) {
        res <- res[[1]]
    }
    res
}
