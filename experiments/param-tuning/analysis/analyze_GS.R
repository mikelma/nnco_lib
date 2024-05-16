## install.packages("ggplot2")
## install.packages("devtools")
## devtools::install_github("b0rxa/scmamp")

library("scmamp")
library("ggplot2")

## =========================== ##

data.file <- "../results/results_GS.csv"

## =========================== ##

data <- read.csv(data.file)

## get the sizes of each instance
parseSize <- function(x) {
    instances <-c("N-t59b11xx", "N-be75eec", "N-stabu70", "N-be75eec_150", "N-stabu1_250")
    sizes <- c(44, 50, 60, 150, 250)
    return(sizes[instances == x])
}
data$size <- unlist(lapply(data$instance, parseSize))

## get the columns interest
# data <- data[c("al", "trunc.size", "instance", "best.fitness")]

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

alphas <- unique(data$alpha)
lambdas <- unique(data$lambda)

num.combinations <- prod(length(alphas), length(lambdas))

## ====> NOTE!!! Josuren kodean: nrow = nreps*nproblems
dataBayesian <- data.frame(matrix(NA, nrow = nreps*nproblems, ncol = num.combinations))


instances <- unique(data$instance)
col <- 1
for (alpha in alphas) {
    for (lambda in lambdas) {
        all.values <- c() ## new
        columname <- paste("(", alpha, lambda, collapse = " ", ")")
        print(columname)
        for (inst in instances) {
            values <- data[(data$lambda == lambda & data$alpha == alpha & data$instance == inst),]$best.fitness

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
pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=FALSE, nsim=4000, nchains=20, parallel=TRUE)


processed.results <- getCredibleIntervalsWeights(pl_model$posterior.weights, interval.size=0.9)
dataBayesian_2 <- data.frame(Setting=rownames(processed.results), processed.results)
bayesianNES_IO<-ggplot(dataBayesian_2, aes(y=Expected, ymin=Lower_bound, ymax=Upper_bound, x=Setting)) + geom_errorbar() + geom_point(col="darkgreen", size=2) +  theme_bw() + coord_flip() + labs(y="Probability of winning")+ggtitle(label = "GS - LOP")

ggsave("GS.pdf") #, width=200, height=250, units="mm")
