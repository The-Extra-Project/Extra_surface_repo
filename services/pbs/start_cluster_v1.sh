#!/bin/bash
#PBS -N spark-cluster
#PBS -l select=4:ncpus=5:mem=20000Mb
#PBS -l walltime=00:40:00

module load spark
pbs-launch-spark -n 5 -m 20000M
