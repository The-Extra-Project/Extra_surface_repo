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
- 8go RAM

# User manual
## Install & compile 
- Edit the file algo-env.sh 
### Build the docker image
```console
$ ./src/docker/docker_interface.sh build
```

### Compile the project 
To compile the project with the 3D CGAL kernel with 4 core, do :

```console
$ ./src/docker/docker_interface.sh compile -j4 -t3
```

# Run the code
## Examples 
Run the 3 examples (monotread, multithread with apache spark on ply, multithread with apache spark on laz)
```console
$ ./run_examples.sh

```
results will be writen in 'outputs' directory.


## Run on LidarHD LAZ dataset 
To run the code on a lidarHD tile : 
- Go to [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) dataset and download a tile.
- Save the downloaded tile into a folder on your computer.
- Set the absolute path of this folder as the value of the `INPUT_DIR` variable in the `run_lidarhd.sh` file.
- Adjust the settings to optimize local Apache Spark scheduling on your computer according to your preferences in the `algo-env.sh` file.
- then run the script `$ ./run_lidarhd.sh`

# dev manual
In this section, the main workflow for LAS lidar point cloud processing is detailled

## Parameters setting / General information
The actuel parametrization is made to produce a "good" result on the LiDARHD dataset.
Because  of the approximate line of sight estimation (bad estimation on the building)
The algorithm confidence is drastically decrese in order to be able to reconstructe building (otherwise many surfaces where the normal is horizontal are badly oriented)
The priority is actually to improve the sensor origin estimation.


## LAZ preprocessing
The first step is to transform a LAZ file to a ply with with the following fileds
  - x,y,z (3D points cloud coordinate) 
  - x_origin,y_origin,z_origin (Sensor origin coordinate)
And the algorithm parameter
  - metadata.xml files

The origin of the sensor is estimated by using the adaptated code from CGAL to estimate the line of sight


## Surface reconstruction
Two workflow are aviable,
- a monothread workflow that takes a ply file and produce
many result with different regularization parameter (good for test)
- A distributed workflow, scheduled with apache spark.

## Lod Surface
While the distributed surface reconstruction is calculated
we provide a tool to produce a different level of detail mesh 'services/mesh23dtile/'

# Todos
- ☐ improving line of sight estimation
