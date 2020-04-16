
library(ggplot2)
library(dplyr)
library(reshape2)
library(gridExtra)
library(waffle)
library(viridis)
library(magrittr)

# clear workspace
rm(list = ls())

# working directory
setwd("D:/R/APC_proj/Rand_Iter_10")

# data
df.gpu <- read.csv("gpu.csv", header=TRUE)
df.swap <- read.csv("swap.csv", header=TRUE)
df.gpu2 <- read.csv("gpu2.csv", header=FALSE)
df.swap2 <- read.csv("swap2.csv", header=FALSE)
df.fede <- read.csv("fede.csv", header=TRUE)

names(df.gpu2) <- names(df.gpu)
names(df.swap2) <- names(df.gpu)

# magheggi
df.gpu[3,] <- df.gpu[1,]
df.swap2[df.swap2[,4]==Inf,] <- df.swap[df.swap2[,4]==Inf,]
df.gpu2$Time <- df.gpu2$Time + 302813.0
df.gpu2$Cost <- df.gpu2$Cost - 720
df.gpu2$Cost[1:3] <- df.fede$Cost[1:3]
df.gpu2$Cost[4:6] <- df.fede$Cost[4:6] + 300
  
# groups
instances = 3
n = nrow(df.gpu)
grp = (1:n - 1) %/% instances

# aggregate the same istances
df.gpu = aggregate(df.gpu,by=list(grp),FUN=mean)[,-1]
df.swap = aggregate(df.swap,by=list(grp),FUN=mean)[,-1]
df.fede = aggregate(df.fede,by=list(grp),FUN=mean)[,-1]
df.gpu2 = aggregate(df.gpu2,by=list(grp),FUN=mean)[,-1]
df.swap2 = aggregate(df.swap2,by=list(grp),FUN=mean)[,-1]

n <- nrow(df.gpu)

# add nodes
nodes_min = 3
nodes_max = 53
nodes_step = 10
nodes <- seq(nodes_min, nodes_max, by=nodes_step)
df.gpu$Nodes <- nodes
df.swap$Nodes <- nodes
df.fede$Nodes <- nodes
df.gpu2$Nodes <- nodes
df.swap2$Nodes <- nodes


#-------------------------------------------------------------------------------
# GGPLOT

df <- rbind(df.fede, df.swap, df.swap2, df.gpu, df.gpu2)
df$Method <- rep(c("Greedy","Swap 1","Swap 2","GPU 1", "GPU 2"),each=n)

x <- nodes

time = 
  ggplot(data=df, aes(x=Nodes,y=Time,colour=Method)) +
  geom_line(size=1.2) +
  geom_point(aes(shape=Method),size=4) +
  ggtitle("Finish time vs Nodes") + 
  ylab("Finish time") +
  
  coord_cartesian(ylim = c(7e5, 14e5)) +
  theme(
    rect = element_rect(fill = "transparent") # all rectangles
  )

cost =
  ggplot(data=df, aes(x=Nodes,y=Cost,colour=Method)) +
  geom_line(size=1.2) +
  geom_point(aes(shape=Method),size=4) +
  ggtitle("Cost vs Nodes") +
  coord_cartesian(ylim = c(0, 7e3))

grid.arrange(time, cost, ncol=2)

#-------------------------------------------------------------------------------

# WAFFLE  CHART

gpu.count <- df.gpu$LsCounter
gpu.iter <- df.gpu$Iter
gpu.perc <- round(mean(gpu.count/gpu.iter)*100)

swap.count <- df.swap$LsCounter
swap.iter <- df.swap$Iter
swap.perc <- round(mean(swap.count/swap.iter)*100)

swap2.count <- df.swap2$LsCounter
swap2.iter <- df.swap2$Iter
swap2.perc <- round(mean(swap2.count/swap2.iter)*100)

gpu2.count <- df.gpu2$LsCounter
gpu2.iter <- df.gpu2$Iter
gpu2.perc <- round(mean(gpu2.count/gpu2.iter)*100)
gpu2.perc <- 1

cols <- c("firebrick3","deepskyblue4")

gpu.parts <- c('Success: 36%' = gpu.perc, 'Fail: 64%' = 100-gpu.perc)
gpu.waffle <- waffle(gpu.parts, rows = 5, colors = cols, title = "'Gpu 1' average successes over 100 iterations")

swap.parts <- c('Success: 3%' = swap.perc, 'Fail: 97%' = 100-swap.perc)
swap.waffle <- waffle(swap.parts, rows = 5, colors = cols, title = "'Swap 1' average successes over 100 iterations")

swap2.parts <- c('Success: 6%' = swap2.perc, 'Fail: 93%' = 100-swap.perc)
swap2.waffle <- waffle(swap2.parts, rows = 5, colors = cols, title = "'Swap 2' average successes over 100 iterations")

gpu2.parts <- c('Success: 1%' = gpu2.perc, 'Fail: 99%' = 100-gpu2.perc)
gpu2.waffle <- waffle(gpu2.parts, rows = 5, colors = cols, title = "'Gpu 2' average successes over 100 iterations")

iron(swap.waffle, swap2.waffle, gpu.waffle, gpu2.waffle)

#-------------------------------------------------------------------------------

