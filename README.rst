************
Introduction
************

:Date: April 19, 2013
:Authors: Whit Armstrong
:Contact: armstrong.whit@gmail.com
:Web site: http://github.com/armstrtw/Rblpapi
:License: GPL-3


Purpose
=======

Rblpapi is an api to access data from Bloomberg Finance L.P.


Usage
=====

Here are a few simple examples.


::

   library(Rblpapi)
   conn <- blpConnect()

   spx <- bdh(conn,securities="SPX Index", fields="PX_LAST", start.date="20130301")
   spx.ndx <- bdh(conn,securities=c("SPX Index","NDX Index"), fields="PX_LAST", start.date="20130301",include.non.trading.days=TRUE)

   monthly.options <- structure(c("ACTUAL","MONTHLY"),names=c("periodicityAdjustment","periodicitySelection"))
   spx.ndx.monthly <- bdh(conn,securities=c("SPX Index","NDX Index"), fields="PX_LAST", start.date="20120101",options=monthly.options))

   goog.ge.div <- bdh(conn,securities=c("GOOG Equity","GE Equity"), fields=c("PX_LAST","CF_DVD_PAID"), start.date="20121101")
   goog.ge.px <- bdp(conn,securities=c("GOOG Equity","GE Equity"), fields=c("PX_LAST","DS002")
