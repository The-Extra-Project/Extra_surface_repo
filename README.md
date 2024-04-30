[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

# Distributed Delaunay triangulation & surface reconstruction on Spark / hadoop. 

This project aims to process algorithm for watertight surface reconstruction based on Delaunay triangulation on distributed infrastructures.
This code is a simplfied version of the article  [Efficiently Distributed Watertight Surface reconstruction](https://hal.science/hal-03380593/file/2021216131.pdf)
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
To compile the project with the 3D CGAL kernel with 4 core, do :

```console
./src/docker/docker_interface.sh compile -j4 -t3
```

## Run the code
### Examples 
Run the 3 examples (monotread, multithread with apache spark on ply, multithread with apache spark on laz)
```console
./run_examples.sh

```

### Run on LidarHD dataset 
- Download a tile from the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) into a folder an set the absolute path in the `INPUT_DIR` variable (`./run_lidarhd.sh` file).
- then run 
```console
./run_lidarhd.sh

```




