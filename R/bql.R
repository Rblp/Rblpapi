#' blp_bql: Bloomberg BQL Query Interface
#'
#' Executes a Bloomberg Query Language (BQL) query and returns the results as a data.frame.
#'
#' @param query Character string with the BQL query.
#' @return A data.frame containing the query results.
#' @examples
#' \dontrun{
#'   blp_bql("get(px_last) for(security('AAPL US Equity'))")
#' }
#' @export
blp_bql <- function(query) {
  stopifnot(is.character(query), length(query) == 1)
  .Call('_Rblpapi_bql', query, PACKAGE = "Rblpapi")
}
