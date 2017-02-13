
##
##  Copyright (C) 2017  Dirk Eddelbuettel and Kevin Jin
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

##' This function uses the Bloomberg API to look up tickers and descriptions
##' given the name of a company.
##'
##' @title Look up symbol from Bloomberg
##' @param query A character variable describing the name of the company; for
##' certain queries a trailing space may help.
##' @param yellowkey A character variable that restricts the asset classes
##' to search in; one of \dQuote{none}, \dQuote{cmdt}, \dQuote{eqty}, \dQuote{muni},
##' \dQuote{prfd}, \dQuote{clnt}, \dQuote{mmkt}, \dQuote{govt}, \dQuote{corp},
##' \dQuote{indx}, \dQuote{curr}, \dQuote{mtge}.
##' @param language A character variable denoting the language that the
##' results will be translated in; one of \dQuote{NONE},
##' \dQuote{english}, \dQuote{kanji}, \dQuote{french},
##' \dQuote{german}, \dQuote{spanish}, \dQuote{portuguese},
##' \dQuote{italian}, \dQuote{chinese_trad}, \dQuote{korean},
##' \dQuote{chinese_simp}, \dQuote{none_1}, \dQuote{none_2},
##' \dQuote{none_3}, \dQuote{none_4}, \dQuote{none_5},
##' \dQuote{russian}
##' @param maxResults A integer variable containing a value by which to limit
##' the search length
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @param con A connection object as created by a \code{blpConnect}
##' call, and retrieved via the internal function
##' \code{defaultConnection}.
##' @return A data.frame with two columns of the ticker and description of each
##' match.
##' @author Kevin Jin and Dirk Eddelbuettel
##' @examples \dontrun{
##'   lookupSecurity("IBM")
##'   lookupSecurity("IBM", maxResuls=1000)    # appears to be capped at 1000
##'   lookupSecurity("IBM", "mtge")
##'   lookupSecurity("IBM ", "mtge")           # trailing space affects query
##' }
lookupSecurity <- function(query,
                           yellowkey = c("none", "cmdt", "eqty", "muni", "prfd", "clnt", "mmkt",
                                         "govt", "corp", "indx", "curr", "mtge"),
                           language = c("none", "english", "kanji", "french", "german", "spanish",
                                        "portuguese", "italian", "chinese_trad", "korean",
                                        "chinese_simp", "none_1", "none_2", "none_3", "none_4",
                                        "none_5", "russian"),
                   maxResults = 20,
                   verbose = FALSE,
                   con = defaultConnection()) {
    yellowkey <- match.arg(yellowkey)
    language <- match.arg(language)

    yellowkey <- paste("YK", "FILTER", toupper(yellowkey), sep="_")
    language <- paste("LANG", "OVERRIDE", toupper(language), sep="_")
    lookup_Impl(con, query, yellowkey, language, maxResults, verbose)
}
