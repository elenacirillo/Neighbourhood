
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
#setwd("D:/Users/camillabelponer/Desktop/APC/Neighbourhood/TESTS/per_gain")

# data
df.gpu20 <- read.csv("/Users/camillabelponer/Desktop/APC/Neighbourhood/TESTS/per_gain/GPU_20/TEST_OUTPUTS_2.csv", header = FALSE);
df.swap20 <- read.csv("/Users/camillabelponer/Desktop/APC/Neighbourhood/TESTS/per_gain/Swap_20/TEST_OUTPUTS_2.csv", header = FALSE);
df.gpu60 <- read.csv("/Users/camillabelponer/Desktop/APC/Neighbourhood/TESTS/per_gain/GPU_60/TEST_OUTPUTS_2.csv", header = FALSE);
df.swap60 <- read.csv("/Users/camillabelponer/Desktop/APC/Neighbourhood/TESTS/per_gain/Swap_60/TEST_OUTPUTS_2.csv", header = FALSE);

gain = df.gpu20[1,]-df.gpu20[2,]/df.gpu20[1,] *100;
