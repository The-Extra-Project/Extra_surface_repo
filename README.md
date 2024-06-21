[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

![logo](./doc/logo.jpeg)

![Example of the algorithm on the "chateau de versaille" LidarHD tile](./doc/header.jpeg)


# Distributed watertight surface reconstruction on Apache Spark. 

This project aims to process algorithm for watertight surface reconstruction based on Delaunay triangulation on distributed infrastructures.
This code is a simplfied version of the article  [Efficiently Distributed Watertight Surface reconstruction](https://lcaraffa.github.io/edwsr/)
finetuned to work on the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) on a signe computer.

To use the code, please reffer to the [user manual](#user-manual) section.
For more technicals informations, reffers to the [dev manual](#dev-manual) section.

 **/!\/!\/!\   Warning /!\/!\/!\\**
- This code is for experimental / researches purposes and will not be maintened in a first place ⇨ we are  developping in priority a CGAL package scheduled with Open MP / MPI. Follow the project page or this github page for updates. 
- This code is optimized for efficiency on an Apache/Spark cluster, so it performs worse on a single computer compared to traditional OpenMP/MPI scheduling.
- The surface reconstruction score function is very basic and may produce bad result compare to state of the art surface reconstruction algorithms  ⇨  Any contribution / improvement in this area is welcome
- This code is published under the  GNU GENERAL PUBLIC LICENSE V3 (See LICENCE.md)

# Requirements 
- Docker (only tested on ubuntu)
- 8go RAM single computer

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
Run all the examples
```console
$ ./run_examples.sh

```
results will be created in the 'outputs' directory.


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
The actuel parameters are set for the LiDARHD dataset.
Because of the approximate line of sight estimation (bad estimation on the building), the algorithm confidence is drastically decrese in order to be able to reconstructe building (otherwise many surfaces where the normal is horizontal are badly oriented). Some high building may not be correctly reconstructed in the actual version


### Generic Algo parameters PARAMS
Each workflow can be parametrized with an xml file of the form :
```xml
<env>
  <datasets>
    <setn1>
      <!-- Input paramters 
      datatype : The type of file
      	       - files : ply file on disk direclty read by c++ call
	       - filestream : onliner ply file that will be read by sc.parallelize(...)
      -->
      <datatype>files</datatype>

      <!-- Algo env setup
      dim : Dimentionnality of the algorithm.
      ddt_kernel : C++ kernel folder (in the build dir)
      StorageLevel : Principal persistance  storage level 
      		   (see for instance https://jaceklaskowski.gitbooks.io/mastering-apache-spark/spark-rdd-caching.html)
      ndtree_depth ; Depth of the Octree in 3D
	  bbox : Bounding box of the point cloud. Points outside will be ignored
	  max_ppt : max number of point per tile
      -->
      <dim>3</dim>
      <ddt_kernel>build-spark-Release-2</ddt_kernel>
      <StorageLevel>MEMORY_AND_DISK</StorageLevel>
      <ndtree_depth>3</ndtree_depth>
      <bbox>0000x10000:0000x10000</bbox>
      <max_ppt>12000</max_ppt>

      <!-- Meta parameters -->
      <do_process>true</do_process>
      
    </setn1>
  </datasets>
</env>
```
### Surface reconstruction Algo parameters PARAMS
Here is parameters that can be added for the surface reconstruction Algorithm
```xml
      <!-- Dempster Shafer -->
      <!-- 
	  nb_samples : number of sampling for the integral computation
	  -->
      <nb_samples>50</nb_samples>

      <!-- Regularization term
      lambda     : Smoothing term		 
	  pscale : Accuracy of the reconstruction (lower values indicate higher precision) 
      max_opt_it : max number of iteration durting the distributed graphcut
      -->
	  <pscale>0.05</pscale>
      <lambda>0.1</lambda>
      <max_opt_it>50</max_opt_it>

```


## LAZ preprocessing
The first step is to transform a LAZ file to a ply with with the following fileds
  - x,y,z ⇨ 3D points cloud coordinate) 
  - x_origin,y_origin,z_origin ⇨ Sensor origin coordinate

The origin of the sensor is estimated by using the adaptated code from CGAL to estimate the line of sight


## Surface reconstruction
Two workflow are aviable,
- A monothread workflow that takes a ply file and produce
many result with different regularization parameter (good for test)
- A distributed workflow, scheduled with apache spark.

## Lod Surface
we provide a tool to produce a different level of detail mesh from a tiled mesh in  'services/mesh23dtile/'

# Todos
- ☐ improving line of sight estimation
- ☐ improving surface reconstruction score function with state of the art approach.
