#!/bin/bash
#PBS -N spark-cluster
#PBS -l select=4:ncpus=5:mem=20000Mb:generation=g2019
#PBS -l walltime=00:40:00
module load spark
echo "spark.local.dir $TMPDIR" > properties.file
pbs-launch-spark -n 5 -m 18000M -p properties.file
