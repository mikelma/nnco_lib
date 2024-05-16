 ## install.packages("ggplot2")
## install.packages("devtools")
## devtools::install_github("b0rxa/scmamp")

library("scmamp")
library("ggplot2")

## =========================== ##

data.file <- "../results/results_nnco.csv"

problem <- "PFSP"
#problem <- "LOP"

## =========================== ##


data <- read.csv(data.file)

## get the columns interest
data <- data[c("learning.rate", "prehead.layers", "hidden.dim",
               "noise.length", "instance", "best.fitness")]


## Select problem
selectInst <- function(x) {
    check <- grepl("tai", x)
    if (problem == "LOP") {
        check <- !check
    }
    return(check)
}
data <- data[unlist(lapply(data$instance, selectInst)),]

# BAYESIAN Credible Interval Plots
getCredibleIntervalsWeights <- function(posterior.samples, interval.size=0.9) {
  qmin <- (1-interval.size)/2
  qmax <- 1-qmin
  lower.bound <- apply(posterior.samples, MARGIN=2, FUN=quantile, p=qmin)
  upper.bound <- apply(posterior.samples, MARGIN=2, FUN=quantile, p=qmax)
  expectation <- apply(posterior.samples, MARGIN=2, FUN=mean)
  return (data.frame(Expected=expectation, Lower_bound=lower.bound, Upper_bound=upper.bound))
}

nreps <- 20
nproblems <- length(unique(data$instance))

learning.rates <- unique(data$learning.rate)
prehead.layers <- unique(data$prehead.layers)
hidden.dims <- unique(data$hidden.dim)
noise.lens <- unique(data$noise.len)

num.combinations <- prod(length(learning.rates), length(prehead.layers),
                         length(hidden.dims), length(noise.lens))

## ====> NOTE!!! Josuren kodean: nrow = nreps*nproblems
dataBayesian <- data.frame(matrix(NA, nrow = nreps*nproblems, ncol = num.combinations))
instances <- unique(data$instance)
col <- 1
for (lr in learning.rates) {
    for (layers in prehead.layers) {
        for (dim in hidden.dims) {
            for (noise.len in noise.lens) {
                all.values <- c() ## new
                columname <- paste("(", lr, layers, dim, noise.len, collapse = " ", ")")
                print(columname)
                for (inst in instances) {
                    values <- data[(data$learning.rate == lr & data$prehead.layers == layers
                        & data$hidden.dim == dim & data$noise.length == noise.len),]$best.fitness

                    values <- head(values, nreps)
                    values <- c(values, rep(NA, nreps-length(values)))
                    all.values <- c(all.values, values)
                }
                dataBayesian[,col] <- all.values
                colnames(dataBayesian)[col] <-columname
                col <- col + 1
            }
        }
    }
}

## col <- 1
## for (lr in learning.rates) {
##     for (layers in prehead.layers) {
##         for (dim in hidden.dims) {
##             for (noise.len in noise.lens) {
##                 columname <- paste("(", lr, layers, dim, noise.len, collapse = " ", ")")
##                 print(columname)
##
##                 values <- data[(data$learning.rate == lr & data$prehead.layers == layers
##                     & data$hidden.dim == dim & data$noise.length == noise.len),]$best.fitness
##
##                 values <- head(values, nrow(dataBayesian))
##
##                 dataBayesian[,col] <- c(values, rep(NA, nrow(dataBayesian)-length(values)))
##                 colnames(dataBayesian)[col] <-columname
##                 col <- col + 1
##             }
##         }
##     }
## }


## pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=FALSE, nsim=20, nchains=5, parallel=TRUE)
pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=(problem == "PFSP"), nsim=4000, nchains=20, parallel=TRUE)


processed.results <- getCredibleIntervalsWeights(pl_model$posterior.weights, interval.size=0.9)
dataBayesian_2 <- data.frame(Setting=rownames(processed.results), processed.results)
bayesianNES_IO<-ggplot(dataBayesian_2, aes(y=Expected, ymin=Lower_bound, ymax=Upper_bound, x=Setting)) + geom_errorbar() + geom_point(col="darkgreen", size=2) +  theme_bw() + coord_flip() + labs(y="Probability of winning")+ggtitle(label = paste("NNCO - ", problem))

fname <- paste("nnco_", problem, ".pdf", sep="")
ggsave(paste("nnco_", problem, ".pdf", sep=""), width=200, height=250, units="mm")
