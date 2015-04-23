
## Rblpapi [![Build Status](https://travis-ci.org/eddelbuettel/Rblpapi.png)](https://travis-ci.org/eddelbuettel/Rblpapi) [![License](http://img.shields.io/badge/license-GPL%20%28%3E=%203%29-brightgreen.svg?style=flat)](http://www.gnu.org/licenses/gpl-3.0.html)

R Access to Bloomberg API

### Background

Rblpapi provides R with API access to data and calculations from Bloomberg
Finance L.P. 

### Requirements

A valid and working Bloomberg installation, and the Bloomberg API
[libraries](http://www.bloomberglabs.com/api/libraries/).

### Examples

Here are a few simple examples.

```{.r}
library(Rblpapi)
con <- blpConnect()

spx <- bdh(con,securities="SPX Index", fields="PX_LAST", start.date="20130301")
spx.ndx <- bdh(conn,securities=c("SPX Index","NDX Index"), fields="PX_LAST",
               start.date="20130301", include.non.trading.days=TRUE)

monthly.options <- structure(c("ACTUAL", "MONTHLY"),
                             names=c("periodicityAdjustment","periodicitySelection"))
spx.ndx.monthly <- bdh(con,securities=c("SPX Index","NDX Index"), fields="PX_LAST",
                       start.date="20120101", options=monthly.options))

goog.ge.div <- bdh(con, securities=c("GOOG Equity","GE Equity"),
                   fields=c("PX_LAST","CF_DVD_PAID"), start.date="20121101")
goog.ge.px <- bdp(con, securities=c("GOOG Equity","GE Equity"),
                  fields=c("PX_LAST","DS002")
```

### Status

Fully functional on Linux. 

### Authors

Whit Armstrong, Dirk Eddelbuettel and John Laing

### License

GPL-3
