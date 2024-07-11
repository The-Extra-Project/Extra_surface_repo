[![arXiv](https://img.shields.io/badge/arXiv-Paper-<COLOR>.svg)](https://hal.science/hal-03380593/file/2021216131.pdf)

![logo](./doc/logo.jpeg)

![Example of the algorithm on the "chateau de versaille" LidarHD tile](./doc/header.jpeg)


# Distributed watertight surface reconstruction on Apache Spark. 

This project aims to produce watertight meshes on large scale datasets. This code is a simplified version of the article [Efficiently Distributed Watertight Surface Reconstruction](https://lcaraffa.github.io/edwsr/), fine-tuned to work on the [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) on a single computer.

To use the code, please reffer to the [user manual](#user-manual) section.
For more technicals informations, reffers to the [dev manual](#dev-manual) section.

 **/!\\/!\\/!\\   Warning  /!\\/!\\/!\\**
- This code is for experimental and research purposes and will not be maintained initially. A CGAL package with OpenMP/MPI scheduling is currently being prioritized. Follow the project page or this GitHub page for updates.
- This code is fine-tuned to work on the LiDAR HD dataset with hardcoded hacks to passby bad line of sligt estimation (the effect of a point with the line of sligt coplanar to the surface is reduced).
- This code runs efficiently on an Apache/Spark cluster but performs worse on a single computer compared to traditional OpenMP/MPI scheduling..
- The surface reconstruction score function is basic and may produce poor results compared to advanced algorithms. Any improvements in this area are welcome. 

- This code is published under the  GNU GENERAL PUBLIC LICENSE V3 (See LICENCE.md)

# Requirements 
- Docker (only tested on Ubuntu)
- 8GB RAM single computer
- conda 
- libpq-devel 

# User manual
## Install & Compile 
- Edit the file algo-env.sh 

```console
## Number of core used by spark
export NUM_PROCESS="2"
```

### Build the docker image
```console
$ ./src/docker/docker_interface.sh build
```

### Compile the project 
To compile the project with the 3D CGAL kernel using 4 cores, run :

```console
$ ./src/docker/docker_interface.sh compile -j4 -t3
```

### Create level of detail mesh
For producing a LOD mesh, first create the conda env : 

```console
$ conda env create --file ./services/mesh23dtile/environment.yml
```	

before runing the next examples, activate the conda env :

```console 
$ conda activate mesh23Dtile
```	

# Run the code
## Examples 
Run all the examples:
```console
./run_workflow.sh --input_dir ${PWD}/datas/lidar_hd_crop_1/ --output_dir ${PWD}/outputs_examples/lidar_hd_crop_1
./run_workflow.sh --input_dir ${PWD}/datas/lidar_hd_crop_2/ --output_dir ${PWD}/outputs_examples/lidar_hd_crop_2

```
Results will be created in the 'outputs' directory.


## Run on LidarHD LAZ dataset 
- To run the code on a LiDAR HD tile collection : 
  - Go to [LiDAR HD dataset](https://geoservices.ign.fr/lidarhd) dataset and download the txt file containing the list of laz file here : `./datas/liste_dalle.txt`.
  - then run the script `$  ./run_lidarhd.sh --list_files ${PWD}/datas/liste_dalle.txt --output_dir ${PWD}/outputs/lidar_hd/` 

ps : You may have to adjust the settings to optimize local Apache Spark scheduling on your computer  in the `algo-env.sh` file.


# Visualize inside ITown

![logo](./doc/lod.jpg)

Clone and install [ITowns](https://github.com/iTowns/itowns)
Copy the LODs folter into the root directory of itown 
You can edit the example in `itowns/examples/3dtiles_basic.htm` with following code
```html
  // Create a new Layer 3d-tiles For DiscreteLOD
// -------------------------------------------
            var $3dTilesLayerDiscreteLOD = new itowns.C3DTilesLayer('3d-tiles-discrete-lod', {
                name: 'DiscreteLOD',
                sseThreshold: 0.5,
                source: new itowns.C3DTilesSource({
      		    url: '/LODs/tileset_final.json',
                }),
            }, view);
```
Start itowns then go to http://localhost:8082/examples/#3dtiles_basic 
# dev manual
In this section, some technical information about LAS LiDAR point cloud processing is added.

## Parameters setting / General information
The current parameters are set for the LiDAR HD dataset.
Because of the approximate line of sight estimation (poor estimation on buildings with surfaces where the normal is horizontal), the algorithm confidence is drastically decrese. Some high building may not be correctly reconstructed in the actual version


## LAZ preprocessing
The first step is to transform a LAZ file to a ply with with the following fileds
  - x,y,z ⇨ 3D points cloud coordinate
  - x_origin,y_origin,z_origin ⇨ Sensor origin coordinate

The origin of the sensor is estimated by using the adaptated code from CGAL to estimate the line of sight.

The workflow "${DDT_MAIN_DIR}/services/wasure/workflow/workflow_preprocess.scala"
will automatically generate a complete xml configuration file.

## Parameters doc 
### Generic Algo parameters
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
### Surface reconstruction Algo parameters
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



## Surface reconstruction
Two workflow are aviable :
- A monothread workflow that takes a ply file and produce
many result with different regularization parameter (good for test)
- A distributed workflow, scheduled with apache spark.

# Todos
- ☐ improving line of sight estimation
- ☐ improving surface reconstruction score function with state of the art approach.
