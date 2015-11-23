\name{NEWS}
\title{News for Package 'Rblpapi}
\newcommand{\ghrepo}{\href{https://github.com/#1}{\pkg{#1}}}
\newcommand{\ghpr}{\href{https://github.com/Rblp/Rblpapi/pull/#1}{##1}}
\newcommand{\ghit}{\href{https://github.com/Rblp/Rblpapi/issues/#1}{##1}}

\section{Changes in Rblpapi version 0.3.2 (2015-12-31)}{
  \itemize{
    \item A new \code{subscribe()} function supports live data
    subscription for a set of tickers and fields (\ghpr{88}).
    \item Both \code{getTicks()} and \code{getBars()} now check the
    start and end times for proper \code{Datetime} class.
    \item The \code{getBars()} function now also return the
    \sQuote{value traded} (request of \ghit{89}).
    \item The \code{beqs()} function is now more robust with respect to
    empty return columns (PR \ghpr{100} by Rademeyer fixing ticket
    \ghit{99}).
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
