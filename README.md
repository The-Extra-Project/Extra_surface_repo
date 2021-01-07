## Distributed Delaunay Triangulation & surface reconstruction doc
## Readme


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
The scriptfile "./compile_all_kernel.sh" compile all aviable kernel


## Runing a workflow 

### Examples
Several examples are aviable in the script "run_all_workflow.sh"

### Command Line

To run a workflow, use the following command
```console
./src/docker/docker_interface.sh run_algo_spark  -i ${INPUT_DIR} -p ${PARAMS} -o ${OUTPUT_DIR} -f ${FILE_SCRIPT}  -s master -c N -m ${MASTER_IP_SPARK} -b ${BUILDS_DIR} ${DEBUG_FLAG}"
```

With the following variables
- INPUT_DIR : the repertory with point cloud.
- PARAMS ; the path to the xml param file.
- OUPUT_DIR : The output dirrectory.
- FILE_SCRIPT : The workflow script.
- N : The number of core in local mode.
- MASTER_IP_SPARK : The ip of the master (localhost by default).
- BUILDS_DIR : The path of the build directory.
- DEBUG_FLAG : set as "-d" to go into the docker container without runing the script.


### Algo parameters PARAMS
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
      -->
      <dim>2</dim>
      <ddt_kernel>build-spark-Release-2</ddt_kernel>
      <StorageLevel>MEMORY_AND_DISK</StorageLevel>
      <ndtree_depth>3</ndtree_depth>
      <bbox>0000x10000:0000x10000</bbox>
      <max_ppt>12000</max_ppt>

      <!-- Meta parameters -->
      <do_process>true</do_process>
      <plot_lvl>3</plot_lvl>
      <do_process>true</do_process>
      <do_expand>false</do_expand>
      
    </setn1>


    <setn2>
      <!-- Plotting level (0 : nothing, 3(max) -->
      <plot_lvl>3</plot_lvl>
      <do_process>true</do_process>
      <datatype>files</datatype>
      <dim>2</dim>
      <ndtree_depth>3,4,5</ndtree_depth>
      <bbox>0000x10000:0000x10000</bbox>
      <max_ppt>12000</max_ppt>
      <pscale>20</pscale>

      <nb_samples>1</nb_samples>

      <lambda>1</lambda>
      <lambda>10</lambda>

      <mode>surface</mode>
      <algo_seed>1000</algo_seed>
      <StorageLevel>MEMORY_AND_DISK</StorageLevel>
      <!-- Meta parameters -->
      <do_process>true</do_process>
      <do_expand>true</do_expand>
      
    </setn2>

  </datasets>
</env>
```

In this part of the documentatino, only global parameters are shown, 
Each service has their own parameters (described in the README of the service