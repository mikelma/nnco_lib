## install.packages("ggplot2")
## install.packages("devtools")
## devtools::install_github("b0rxa/scmamp")

library("scmamp")
library("ggplot2")

## =========================== ##

data.file <- "../results/results_umda.csv"
nreps <- 20

## =========================== ##


data <- read.csv(data.file)

## get the sizes of each instance
parseSize <- function(x) {
    a <- strsplit(x, "_")[[1]][1]
    b <- strsplit(a, "tai")[[1]][2]
    return(as.numeric(b))
}

data$size <- unlist(lapply(data$instance, parseSize))
data$pop.size <- data$pop.size / data$size

## get the columns interest
data <- data[c("pop.size", "trunc.size", "instance", "min")]

# BAYESIAN Credible Interval Plots
getCredibleIntervalsWeights <- function(posterior.samples, interval.size=0.9) {
  qmin <- (1-interval.size)/2
  qmax <- 1-qmin
  lower.bound <- apply(posterior.samples, MARGIN=2, FUN=quantile, p=qmin)
  upper.bound <- apply(posterior.samples, MARGIN=2, FUN=quantile, p=qmax)
  expectation <- apply(posterior.samples, MARGIN=2, FUN=mean)
  return (data.frame(Expected=expectation, Lower_bound=lower.bound, Upper_bound=upper.bound))
}

nproblems <- length(unique(data$instance))

pop.sizes <- unique(data$pop.size)
trunc.sizes <- unique(data$trunc.size)

num.combinations <- prod(length(pop.sizes), length(trunc.sizes))

dataBayesian <- data.frame(matrix(NA, nrow = nreps*nproblems, ncol = num.combinations))

instances <- unique(data$instance)
col <- 1
for (ps in pop.sizes) {
    for (ts in trunc.sizes) {
        all.values <- c() ## new
        columname <- paste("(", ps, ts, collapse = " ", ")")
        print(columname)
        for (inst in instances) {
            values <- data[(data$pop.size == ps & data$trunc.size == ts & data$instance == inst),]$min

            values <- head(values, nreps)
            values <- c(values, rep(NA, nreps-length(values)))

            all.values <- c(all.values, values)
        }
        dataBayesian[,col] <- all.values
        colnames(dataBayesian)[col] <-columname
        col <- col + 1
    }
}

## pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=FALSE, nsim=20, nchains=5, parallel=TRUE)
pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=TRUE, nsim=4000, nchains=20, parallel=TRUE)


processed.results <- getCredibleIntervalsWeights(pl_model$posterior.weights, interval.size=0.9)
dataBayesian_2 <- data.frame(Setting=rownames(processed.results), processed.results)
bayesianNES_IO<-ggplot(dataBayesian_2, aes(y=Expected, ymin=Lower_bound, ymax=Upper_bound, x=Setting)) + geom_errorbar() + geom_point(col="darkgreen", size=2) +  theme_bw() + coord_flip() + labs(y="Probability of winning")+ggtitle(label = "UMDA - PFSP")

ggsave("umda.pdf") #, width=200, height=250, units="mm")
