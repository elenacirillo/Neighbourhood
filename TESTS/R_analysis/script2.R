
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
setwd("D:/R/APC_proj/Neigh_size_3")

# data
df.gpu <- read.csv("GPU.csv")
df.swap <- read.csv("swap.csv")
df.fede <- read.csv("fede.csv")

# groups
instances = 3
n = nrow(df.gpu)
grp = (1:n - 1) %/% instances

# aggregate the same istances
df.gpu = aggregate(df.gpu,by=list(grp),FUN=mean)[,-1]
df.swap = aggregate(df.swap,by=list(grp),FUN=mean)[,-1]
df.fede = aggregate(df.fede,by=list(grp),FUN=mean)[,-1]

n <- nrow(df.gpu)

# add nodes
nodes_min = 1
nodes_max = 51
nodes_step = 10
nodes <- seq(nodes_min, nodes_max, by=nodes_step)
df.gpu$Nodes <- nodes
df.swap$Nodes <- nodes
df.fede$Nodes <- nodes


#-------------------------------------------------------------------------------
# GGPLOT

df <- rbind(df.fede, df.swap, df.gpu)
df$Method <- rep(c("Fede","Swap","Gpu"),each=n)

x <- nodes

time = 
  ggplot(data=df, aes(x=Nodes,y=Time,colour=Method)) +
  geom_line(size=1.2) +
  geom_point(aes(shape=Method),size=4) +
  ggtitle("Finish time vs Nodes") + 
  ylab("Finish time")

cost =
  ggplot(data=df, aes(x=Nodes,y=Cost,colour=Method)) +
  geom_line(size=1.2) +
  geom_point(aes(shape=Method),size=4) +
  ggtitle("Cost vs Nodes")


grid.arrange(time, cost, ncol=2)

#-------------------------------------------------------------------------------

# WAFFLE  CHART

gpu.count <- df.gpu$LsCounter
gpu.iter <- df.gpu$Iter
gpu.perc <- round(mean(gpu.count/gpu.iter)*100)

swap.count <- df.swap$LsCounter
swap.iter <- df.swap$Iter
swap.perc <- round(mean(swap.count/swap.iter)*100)

cols <- c("firebrick3","deepskyblue4")

gpu.parts <- c('Success: %' = gpu.perc, 'Fail: %' = 100-gpu.perc)
gpu.waffle <- waffle(gpu.parts, rows = 5, colors = cols, title = "Local Search GPU average successes over 100 iterations")

swap.parts <- c('Success: %' = swap.perc, 'Fail: %' = 100-swap.perc)
swap.waffle <- waffle(swap.parts, rows = 5, colors = cols, title = "Local Search Swap average successes over 100 iterations")

iron(gpu.waffle,swap.waffle)

gpu.perc
swap.perc

#-------------------------------------------------------------------------------







