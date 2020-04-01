
library(ggplot2)
library(dplyr)
library(reshape2)
library(gridExtra)


# clear workspace
rm(list = ls())

setwd("D:/R/APC_proj/Data")

# import data
df.gpu <- read.csv("GPU_best_fit_10iter_50nodes.csv")
df.swap <- read.csv("Swap_best_fit_10iter_50nodes.csv")
df.fede <- read.csv("FedeCpp4_R_10iter_50nodes.csv")
df.gpu.first <- read.csv("GPU_first_fit_10iter_50nodes.csv")


#-------------------------------------------------------------------------------
# Magheggio  sullo swap

df.swap2 <- df.swap[1,]
new.row <- df.swap2

for (i in 1:50) {
  
  old.indices <- c(i*3-2,i*3-1,i*3)
  non.inf.indices <- old.indices[df.swap[old.indices,]$Cost!= Inf]
  
  df.swap[old.indices,]$Iter
  
  row1 <- df.swap[i*3-2,]
  row2 <- df.swap[i*3-1,]
  row3 <- df.swap[i*3,]
  
  new.row$Method <- row1$Method
  new.row$Iter <- row1$Iter
  new.row$LsCounter <- mean(df.swap[old.indices,]$LsCounter)
  new.row$Time <- mean(df.swap[old.indices,]$Time)
  new.row$Tardiness <- mean(df.swap[old.indices,]$Tardiness)
  
  # Here consider only indices of rows that have cost != Inf
  new.row$Cost <- mean(df.swap[non.inf.indices,]$Cost)
  
  df.swap2[i,] <- new.row
  
}


#-------------------------------------------------------------------------------
# "Correggo" i valori brutti con le medie dei valori vicini

which(df.gpu$Time > 1e11) #5

df.gpu[5,]$Time <- mean(df.gpu[4,]$Time,df.gpu[6,]$Time)
df.gpu[5,]$Cost <- mean(df.gpu[4,]$Cost,df.gpu[6,]$Cost)

which(df.fede$Time >= 1e10) #42, 43, 46, 47, 48

df.fede[42,]$Time <- mean(df.fede[41,]$Time,df.fede[44,]$Time)
df.fede[43,]$Time <- mean(df.fede[41,]$Time,df.fede[44,]$Time)
df.fede[46,]$Time <- mean(df.fede[45,]$Time,df.fede[49,]$Time)
df.fede[47,]$Time <- mean(df.fede[45,]$Time,df.fede[49,]$Time)
df.fede[48,]$Time <- mean(df.fede[45,]$Time,df.fede[49,]$Time)

df.fede[42,]$Cost <- mean(df.fede[41,]$Cost,df.fede[44,]$Time)
df.fede[43,]$Cost <- mean(df.fede[41,]$Cost,df.fede[44,]$Time)
df.fede[46,]$Cost <- mean(df.fede[45,]$Cost,df.fede[49,]$Cost)
df.fede[47,]$Cost <- mean(df.fede[45,]$Cost,df.fede[49,]$Cost)
df.fede[48,]$Cost <- mean(df.fede[45,]$Cost,df.fede[49,]$Cost)

#-------------------------------------------------------------------------------
# Basic plot

{
par(mfrow=c(1,2))

plot(1:50, df.gpu$Time, col="red", type="l", pch=16, xlab="#nodes" , ylab="time", ylim=c(4e5,12e5),
     main="Time")
points(1:50, df.swap2$Time, col="blue", type="l", pch=16)
points(1:50, df.fede$Time, col="green", type="l", pch=16)
legend("topleft", legend = c("gpu", "swap","fede"), col = c("red","blue","green"), pch = c(16,16), cex = 0.75)
grid()

plot(1:50, df.gpu$Cost, col="red", type="l", pch=16, xlab="#nodes" , ylab="cost",
     main="Cost")
points(1:50, df.swap2$Cost, col="blue", type="l", pch=16)
points(1:50, df.fede$Cost, col="green", type="l", pch=16)
legend("topleft", legend = c("gpu", "swap","fede"), col = c("red","blue","green"), pch = c(16,16), cex = 0.75)
grid()
}


#-------------------------------------------------------------------------------
# GGPLOT

time = 
  ggplot() +
  geom_line(data=df.swap2, aes(x=1:50, y=Time, colour = "Swap"), size = 1) +
  geom_line(data=df.gpu, aes(x=1:50, y=Time, colour = "Gpu"), size = 1) +
  geom_line(data=df.fede, aes(x=1:50, y=Time, colour = "Fede"), size = 1) +
  ylim(c(5e5, 1e6)) +
  ggtitle("Time vs nodes") + xlab("Nodes") + ylab("Time") +
  labs(color='Method') 


cost = 
  ggplot() +
  geom_line(data=df.swap2, aes(x=1:50, y=Cost, colour = "Swap"), size = 1) +
  geom_line(data=df.gpu, aes(x=1:50, y=Cost, colour = "Gpu"), size = 1) +
  geom_line(data=df.fede, aes(x=1:50, y=Cost, colour = "Fede"), size = 1) +
  ggtitle("Cost vs nodes") + xlab("Nodes") + ylab("Cost") +
  labs(color='Method') 


grid.arrange(time, cost, ncol=2)


#-------------------------------------------------------------------------------

# FIRST IMPROVEMENT vs BEST IMPROVEMENT (GPU)

time = 
  ggplot() +
  geom_line(data=df.gpu.best, aes(x=1:50, y=Time, colour = "Best"), size = 1) +
  geom_line(data=df.gpu.first, aes(x=1:50, y=Time, colour = "First"), size = 1) +
  ylim(c(5e5, 1e6)) +
  ggtitle("Time vs nodes") + xlab("Nodes") + ylab("Time") +
  labs(color='Method') 

cost = 
  ggplot() +
  geom_line(data=df.gpu.best, aes(x=1:50, y=Cost, colour = "Best"), size = 1) +
  geom_line(data=df.gpu.first, aes(x=1:50, y=Cost, colour = "First"), size = 1) +
  ggtitle("Cost vs nodes") + xlab("Nodes") + ylab("Cost") +
  labs(color='Method') 


grid.arrange(time, cost, ncol=2)


