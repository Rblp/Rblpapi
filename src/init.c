#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP Rblpapi_authenticate_Impl(SEXP, SEXP, SEXP);
extern SEXP Rblpapi_bdh_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_bdp_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_bds_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_beqs_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_blpConnect_Impl(SEXP, SEXP);
extern SEXP Rblpapi_bsrch_Impl(SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_fieldInfo_Impl(SEXP, SEXP);
extern SEXP Rblpapi_fieldSearch_Impl(SEXP, SEXP, SEXP);
extern SEXP Rblpapi_getBars_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_getHeaderVersion();
extern SEXP Rblpapi_getPortfolio_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_getRuntimeVersion();
extern SEXP Rblpapi_getTicks_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_lookup_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP Rblpapi_subscribe_Impl(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"Rblpapi_authenticate_Impl", (DL_FUNC) &Rblpapi_authenticate_Impl,  3},
    {"Rblpapi_bdh_Impl",          (DL_FUNC) &Rblpapi_bdh_Impl,          10},
    {"Rblpapi_bdp_Impl",          (DL_FUNC) &Rblpapi_bdp_Impl,           7},
    {"Rblpapi_bds_Impl",          (DL_FUNC) &Rblpapi_bds_Impl,           7},
    {"Rblpapi_beqs_Impl",         (DL_FUNC) &Rblpapi_beqs_Impl,          7},
    {"Rblpapi_blpConnect_Impl",   (DL_FUNC) &Rblpapi_blpConnect_Impl,    2},
    {"Rblpapi_bsrch_Impl",        (DL_FUNC) &Rblpapi_bsrch_Impl,         4},
    {"Rblpapi_fieldInfo_Impl",    (DL_FUNC) &Rblpapi_fieldInfo_Impl,     2},
    {"Rblpapi_fieldSearch_Impl",  (DL_FUNC) &Rblpapi_fieldSearch_Impl,   3},
    {"Rblpapi_getBars_Impl",      (DL_FUNC) &Rblpapi_getBars_Impl,       8},
    {"Rblpapi_getHeaderVersion",  (DL_FUNC) &Rblpapi_getHeaderVersion,   0},
    {"Rblpapi_getPortfolio_Impl", (DL_FUNC) &Rblpapi_getPortfolio_Impl,  7},
    {"Rblpapi_getRuntimeVersion", (DL_FUNC) &Rblpapi_getRuntimeVersion,  0},
    {"Rblpapi_getTicks_Impl",     (DL_FUNC) &Rblpapi_getTicks_Impl,      7},
    {"Rblpapi_lookup_Impl",       (DL_FUNC) &Rblpapi_lookup_Impl,        6},
    {"Rblpapi_subscribe_Impl",    (DL_FUNC) &Rblpapi_subscribe_Impl,     6},
    {NULL, NULL, 0}
};

void R_init_Rblpapi(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
