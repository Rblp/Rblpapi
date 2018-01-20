\name{NEWS}
\title{News for Package \pkg{Rblpapi}}
\newcommand{\ghrepo}{\href{https://github.com/#1}{\pkg{#1}}}
\newcommand{\ghpr}{\href{https://github.com/Rblp/Rblpapi/pull/#1}{##1}}
\newcommand{\ghit}{\href{https://github.com/Rblp/Rblpapi/issues/#1}{##1}}

\section{Changes in Rblpapi version 0.3.7 (2018-01-20)}{
  \itemize{
    \item The 140 day limit for intra-day data histories is now
    mentioned in the \code{getTicks} help (Dirk in \ghpr{226} addressing
    \ghit{215} and \ghit{225}).
    \item The Travis CI script was updated to use \code{run.sh} (Dirk in \ghpr{226}).
    \item The \code{install_name_tool} invocation under macOS was
    corrected (@spennihana in \ghit{232})
    \item The \code{blpAuthenticate} help page has additional examples
    (@randomee in \ghpr{252}).
    \item The \code{blpAuthenticate} code was updated and improved (Whit
    in \ghpr{258} addressing \ghpr{257})
  }
}

\section{Changes in Rblpapi version 0.3.6 (2017-04-20)}{
  \itemize{
    \item \code{bdh} can now store in \code{double} preventing overflow
    (Whit and John in \ghpr{205} closing \ghit{163})
    \item \code{bdp} documentation has another ovveride example
    \item A new function \code{lookupSecurity} can search for securities,
    optionally filtered by yellow key (Kevin Jin and Dirk in \ghpr{216}
    and \ghpr{217} closing \ghit{215})
    \item Added file \code{init.c} with calls to \code{R_registerRoutines()}
    and \code{R_useDynamicSymbols()}; also use \code{.registration=TRUE}
    in \code{useDynLib} in \code{NAMESPACE} (Dirk in \ghpr{220})
    \item \code{getBars} and \code{getTicks} can now return
    \code{data.table} objects (Dirk in \ghpr{221})
    \item \code{bds} has improved internal protect logic via
    \code{Rcpp::Shield} (Dirk in \ghpr{222})
  }
}

\section{Changes in Rblpapi version 0.3.5 (2016-10-25)}{
  \itemize{
    \item Add new function \code{getPortfolio} to retrieve portfolio data
    via \code{bds} (John in \ghpr{176}) 
    \item Extend \code{getTicks()} to (optionally) return non-numeric
    data as part of \code{data.frame} or \code{data.table} (Dirk in
    \ghpr{200})
    \item Similarly extend \code{getMultipleTicks} (Dirk in \ghpr{202})
    \item Correct statement on timestamp for \code{getBars} (Closes
    issue \ghit{192})
    \item Minor edits to a few files in order to either please
    \code{R(-devel) CMD check --as-cran}, or update documentation
    \item Adding registration of routines are R 3.4.0 now desires.
    \item Optional export as \code{data.table} is now more standardized.
  }
}

\section{Changes in Rblpapi version 0.3.4 (2016-05-08)}{
  \itemize{
    \item On startup, the API versions of both the headers and the
    runtime are displayed (PR \ghpr{161} and \ghpr{165}).
    \item Documentation about extended futures roll notation was added
    to the \code{bdh} manul page.
    \item Additional examples for overrides where added to \code{bdh}
    (PR \ghpr{158}).
    \item Internal code changes make retrieval of data in
    \sQuote{unusual} variable types more robust (PRs \ghpr{157} and \ghpr{153})
    \item General improvements and fixes to documentation (PR \ghpr{156})
    \item The \code{bdp} function now also supports an option
    \code{verbose} (PR \ghpr{149}).
    \item The internal header \code{Rblpapi_types.h} was renamed from a
    lower-cased variant to conform with Rcpp Attributes best
    practices (PR \ghpr{145}).
  }
}

\section{Changes in Rblpapi version 0.3.3 (2016-03-14)}{
  \itemize{
    \item \code{configure} adds a missing \code{method="libcurl"}
    options (PR \ghpr{109} by Martin Bel).
    \item New function \code{bsrch()} adds basic search functionality
    (PR \ghpr{111} by Morgan Williams fixing ticket \ghit{82}).
    \item The licensing status of the Rblpapi source package was
    clarified (PR \ghpr{119}).
    \item The \code{bds()} help page now shows an example using an
    \code{overrides} argument (PR \ghpr{121}).
    \item A new function \code{fieldInfo} was added, and field
    information is used in \code{bdh} and \code{bdp} (PRs \ghpr{122},
    \ghpr{123}, \ghpr{125}, and \ghpr{127}).
    \item The \code{bdp} function now checks the order (PR \ghpr{126}).
    \item Unit testing was added (PRs \ghpr{129} and \ghpr{131}).
    \item Data retrieval is now smarter about the types returned from
    Bloomberg (PR \ghpr{132} and \ghpr{133}; and \ghpr{141} and \ghpr{142}).
    \item The \code{bdh} and \code{bds} functions now support an option
    \code{verbose} (PRs \ghpr{138}).
  }
}

\section{Changes in Rblpapi version 0.3.2 (2015-12-06)}{
  \itemize{
    \item A new \code{subscribe()} function supports live data
    subscription for a set of tickers and fields (\ghpr{88}).
    \item In the \code{beqs()} example the correct date format is used
    (PR \ghpr{85} by Rademeyer).
    \item Both \code{getTicks()} and \code{getBars()} now check the
    start and end times for proper \code{Datetime} class.
    \item The \code{getBars()} function now also return the
    \sQuote{value traded} (request of \ghit{89}).
    \item An error in the documentation for \code{bdh} was corrected (PR
    \ghpr{93} by Rademeyer closing \ghit{85}).
    \item The \code{beqs()} function is now more robust with respect to
    empty return columns (PR \ghpr{100} by Rademeyer fixing ticket
    \ghit{99}).
    \item The \code{getBars()} function now uses a single (named) vector
    \code{options} to pass optional values to the underlying function
    (PR \ghpr{105} updating PR \ghpr{48} and fixing \ghit{47}). 
    \item When R is built with libcurl support, it used to download the
    build-time library and headers; otherwise \code{curl} is used. This
    avoids an issue on OS X where \code{curl} is insufficient.
  }
}

\section{Changes in Rblpapi version 0.3.1 (2015-10-19)}{
  \itemize{
    \item Added \code{beqs()} to run Bloomberg Equity Screen Search,
    based on initial PR \ghpr{79} by Rademeyer Vermaak, reworked in PRs
    \ghpr{83} and \ghpr{84} by Dirk; closes tickets \ghit{63} and \ghit{80}.
    \item Bloomberg header and library files are now cached locally
    instead of being re-downloaded for every build (PR \ghpr{78}
    by Dirk addressing issue \ghpr{65}).
    \item For multiple ticks, time-stamps are unique-yfied before
    merging (PR \ghpr{77} by Dirk addressing issue \ghit{76}).
    \item A compiler warning under new g++ versions is now suppressed
    (PR \ghpr{69} by John, fixing \ghit{68}).
  }
}

\section{Changes in Rblpapi version 0.3.0 (2015-08-13)}{
  \itemize{
    \item First CRAN Release
    \item Source and binary builds supported on Linux, Windows and OS X
    \item Build procedures rewritten so that required headers and
    libraries are downloaded during installation from corresponding GitHub
    repo \ghrepo{Rblp/blp} which contains custom tarballs derived from the
    corresponding upstream releases at \url{http://www.bloomberglabs.com/api}.
  }
}
