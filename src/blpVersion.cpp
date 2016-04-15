// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  blpVersion.cpp -- Version of the Bloomberg API
//
//  Copyright (C) 2016  Whit Armstrong and Dirk Eddelbuettel and John Laing
//
//  This file is part of Rblpapi
//
//  Rblpapi is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  Rblpapi is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Rblpapi.  If not, see <http://www.gnu.org/licenses/>.

#include <Rcpp.h>
#include <blpapi_versioninfo.h>

using BloombergLP::blpapi::VersionInfo;

// ' This function retrieves the version string of the Bloomberg API.
// '
// ' Note that formatting of the returned string is described as
// ' \sQuote{unspecified} by API documentation
// ' @title Get Bloomberg library and run-time version
// ' @return A string (with unspecified format) containing the version
// ' of the Blpapi runtime library.
// ' @author Dirk Eddelbuettel
// ' @seealso \code{getHeaderVersion}, \code{getRuntimeVersion}
// ' @examples
// ' \dontrun{
// '    getVersionIdentifier()
// ' }
// [ [ Rcpp::export ] ]
// std::string getVersionIdentifier() {
//     // Return a string containing a sequence of printable ascii characters
//     // (with values from 0x20 to 0x7e, inclusive) identifying the version
//     // of the blpapi runtime library.  The format of this string is
//     // unspecified.
//     //return VersionInfo::versionIdentifier();
//     return std::string(blpapi_getVersionIdentifier());
// }

//' This function retrieves the version of Bloomberg API headers.
//'
//' @title Get Bloomberg library header version
//' @return A string with four dot-separated values for major, minor,
//' pathch and build version of the headers.
//' @author Dirk Eddelbuettel
//' @seealso \code{getRuntimeVersion}
//' @examples
//' \dontrun{
//'    getHeaderVersion()
//' }
// [[Rcpp::export]]
std::string getHeaderVersion() {
    // VersionInfo vi = VersionInfo::headerVersion();
    // //Rcpp::Rcout << vi << std::endl;
    // char txt[128];
    // snprintf(txt, 127, "%d.%d.%d.%d",
    //          vi.majorVersion(),
    //          vi.minorVersion(),
    //          vi.patchVersion(),
    //          vi.buildVersion());

    char txt[128];
    snprintf(txt, 127, "%d.%d.%d.%d",
             BLPAPI_VERSION_MAJOR,
             BLPAPI_VERSION_MINOR,
             BLPAPI_VERSION_PATCH,
             BLPAPI_VERSION_BUILD);
    return std::string(txt);
}

//' This function retrieves the version of Bloomberg API run-time.
//'
//' @title Get Bloomberg library run-time version
//' @return A string with four dot-separated values for major, minor,
//' pathch and build version of the run-time library.
//' @author Dirk Eddelbuettel
//' @seealso \code{getHeaderVersion}
//' @examples
//' \dontrun{
//'    getRuntimeVersion()
//' }
// [[Rcpp::export]]
std::string getRuntimeVersion() {
    // VersionInfo vi = VersionInfo::runtimeVersion();
    // //Rcpp::Rcout << vi << std::endl;
    // char txt[128];
    // snprintf(txt, 127, "%d.%d.%d.%d",
    //          vi.majorVersion(),
    //          vi.minorVersion(),
    //          vi.patchVersion(),
    //          vi.buildVersion());

    int major, minor, patch, build;
    blpapi_getVersionInfo(&major, &minor, &patch, &build);
    char txt[128];
    snprintf(txt, 127, "%d.%d.%d.%d",
             major, minor, patch, build);
    return std::string(txt);
}
