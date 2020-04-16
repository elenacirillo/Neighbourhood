
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
setwd("D:/Poli/Progetto_APC/sharedfolder/Neighbourhood/TESTS")


# data
df.gpu20 <- read.csv("per_gain/GPU_20/TEST_OUTPUTS_2.csv", header = FALSE);
df.swap20 <- read.csv("per_gain/Swap_20/TEST_OUTPUTS_2.csv", header = FALSE);
df.gpu60 <- read.csv("per_gain/GPU_60/TEST_OUTPUTS_2.csv", header = FALSE);
df.swap60 <- read.csv("per_gain/Swap_60/TEST_OUTPUTS_2.csv", header = FALSE);

df.gpu20[is.na(df.gpu20)] <- 0
df.swap20[is.na(df.swap20)] <- 0
df.gpu60[is.na(df.gpu60)] <- 0
df.swap60[is.na(df.swap60)] <- 0

anyNA(df.gpu20)
anyNA(df.swap20)
anyNA(df.gpu60)
anyNA(df.swap60)

gain <- (df.gpu20[,1]-df.gpu20[,2]) / ifelse(df.gpu20[,1]==0, 1 ,df.gpu20[,1])
gain

plot(1:401, -gain/10, "l")


