[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

![logo](./doc/logo.jpeg)

![Example of the algorithm on the "chateau de versaille" LidarHD tile](./doc/header.jpeg)



# Distributed watertight surface reconstruction on Apache Spark. 

This project aims to process algorithm for watertight surface reconstruction based on Delaunay triangulation on distributed infrastructures.
This code is a simplfied version of the article  [Efficiently Distributed Watertight Surface reconstruction](https://lcaraffa.github.io/edwsr/)
finetuned to work on the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) on a signe computer.

To use the code, please reffer to the [user manual](#user-manual) section.
For more technicals informations, reffers to the [dev manual](#dev-manual) section.

**/!\ Warning /!\\**  This code is for experimental / researches purposes and will not be maintened in a first place ⇨ we are  developping in priority a CGAL package scheduled with Open MP / MPI. Follow the project page or this github page for updates.

# Requirements 
- Docker (only tested on ubuntu)

# User manual
## Install & compile 
- Create and edit the file algo-env.sh from the template algo-env.sh.conf 

### Build the docker image
```console
$ ./src/docker/docker_interface.sh build
```

### Compile the project 
To compile the project with the 3D CGAL kernel with 4 core, do :

```console
$ ./src/docker/docker_interface.sh compile -j4 -t3
```

## Run the code


### Examples 
Run the 3 examples (monotread, multithread with apache spark on ply, multithread with apache spark on laz)
```console
$ ./run_examples.sh

```

### Run on LidarHD LAZ dataset 
- Download a tile from the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) into a folder an set the absolute path in the `INPUT_DIR` variable (`./run_lidarhd.sh` file).
- edit the `algo-env.sh` to finetune the local apache spark scheduling on your computer
```console
export SPARK_EXECUTOR_MEMORY="16G"
export SPARK_DRIVER_MEMORY="16G"
export SPARK_WORKER_MEMORY="16G"
export NUM_PROCESS="10"
```

- then run 
```console
$ ./run_lidarhd.sh

```

# dev manual
## Inputs
A folder with inside : 
- a Set of ply files with, for each 3D points
  - x,y,z (3D points cloud coordinate) 
  - x_origin,y_origin,z_origin (Sensor origine coordinate)
- metadata.xml files 

## LAZ preprocessing
A workflow exists to preprocess a las file into a ply with sensor origin approximation
see the run_lidar.sh


