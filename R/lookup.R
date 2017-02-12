
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
##' @param query A character variable describing the name of the company
##' @param yellowKeyFilter A character variable that restricts the asset classes
##' to search in; one of \dQuote{YK_FILTER_NONE}, \dQuote{YK_FILTER_CMDT},
##' \dQuote{YK_FILTER_EQTY}, \dQuote{YK_FILTER_MUNI}, \dQuote{YK_FILTER_PRFD},
##' \dQuote{YK_FILTER_CLNT}, \dQuote{YK_FILTER_MMKT}, \dQuote{YK_FILTER_GOVT},
##' \dQuote{YK_FILTER_CORP}, \dQuote{YK_FILTER_INDX}, \dQuote{YK_FILTER_CURR},
##' or \dQuote{YK_FILTER_MTGE}
##' @param languageOverride A character variable denoting the language that the
##' results will be translated in; one of \dQuote{LANG_OVERRIDE_NONE},
##' \dQuote{LANG_OVERRIDE_ENGLISH}, \dQuote{LANG_OVERRIDE_KANJI},
##' \dQuote{LANG_OVERRIDE_FRENCH}, \dQuote{LANG_OVERRIDE_GERMAN},
##' \dQuote{LANG_OVERRIDE_SPANISH}, \dQuote{LANG_OVERRIDE_PORTUGUESE},
##' \dQuote{LANG_OVERRIDE_ITALIAN}, \dQuote{LANG_OVERRIDE_CHINESE_TRAD},
##' \dQuote{LANG_OVERRIDE_KOREAN}, \dQuote{LANG_OVERRIDE_CHINESE_SIMP},
##' \dQuote{LANG_OVERRIDE_NONE_1}, \dQuote{LANG_OVERRIDE_NONE_2},
##' \dQuote{LANG_OVERRIDE_NONE_3}, \dQuote{LANG_OVERRIDE_NONE_4},
##' \dQuote{LANG_OVERRIDE_NONE_5}, \dQuote{LANG_OVERRIDE_RUSSIAN}
##' @param maxResults A integer variable containing a value by which to limit
##' the search length
##' @param verbose A boolean indicating whether verbose operation is
##' desired, defaults to \sQuote{FALSE}
##' @return A data.frame with two columns of the ticker and description of each
##' match.
##' @author Kevin Jin
lookup <- function(query,
                     yellowKeyFilter = "YK_FILTER_NONE",
                     languageOverride = "LANG_OVERRIDE_NONE",
                     maxResults = 20,
                     verbose = FALSE,
                     con = defaultConnection()) {
    lookup_Impl(con, query, yellowKeyFilter, languageOverride, maxResults,
                verbose)
}
