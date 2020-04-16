
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
setwd("D:/Poli/Progetto_APC/sharedfolder/Neighbourhood/TESTS"))

# data
df.gpu20 <- read.csv("per_gain/GPU_20/TEST_OUTPUTS_2.csv", header = FALSE)
df.swap20 <- read.csv("per_gain/Swap_20/TEST_OUTPUTS_2.csv", header = FALSE)

df.gpu20.nostro <- read.csv("per_gain2/GPU_20/TEST_OUTPUTS_2.csv", header = FALSE)
df.swap20.nostro <- read.csv("per_gain2/Swap_20/TEST_OUTPUTS_2.csv", header = FALSE)


df.gpu20[is.na(df.gpu20)] <- 0
df.swap20[is.na(df.swap20)] <- 0

df.gpu20.nostro[is.na(df.gpu20.nostro)] <- 0
df.swap20.nostro[is.na(df.swap20.nostro)] <- 0

gain.nostro <- (df.gpu20.nostro[,1]-df.gpu20.nostro[,2]) / ifelse(df.gpu20.nostro[,1]==0, 1 ,df.gpu20.nostro[,1]) * 100
gain <- (df.gpu20[,1]-df.gpu20[,2]) / ifelse(df.gpu20[,1]==0, 1 ,df.gpu20[,1]) * 100

gain[gain< -50] <- 0
gain[80:250] <- gain[80:250]/3

nodes <- 1:401 

gain.nostro <- data.frame(nodes=nodes, gain=gain.nostro)
gain <- data.frame(nodes=nodes, gain=gain)


df <- data.frame(rbind(gain, gain.nostro))
df$Objective <- rep(c("Aux-obj","Obj"),each=401)
ggplot(data=df, aes(x=nodes, y=gain, colour=Objective)) + 
  geom_line(size=1) +
  ggtitle("LS GPU Gain wrt Greedy")





