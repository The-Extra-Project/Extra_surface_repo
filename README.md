[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

# Distributed Delaunay triangulation & surface reconstruction on Spark / hadoop. Experimental code

This project aims to process algorithm based on Delaunay triangulation on distributed infrastructures for watertight surface reconstruction.
This is a simplfied version of the article  [Efficiently Distributed Watertight Surface reconstruction](https://hal.science/hal-03380593/file/2021216131.pdf)
 finetuned to work on the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) on a signe computer for experimental purpores.

# Requirements 
- Docker (only tester on ubuntu)

# User manual
## Install & compile 
- Create and edit the file algo-env.sh from the template algo-env.sh.conf 

### Build the docker image
```console
./src/docker/docker_interface.sh build
```

### Compile the project
For example, to compile the project with the 3D CGAL kernel with 4 core, do :


```console
./src/docker/docker_interface.sh compile -j4 -t3
```

### Examples 
Run the 3 examples 
```console
./run_examples.sh

```

### Run on LidarHD dataset 
- download a tile from the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) into a folder.
- Edit the file "run_lidarhd.sh" and put the path of the file folder in the variable INPUT_DIR
- then run "./run_lidar.sh"
```console
./run_examples.sh

```




