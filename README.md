[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

![logo](./doc/logo.jpeg)

![Example of the algorithm on the "chateau de versaille" LidarHD tile](./doc/header.jpeg)


# Distributed watertight surface reconstruction on Apache Spark. 

This project aims to process algorithm for watertight surface reconstruction based on Delaunay triangulation on distributed infrastructures.
This code is a simplfied version of the article  [Efficiently Distributed Watertight Surface reconstruction](https://lcaraffa.github.io/edwsr/)
finetuned to work on the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) on a signe computer.

To use the code, please reffer to the [user manual](#user-manual) section.
For more technicals informations, reffers to the [dev manual](#dev-manual) section.

- **/!\ Warning /!\\**  This code is for experimental / researches purposes and will not be maintened in a first place ⇨ we are  developping in priority a CGAL package scheduled with Open MP / MPI. Follow the project page or this github page for updates.

- The code is published under the  GNU GENERAL PUBLIC LICENSE V3 ([LICENCE.md][LICENCE.md])

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
results will be writen in 'outputs' directory.

### Run on LidarHD LAZ dataset 
To run the code on a lidarHD tile : 
- Go to [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) dataset and download a tile.
- Save the downloaded tile into a folder on your computer.
- Set the absolute path of this folder as the value of the `INPUT_DIR` variable in the `run_lidarhd.sh` file.
- Adjust the settings to optimize local Apache Spark scheduling on your computer according to your preferences in the `algo-env.sh` file.
- then run the script `$ ./run_lidarhd.sh`

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

## Todos
- ☐ improving line of sight estimation
