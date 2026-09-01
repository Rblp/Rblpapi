# Copyright (C) 2025  Dirk Eddelbuettel, Whit Armstrong and John Laing
#
# This file is part of Rblpapi.
#
# Rblpapi is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Rblpapi is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

library(tinytest)
library(Rblpapi)

## every JSON parser which is installed is tested, and all of them must give
## the very same result
.parsers <- Filter(function(p) requireNamespace(p, quietly=TRUE),
                   Rblpapi:::.bqlParsers)
if (length(.parsers) == 0L)
    exit_file("Skipping as no JSON parser is available")

.readFixture <- function(file) {
    paste(readLines(file.path("bql", file), warn=FALSE), collapse="\n")
}
.parse <- function(file, ...) Rblpapi:::.bqlParse(.readFixture(file), ...)

## the parser in use labels every assertion; .p is resolved at call time
.with <- function(txt) paste0(txt, " [", .p, "]")

## compare outcomes and not only successes: a fixture which must raise has to
## raise the same way through every parser and every chunking
.outcome <- function(x, p) tryCatch(suppressWarnings(Rblpapi:::.bqlParse(x, parser=p)),
                                    error = function(e) conditionMessage(e))

.allIdentical <- function(x) all(vapply(x[-1], identical, logical(1), x[[1]]))

## one skeleton for the single-item documents built by hand below; 'values' is
## a vector of JSON literals, one per row, so quote any string value yourself
.oneItemDoc <- function(type, values, name="v") {
    paste0('{"results":{"', name, '":{"name":"', name, '","idColumn":{"name":"ID",',
           '"type":"STRING","values":[',
           paste0('"', seq_along(values), '"', collapse=","), ']},',
           '"valuesColumn":{"name":"VALUE","type":"', type, '","values":[',
           paste(values, collapse=","), ']},"secondaryColumns":[]}},',
           '"responseExceptions":[]}')
}

## -- offline parsing tests (no Bloomberg connection required) --------------

for (.p in .parsers) {

    ## single-item query: one data.frame with declared column types
    res <- .parse("response_px_last.json", parser=.p)
    expect_true(inherits(res, "data.frame"), info = .with("single item simplifies to data.frame"))
    expect_equal(dim(res), c(3L, 4L), info = .with("three rows, four columns"))
    expect_equal(colnames(res), c("ID", "px_last", "DATE", "CURRENCY"), info = .with("column names"))
    expect_equal(unname(sapply(res, class)), c("character", "numeric", "Date", "character"),
                 info = .with("column types follow declared JSON types"))
    expect_equal(res$px_last[1:2], c(229.33, 254.49), info = .with("numeric values"))
    ## expect_identical, not expect_equal: all.equal() treats NaN as equal to
    ## NA_real_, so expect_equal would pass even if the placeholder handling
    ## were removed altogether and as.numeric("NaN") left a NaN behind
    expect_identical(res$px_last[3], NA_real_, info = .with("string 'NaN' becomes NA, not NaN"))
    expect_true(is.na(res$DATE[3]) && is.na(res$CURRENCY[3]), info = .with("JSON null becomes NA"))
    expect_equal(res$DATE[1], as.Date("2024-12-17"), info = .with("DATE conversion"))

    ## simplify=FALSE keeps the list shape
    res <- .parse("response_px_last.json", simplify=FALSE, parser=.p)
    expect_true(is.list(res) && length(res) == 1L && names(res) == "px_last",
                info = .with("simplify=FALSE returns named list"))

    ## multi-item query: one data.frame per 'get' item
    res <- .parse("response_multi_item.json", parser=.p)
    expect_true(is.list(res) && !inherits(res, "data.frame"), info = .with("multi item returns list"))
    expect_equal(names(res), c("name", "pe_ratio"), info = .with("list named by data item"))
    expect_equal(colnames(res$name), c("ID", "name"), info = .with("no secondary columns"))
    expect_equal(colnames(res$pe_ratio),
                 c("ID", "pe_ratio", "AS_OF_DATE", "PERIOD_END_DATE", "REVISION_COUNT"),
                 info = .with("secondary columns appended"))
    expect_equal(class(res$pe_ratio$REVISION_COUNT), "integer", info = .with("INT maps to integer"))
    expect_equal(res$name$name[2], "Apple Inc", info = .with("string values"))
    ## a DATE column is converted through unique()/match(), so assert the
    ## values and not only the name: these two are deliberately descending,
    ## and a column of duplicates could not detect a reordering
    expect_equal(res$pe_ratio$PERIOD_END_DATE, as.Date(c("2024-09-30", "2024-09-28")),
                 info = .with("secondary DATE column keeps the row order"))

    ## BQL errors surface as R errors
    expect_error(.parse("response_syntax_error.json", parser=.p),
                 pattern = "Unable to parse request", info = .with("responseExceptions raise"))

    ## literal "NA" strings in STRING columns are preserved, not turned into NA
    res <- .parse("response_string_na.json", parser=.p)
    expect_equal(res$ticker[2], "NA", info = .with("literal 'NA' string value preserved"))
    expect_false(anyNA(res$ticker), info = .with("no spurious NAs in string column"))

    ## item-level responseExceptions surface as warnings, data is kept
    expect_warning(res <- .parse("response_item_error.json", parser=.p),
                   pattern = "Insufficient data", info = .with("item-level exceptions warn"))
    expect_equal(nrow(res), 1L, info = .with("partial data still returned"))

    ## grouped aggregation, e.g. let(#mv=sum(group(amt_outstanding(),
    ## by=[year(maturity()), industry_sector()]));): the ID column holds
    ## composite group labels, year() yields an INT column, and ORIG_IDS is
    ## null for multi-security groups but set for single-security groups
    ## (synthetic values; structure verified against a live response)
    res <- .parse("response_grouped.json", parser=.p)
    expect_equal(dim(res), c(6L, 8L), info = .with("grouped: dimensions"))
    expect_equal(colnames(res),
                 c("ID", "#mv", "CURRENCY_OF_ISSUE", "MULTIPLIER", "CURRENCY",
                   "ORIG_IDS", "YEAR(MATURITY())", "INDUSTRY_SECTOR()"),
                 info = .with("grouped: column names"))
    expect_equal(unname(sapply(res, function(x) class(x)[1])),
                 c("character", "numeric", "character", "numeric", "character",
                   "character", "integer", "character"),
                 info = .with("grouped: column types incl. INT from year()"))
    expect_equal(res$ID[1], "2027.0:Technology", info = .with("grouped: composite group id"))
    expect_equal(res[["#mv"]][1], 1500000000, info = .with("grouped: aggregated value"))
    expect_equal(res[["YEAR(MATURITY())"]][1], 2027L, info = .with("grouped: integer year"))
    expect_true(anyNA(res$ORIG_IDS) && !all(is.na(res$ORIG_IDS)),
                info = .with("grouped: ORIG_IDS null for groups, set for singletons"))
    expect_equal(res$CURRENCY_OF_ISSUE[1], "USD",
                 info = .with("grouped: undeclared types like ENUM fall back to character"))
}

## -- fragmented responses -------------------------------------------------

## The C++ layer returns one string per response message, and the service cuts
## a response above 4 MiB at a byte boundary, in the middle of a token, so the
## fragments form one document only once joined. Chunking a fixture into pieces
## far smaller than 4 MiB reproduces that without needing a 4 MiB response.
##
## The chunking is on bytes, not on characters, because that is what the
## service does: a boundary can fall inside a multi-byte UTF-8 character, which
## leaves that one fragment invalid UTF-8 on its own.
.chunkBytes <- function(txt, n) {
    b <- charToRaw(txt)
    i <- split(seq_along(b), ceiling(seq_along(b) / n))
    vapply(i, function(k) rawToChar(b[k]), character(1), USE.NAMES = FALSE)
}

for (.p in .parsers) {
    for (f in list.files("bql", pattern = "[.]json$")) {
        doc <- .readFixture(f)
        ref <- .outcome(doc, .p)
        ## the two largest sizes are derived from the document, as a fixed size
        ## above the smallest fixture would give one chunk and compare the
        ## document with itself
        nb <- nchar(doc, type = "bytes")
        for (n in unique(c(1L, 7L, 64L, nb %/% 7L, nb %/% 2L))) {
            frags <- .chunkBytes(doc, n)
            expect_true(length(frags) > 1L,
                        info = .with(paste0(f, " really is split at ", n, " bytes")))
            ## the join must return the original bytes; asserted directly as
            ## well as through the parser, so a failure says which one broke
            expect_identical(Rblpapi:::.bqlJoin(frags), doc,
                             info = .with(paste0(f, " rejoins byte for byte at ", n)))
            expect_equal(.outcome(frags, .p), ref,
                         info = .with(paste0(f, " in ", length(frags),
                                             " chunks of ", n, " bytes")))
        }
    }

    ## a single fragment is not a document: the failure the joining prevents
    doc <- .readFixture("response_px_last.json")
    frags <- .chunkBytes(doc, nchar(doc, type = "bytes") %/% 2L)
    expect_true(length(frags) > 1L, info = .with("the fixture really was split"))
    expect_error(Rblpapi:::.bqlParse(frags[1], parser = .p),
                 info = .with("a lone fragment does not parse"))

    ## a boundary inside a multi-byte UTF-8 character must still rejoin; the
    ## characters are written as escapes so that this file stays ASCII
    utf8doc <- .oneItemDoc("STRING",
                           c('"Nestl\u00e9 S\u00e9n\u00e9gal"', '"\u00dcbermorgen"'),
                           name = "name")
    want <- c("Nestl\u00e9 S\u00e9n\u00e9gal", "\u00dcbermorgen")
    expect_equal(Rblpapi:::.bqlParse(utf8doc, parser = .p)$name, want,
                 info = .with("multi-byte characters read correctly"))
    for (n in 1L:8L) {
        frags <- .chunkBytes(utf8doc, n)
        expect_identical(Rblpapi:::.bqlJoin(frags), utf8doc,
                         info = .with(paste0("multi-byte rejoin at ", n, " bytes")))
        expect_equal(Rblpapi:::.bqlParse(frags, parser = .p)$name, want,
                     info = .with(paste0("multi-byte characters survive ", n,
                                         "-byte chunking")))
    }
}

## -- the parsers must agree exactly ----------------------------------------

if (length(.parsers) > 1L) {
    for (f in list.files("bql", pattern="[.]json$")) {
        out <- lapply(.parsers, function(p) .outcome(.readFixture(f), p))
        expect_true(.allIdentical(out), info = paste("all parsers agree on", f))
    }
}

## -- parser selection ------------------------------------------------------

expect_true(Rblpapi:::.bqlParser() %in% .parsers, info = "default parser is installed")
## the expected name is written out rather than taken from .bqlParsers, which
## would make the assertion agree with any order that variable happened to have
if (all(c("RcppSimdJson", "jsonlite") %in% .parsers))
    expect_equal(Rblpapi:::.bqlParser(), "RcppSimdJson",
                 info = "RcppSimdJson is preferred when both are installed")
## one helper, so a throwing assertion cannot leak the option to later tests
.withOption <- function(value, expr) {
    old <- options(bqlParser=value)
    on.exit(options(old))
    force(expr)
}
.withOption(.parsers[length(.parsers)],
            expect_equal(Rblpapi:::.bqlParser(), .parsers[length(.parsers)],
                         info = "option selects the parser"))

## an unknown parser name must be reported, not treated as "no data"
expect_error(Rblpapi:::.bqlFromJSON("{}", "notAParser"),
             pattern = "Unknown BQL JSON parser",
             info = "an unknown parser name is an error")

## the option is validated: no abbreviations, and an unknown name is not
## silently dropped when a known one sits beside it
for (.bad in list("notAParser", c("notAParser", "jsonlite"), "R", "j",
                  NA_character_, "", 1L, TRUE, list("jsonlite"), character(0)))
    .withOption(.bad,
                expect_error(Rblpapi:::.bqlParser(),
                             info = paste("option rejected:",
                                          paste(deparse(.bad), collapse = ""))))

## the parsers must agree on the intermediate structure, not merely on the
## final data.frame: '[]', '{}' and null are where they differ by default, so
## this is what the two 'empty' arguments and max_simplify_lvl="list" buy
if (length(.parsers) > 1L) {
    .shapes <- '{"a":[],"b":{},"c":[1,null,"x",true],"d":{"e":[{"f":null}]}}'
    .trees <- lapply(.parsers, function(p) Rblpapi:::.bqlFromJSON(.shapes, p))
    expect_true(.allIdentical(.trees),
                info = "parsers agree on the intermediate structure")
}

## -- column conversion -----------------------------------------------------

.col <- function(...) Rblpapi:::.bqlColumn(list(...))

## a column of only nulls must keep the type its declaration implies
expect_equal(.col(type="STRING", values=list(NULL, NULL)), c(NA_character_, NA_character_),
             info = "all-null STRING column stays character")
expect_equal(.col(type="DATE", values=list(NULL)), as.Date(NA),
             info = "all-null DATE column stays Date")
expect_equal(.col(type="DOUBLE", values=list(NULL)), NA_real_,
             info = "all-null DOUBLE column stays numeric")

## an empty or absent 'values' key gives a zero-length column, not an error
expect_equal(.col(type="DOUBLE", values=list()), numeric(0), info = "empty column")
expect_equal(.col(type="STRING"), character(0), info = "absent values key")
expect_equal(.col(type="DATE"), as.Date(character(0)), info = "absent values key, DATE")

## Every placeholder means NA in a numeric column, and only there. These use
## expect_identical because all.equal() treats NaN as equal to NA_real_, so
## expect_equal could not tell a real NA from the NaN that as.numeric("NaN")
## leaves behind when the placeholder handling is missing.
expect_identical(.col(type="DOUBLE", values=list(1, "NaN", "NA", "", 2)),
                 c(1, NA, NA, NA, 2), info = "DOUBLE placeholders become NA")
expect_identical(.col(type="INT", values=list(1L, "NaN", "NA", "")),
                 c(1L, NA, NA, NA), info = "INT placeholders become NA")
expect_false(any(is.nan(.col(type="DOUBLE", values=list(1, "NaN")))),
             info = "'NaN' becomes NA rather than NaN")
expect_equal(.col(type="STRING", values=list("NaN", "NA", "")),
             c("NaN", "NA", ""), info = "STRING keeps the same spellings verbatim")

## JSON booleans and their string spellings both convert
expect_equal(.col(type="BOOLEAN", values=list(TRUE, FALSE, NULL)), c(TRUE, FALSE, NA),
             info = "JSON booleans")
expect_equal(.col(type="BOOLEAN", values=list("true", "FALSE", NULL)), c(TRUE, FALSE, NA),
             info = "boolean strings")

## DATE and DATETIME are converted through unique()/match(), so a column whose
## distinct values are neither sorted nor unique must keep its own row order
.dts <- c("2024-03-05", "2024-01-31", "2024-03-05", "2024-02-29", "2024-01-31")
expect_equal(.col(type="DATE", values=as.list(paste0(.dts, "T00:00:00Z"))),
             as.Date(.dts), info = "DATE column keeps the row order")
expect_equal(.col(type="DATE",
                  values=c(as.list(paste0(.dts[1:2], "T00:00:00Z")), list(NULL),
                           as.list(paste0(.dts[3:5], "T00:00:00Z")))),
             as.Date(c(.dts[1:2], NA, .dts[3:5])),
             info = "DATE column with an interleaved null keeps the row order")
.tms <- c("2024-03-05T13:45:30Z", "2024-01-31T09:00:00Z", "2024-03-05T13:45:30Z")
expect_equal(.col(type="DATETIME", values=as.list(.tms)),
             as.POSIXct(.tms, format="%Y-%m-%dT%H:%M:%OS", tz="UTC"),
             info = "DATETIME column keeps the row order")

## a nested value would silently shift the rows of a column, so it must fail
expect_error(.col(name="X", type="STRING", values=list("a", list("b", "c"))),
             pattern = "non-scalar", info = "non-scalar values are rejected")

## -- double precision through the JSON layer -------------------------------

## A DOUBLE column keeps the exact values the document carried, whether or not
## a placeholder string sits beside them: the placeholders are blanked before
## the column is flattened, so it never detours through character. The second
## case is the one that regresses if that step is dropped.
## as.numeric() is correctly rounded, so as.numeric(.v) is bit-identical to
## the literal in the document and needs no separate expected value.
## 230.66000366210938 is a real float32-derived price, the case which
## motivated all of this.
for (.p in .parsers)
    for (.v in c("0.12345678901234568", "230.66000366210938"))
        for (.ph in c("null", '"NaN"', '"NA"', '""')) {
            res <- Rblpapi:::.bqlParse(.oneItemDoc("DOUBLE", c(.v, .ph)), parser=.p)
            expect_identical(res$v[1], as.numeric(.v),
                             info = .with(paste("DOUBLE keeps every digit beside", .ph)))
            expect_identical(res$v[2], NA_real_,
                             info = .with(paste("the", .ph, "placeholder itself is NA")))
        }

## A number written as a string is not a placeholder and must still convert.
## It is also the one case which does not keep every digit, because the column
## has to come back from character; expect_identical pins that, since
## expect_equal's tolerance would hide it either way.
expect_identical(.col(type="DOUBLE", values=list("123.45", 6, "NaN")),
                 c(123.45, 6, NA), info = "a number sent as a string still converts")
expect_identical(.col(type="DOUBLE", values=list(230.66000366210938, "123.45")),
                 c(as.numeric(as.character(230.66000366210938)), 123.45),
                 info = "a number sent as a string costs the column its last digits")
expect_identical(.col(type="DOUBLE", values=list(230.66000366210938, "NaN")),
                 c(230.66000366210938, NA),
                 info = "a placeholder does not")

## -- assembling an item into a data.frame -----------------------------------

.item <- function(...) Rblpapi:::.bqlItemToDataFrame(list(...))
.scol <- function(nm, vals) list(name=nm, type="STRING", values=as.list(vals))

## an item with no columns at all is an empty data.frame, not an error from
## make.unique() being handed the NULL names of an empty list
expect_equal(dim(.item(name="x", idColumn=NULL, valuesColumn=NULL,
                       secondaryColumns=list())), c(0L, 0L),
             info = "an item with no columns gives an empty data.frame")

## a repeated column name must add a column, not replace the earlier one
res <- .item(name="x", idColumn=.scol("ID", c("a", "b")),
             valuesColumn=.scol("VALUE", c("1", "2")),
             secondaryColumns=list(.scol("DATE", c("d1", "d2")),
                                   .scol("DATE", c("e1", "e2"))))
expect_equal(ncol(res), 4L, info = "a repeated column name keeps both columns")
expect_equal(colnames(res), c("ID", "x", "DATE", "DATE.1"),
             info = "make.unique() renames the second one")
expect_equal(res$DATE, c("d1", "d2"), info = "the first DATE column is intact")
expect_equal(res[["DATE.1"]], c("e1", "e2"), info = "the second DATE column is intact")

## columns of unequal length cannot make a valid data.frame, so say so
expect_error(.item(name="x", idColumn=.scol("ID", c("a", "b", "c")),
                   valuesColumn=.scol("VALUE", c("1", "2")),
                   secondaryColumns=list()),
             pattern = "unequal length",
             info = "unequal column lengths are rejected")

## -- live test (requires a Bloomberg connection) ----------------------------

.runThisTest <- Sys.getenv("RunRblpapiUnitTests") == "yes"
if (!.runThisTest) exit_file("Skipping live BQL test")

res <- bql("get(px_last) for(['IBM US Equity', 'AAPL US Equity'])")
expect_true(inherits(res, "data.frame"), info = "live query returns data.frame")
expect_equal(nrow(res), 2L, info = "one row per security")
expect_true(is.numeric(res$px_last), info = "px_last is numeric")
