
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

spx <- bdh(securities = "SPX Index", 
           fields = "PX_LAST", 
           start.date = as.Date("2013-03-01"))

spx.ndx <- bdh(securities = c("SPX Index","NDX Index"), 
               fields = "PX_LAST",
               start.date = as.Date("2013-03-01"), 
               include.non.trading.days = TRUE)

monthly.options <- structure(c("ACTUAL", "MONTHLY"),
                             names = c("periodicityAdjustment",
                                       "periodicitySelection"))
spx.ndx.monthly <- bdh(securities = c("SPX Index","NDX Index"), 
                       fields = "PX_LAST",
                       start.date = as.Date("2012-01-01"), 
                       options = monthly.options)

goog.ge.div <- bdh(securities = c("GOOG Equity","GE Equity"),
                   fields = c("PX_LAST","CF_DVD_PAID"), 
                   start.date = as.Date("2012-11-01"))

goog.ge.px <- bdp(securities = c("GOOG Equity","GE Equity"),
                  fields = c("PX_LAST","DS002"))
```

### Status

Fully functional on Linux and Windows. 

### Authors

Whit Armstrong, Dirk Eddelbuettel and John Laing

### License

GPL-3
