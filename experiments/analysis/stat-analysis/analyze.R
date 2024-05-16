library("scmamp")
library("ggplot2")

problem <- "PFSP"
only.big.instances <- FALSE
num.reps <- 20
num.algorithms <- 3

fname <- paste("result_joined_", problem, ".csv", sep = "")
data <- read.csv(fname)

if (only.big.instances) {
    mode <- "_BIG_"
    data <- data[data$problem.size >= 100,]
} else {
    mode <- "_all_"
}

instances <- unique(data$instance)
num.instances <- length(instances)
## algorithms <- unique(data$algorithm)

if (problem == "PFSP") {
    algorithms <- c("NNCO", "UMDA", "RK-EDA")
    ## default.value <- Inf
} else {
    algorithms <- c("NNCO", "GS", "PL-EDA")
    ## default.value <- 0
}

dataBayesian <- data.frame(matrix(NA, nrow = num.reps*num.instances, ncol = length(algorithms)))
colnames(dataBayesian) <- algorithms

col <- 1
for (algo in algorithms) {
    values <- c()
    for (inst in instances) {
        r <- data[(data$algorithm == algo & data$instance == inst),]$best.fitness
        r <- head(r, num.reps)  # fix greater num. of runs than expected
        r <- c(r, rep(NA, num.reps - length(r)))
        values <- c(values, r)
    }
    dataBayesian[,col] <- values
    col <- col + 1
}

pl_model <-bPlackettLuceModel(x.matrix=dataBayesian, min=(problem == "PFSP"), nsim=4000, nchains=20, parallel=TRUE)

##
## Simplex plots
## (from https://github.com/b0rxa/scmamp/blob/master/R/plotting.R#L667)
##

library("geometry") # provides bary2cart

plotResults <- function(pl_model, names, plot.density=TRUE, plot.points=TRUE,
                        palette=c("green", "darkgray", "red"), point.size=1,
                        font.size=5, alpha=NULL, posterior.label=FALSE) {

    theme(panel.background = element_blank())

    post.sample <- pl_model$posterior
    colnames(post.sample) <- names
    colnames(pl_model$posterior.weights) <- names

    ## post.sample <- post.sample[, c("Left","Rope","Right")]

    ## Get the winner for the color of the points
    aux <- apply(post.sample, MARGIN=1, FUN=which.max)
    aux[aux==1] <- colnames(post.sample)[1]
    aux[aux==2] <- colnames(post.sample)[2]
    aux[aux==3] <- colnames(post.sample)[3]
    colors <- factor(aux, colnames(post.sample))

    ## Coordinates of the eges of the Simplex
    simplex.coords <- rbind(c(2, 0), c(1,1), c(0, 0))

    ## Auxiliar info to draw the triangle
    center <- c(1, 0.3333333)
    ab <- c(0.5, 0.5)
    bc <- c(1.5, 0.5)
    ac <- c(1, 0)

    ##Convert from barycentric coords to cartesians and add the winner
    points <- data.frame(Color=colors, bary2cart(simplex.coords, as.matrix(post.sample)))
    names(points) <- c("Colors", "X", "Y")

    ## Additional info for the plot (triangle and lines)
    triangle <- data.frame(simplex.coords[c(1, 2, 3), ])
    names(triangle) <- c("X", "Y")

    divisors1 <- data.frame(rbind(center, ab))
    names(divisors1) <- c("X", "Y")
    divisors2 <- data.frame(rbind(center, bc))
    names(divisors2) <- c("X", "Y")
    divisors3 <- data.frame(rbind(center, ac))
    names(divisors3) <- c("X", "Y")

    p.right<- pl_model$posterior.weights[, names[1]]
    p.rope <- pl_model$posterior.weights[, names[2]]
    p.left <- pl_model$posterior.weights[, names[3]]

    if (is.null(alpha)) {
        alpha <- min(1, 250/nrow(points))
    }

    ## Create the plot, layer by layer
    g <- ggplot(points, aes(x=X, y=Y))

    ## Optionally, add the points
    if (plot.points) {
        g <- g + geom_point(aes(color=Colors), alpha=alpha, shape=19, size=point.size) +
            scale_color_manual(values=setNames(c(palette[1], palette[2], palette[3]), algorithms), guide="none")
            ## scale_color_manual(values=c(algorithms[3]=palette[3], algorithms[2]=palette[2], algorithms[1]=palette[1]), guide="none")

    }

    ## Optionally, add the density
    if (plot.density) {
        g <- g + stat_density2d(aes(fill=..level..,alpha=..level..),geom="polygon", show.legend=FALSE) +
            scale_fill_gradient2(low="#ffffff", mid="#78c679", high="#005a32", guide="none") +
            geom_polygon(data=data.frame(X=c(-0.1,1.1,-0.1), Y=c(-0.1,1.1,1.1)), fill="white") +
            geom_polygon(data=data.frame(X=c(0.9,2.1,2.1), Y=c(1.1,1.1,-0.1)), fill="white") +
            geom_polygon(data=data.frame(X=c(-0.1,2.1,2.1,-0.1), Y=c(0,0,-0.1,-0.1)), fill="white")
    }

    ## Add the triagle and annotations
    g <- g + geom_polygon(data=triangle, color="black", fill=NA) +
        geom_line(data=divisors1, color="black", linetype=2) +
        geom_line(data=divisors2, color="black", linetype=2) +
        geom_line(data=divisors3, color="black", linetype=2) +
        annotate("text", x=0, y=-0.05, label=algorithms[3], hjust=0, size=font.size) +
        annotate("text", x=2, y=-0.05, label=algorithms[1], hjust=1, size=font.size) +
        annotate("text", x=1, y=1.05, label=algorithms[2], hjust=0.5, size=font.size) +
        scale_y_continuous(limits=c(-0.1, 1.1)) + theme_void() + theme(panel.background=element_rect(fill="white"))

    ## And, optionally, add the probabilities
    if (posterior.label) {

        ## -----> TODO: Use pl_model$expected.win.prob!!!!!

        g <- g + annotate("text", x=0, y=1, vjust=1, hjust=0, size=font.size*0.6,
                          label=paste0("P(", algorithms[1], " Win)= ", round(mean(p.right), 3), "\n",
                                       "P(", algorithms[3], " Win)= ", round(mean(p.left), 3), "\n",
                                       "P(", algorithms[2], " Win)= ", round(mean(p.rope), 3), "\n"))
    }

    g <- g + annotate("text", x=0, y=1.1, label=problem, hjust=0, vjust=1, size=font.size*1.5)

    ggsave(paste("stat_analysis", mode, "_", problem,".png", sep=""), width = 1000, height = 1000, units = "px")
}

plotResults(pl_model, names = algorithms, posterior.label = TRUE, font.size = 4)
