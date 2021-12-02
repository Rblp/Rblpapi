
library(Rblpapi)

blpConnect()

res <- Rblpapi:::bds_debug("YCSW0303 Index", "par_curve", repeats = 10000)

print("no segfault")
print(length(res))
