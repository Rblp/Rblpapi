
## Rblpapi [![Build Status](https://travis-ci.org/Rblp/Rblpapi.svg)](https://travis-ci.org/Rblp/Rblpapi) [![Package-License](http://img.shields.io/badge/license-GPL--3-brightgreen.svg?style=flat)](http://www.gnu.org/licenses/gpl-3.0.html) [![LibraryLicense](https://img.shields.io/badge/license-License.txt-yellow.svg?style=flat)](https://raw.githubusercontent.com/Rblp/Rblpapi/master/inst/License.txt) [![CRAN](http://www.r-pkg.org/badges/version/Rblpapi)](https://cran.r-project.org/package=Rblpapi) [![Downloads](http://cranlogs.r-pkg.org/badges/Rblpapi?color=brightgreen)](http://www.r-pkg.org/pkg/Rblpapi)

R Access to Bloomberg API

### Background

Rblpapi provides R with access to data and calculations from Bloomberg
Finance L.P. via the [API libraries](https://www.bloomberglabs.com/api/libraries/) provided by
Bloomberg at [Bloomberg Labs](https://www.bloomberglabs.com).
 

### Requirements

A valid and working Bloomberg installation.

### Examples

Here are a few simple examples.

```{.r}
library(Rblpapi)
con <- blpConnect() 	# automatic if option("blpAutoConnect") is TRUE

spx <- bdh(securities = "SPX Index", 
           fields = "PX_LAST", 
           start.date = as.Date("2013-03-01"))

spx_ndx <- bdh(securities = c("SPX Index","NDX Index"), 
               fields = "PX_LAST",
               start.date = as.Date("2013-03-01"), 
               include.non.trading.days = TRUE)

monthlyOptions <- structure(c("ACTUAL", "MONTHLY"),
                            names = c("periodicityAdjustment",
                                      "periodicitySelection"))
spx_ndx_monthly <- bdh(securities = c("SPX Index","NDX Index"), 
                       fields = "PX_LAST",
                       start.date = as.Date("2012-01-01"), 
                       options = monthly.options)

goog_ge_div <- bdh(securities = c("GOOG Equity","GE Equity"),
                   fields = c("PX_LAST","CF_DVD_PAID"), 
                   start.date = as.Date("2012-11-01"))

goog_ge_px <- bdp(securities = c("GOOG Equity","GE Equity"),
                  fields = c("PX_LAST","DS002"))
```

### Status

Fully functional on Linux, OS X and Windows.

### Installation

The package is on [CRAN](https://cran.r-project.org) and can be installed as
usual via

```r
install.packages("Rblpapi")
```

Interim (source or binary) releases _may_ be also be made available through the
[ghrr drat](http://ghrr.github.io/drat) repository as well and can be accessed via

```r
install.packages("drat")       # easier repo access + creation
drat:::add("ghrr")             # make it known
install.packages("Rblpapi")    # install it
```

### Authors

Whit Armstrong, Dirk Eddelbuettel and John Laing

### License

GPL-3 for our code

[License.txt](inst/License.txt) for the Bloomberg libraries and headers it relies upon


