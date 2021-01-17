## Distributed Delaunay Triangulation & surface reconstruction
## Readme

Distributed Delaunay triangulation and surface reconstruction on Spark / hadoop

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
The scriptfile "./compile_all_kernel.sh" shows all aviable kernel compile command


## Runing a workflow 

### Examples
Several examples are aviable in the script "run_all_workflow.sh"
#### Inputs
The input of the algorithm is a set of plyfile
- For local version -
  classic ply file is used "*.ply"
- For Cloud version(using HDFS)
onliner text ply is mandatory (*.stream)
The *.stream can be produce with the preprocessing_step

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
  </datasets>
</env>
```
### Surface reconstruction Algo parameters PARAMS
Here is parameters that can be added for the surface reconstruction Algorithm
```xml
      <!-- Dempster Shafer -->
      <!-- number of sampling -->
      <nb_samples>1</nb_samples>

      <!-- Regularization term
      lambda     : Smoothing term		  
      max_opt_it : max number of iteration durting the distributed graphcut
      dump_debug : dump debug results
      -->
      <lambda>0.1</lambda>
      <max_opt_it>10</max_opt_it>
      <dump_debug>false</dump_debug>

```

# Dev Manual
## Introduction
This projet aim to schedule algorithms on the cloud based on Delaunay triangulation.
For that, the Delauany trinagulation is decomposed in tiles where each tile is overlapped with its neighbors

![fig](https://github.com/lcaraffa/spark-ddt/blob/master/doc/dt_struct.png?raw=true)
*The structure of the distributed triangulation is the folowing, from left to right : First image, the full triangulation. 
Second image shows in color the DT of a supset of the local points of the bottom left corner tile. 
These extra points, denoted as foreign points are called redundant if they are not adjacent to a local point. Simplices may be categorized as local,  
mixed or foreign. The final image shows the tile after simplification, by removing redundant foreign points from the local triangulation.*



### Distributed Delauney triangulation
A distributed algorithm based on the tiling structure is implemented based on this article. (see https://hal.archives-ouvertes.fr/hal-02551509)
The workflow of the algorithm is the following :

![fig](https://github.com/lcaraffa/spark-ddt/blob/master/doc/workflow_wasure.png?raw=true)
*Proposed distributed Delaunay triangulation workflow in Spark. P^r denotes the input point set, accessible through chunks P^r_k, P_i the tiled point,
 T^0_i the local triangulation of the tile, S^0_{i -> all} the first broadcasted point set, S_{i -> j}^n the point set sent from tile i to tile j, 
Solid (resp. dashed) red arrows show active (resp. inactive) connections between two tiles. 
C^0_i the finalized cells after the first triangulation.
Dashed boxes denote geometrical processing transformations (e.g. Ins;Simp;Splay  denotes an insertion following by a simplification and starsplaying). 
The color of the node represents the Spark persistence level.*


Once this delaunay triangulation is performed, several opperation can be done on it.




### Surface reconstruction : 
A distributed surface reconstruction algorithm is implanted based on the Inside/Outside segmentation of the space 
The goal is to label each tets of the triangulation as Inside or Outside.



### Architecture
The following chart shows the workflow of the distributed delaunay triangulation

![fig](https://github.com/lcaraffa/spark-ddt/blob/master/doc/workflow_ddt.png?raw=true)



All the communications between executors are preformed by a streaming architecture scheduled with Spark. Data sets are both persisted on memory and disk. Large data sets like input point clouds and finalized cells are stored only on disk (green color in figure~\ref{fig:algorithmflow}) and lightweight data sets like simplified triangulation during the iterative scheme that are likely to have multiple I/O are stored both in memory and disk.% (red color in figure~\ref{fig:algorithmflow}).
In this section, the implementation choices are detailed.
For implementing, both \CC and Spark are used.
On a higher level, Spark provides a fault-tolerant and lazy programming language that distributes efficiently the operations on the cluster according to the data location thanks to the use of resilient distributed data sets ($RDD$).

\paragraph{C++}
\CC is widely used in computational geometry because of the low-level architecture that allows high speed computational time.
Each geometric operation is implemented with a \CC 
executable leveraging the CGAL library. Once a triangulation is computed, the \CC sets are encoded with the \emph{Base64} encoding, consequently, the data integrity is guarantied across transformations.
%This base64 serialization enables to communicate losslessly with a reasonable overhead between Spark and the \CC executable using the plain standard inputs and outputs.

%% For a triangulation, it is composed by a vector of double for points coordinate and two vectors of integer for the simplex encoding and the neighbours relations.
%% In the next \CC call, The stream received by the Spark Scheduler is decoded and the local view of the triangulation rebuilds.

\paragraph{Spark}
The Scala interface of Spark is used for the scheduling process.
Each point set $P_I$ and triangulation $T_I$ are stored in a $RDD$ of size $|I|$ serialized with a \emph{Base64} encoding in a \emph{String}. The key/value formalism is used. Each element of the $RDD$ is represented by a key $K$ and a value $V$ which is a list of sets (Point Set, local view of a triangulation, etc.). The value is then  $V=List[String]$, finally we have $RDD[(K,V)]$.
To implement the shuffling $S_{i -> j}$, the GraphX library~\cite{bib:graphx} is used. An exchange is stored as an edge of a graph with the triplet $RDD[(K_i,K_j,V)])$ where $K_i$ is the key of the source and $K_j$ the key of the target.
%\paragraph{Transformation}
%% Spark works with transformation abstraction. All Spark transformations are \emph{lazy}, it means that nothing is computed before an action is called.
For each $RDD$ transformation that requires geometric processing, the content of a $RDD$ is encoded in base64 and streamed to a \CC
executable by using the transformation \emph{pipe} operator for Spark $RDD$. 
Since each serialized value encodes its own key, the \CC thread can interpret and build the local view of the triangulation.
As an example, the local triangulation step (alg~\ref{alg:overall}.\ref{alg:tile}) which is a map function of %% each triangulation is written as:

```scala
    RDD[(K,P_i)].pipe(./Delaunay)
```


where ./Delaunay is a \CC executable that takes as input the streamed point cloud and stream a new  triangulation
The union operator ($\cup$ in alg:\ref{alg:overall} and $\diamond$ in figure~\ref{fig:algorithmflow}) is a union following by a \emph{ReduceByKey} in Spark.
As an example, the star splaying step (lines~\ref{alg:star1} and \ref{alg:star2}) can be written as follows:
\begin{footnotesize}
\begin{alignat*}{2}
  &( && RDD[(K_i,K_j,S_{i \rightarrow j})].map(e \rightarrow (e.K_j,e.V) )  \cup  RDD[(K,T_j)] \\
 &).&&reduceByKey((a,b)\rightarrow (a \cup b)).pipe(\text{./StarSplay}) 
\end{alignat*}
\end{footnotesize}
Where $\text{./StarSplay}$ is a \CC executable that takes as input the union of the previous step triangulation and the received points, do the insertion, the simplification, extract the stars and produce as output the new triangulation with the new points to send.
Finally, a filter operator is used to separate the output stream of each \CC call.

## Code
The source code of this project is decomposed in 3  parts :
### Distributed delaunay triangulation core
    The c++ classes for the distributed delaunay triangulation structure are sotred in ./src/core
### Scheduler : The spark-core API
For the scheduling, we use Scala and Spark.
The core of the scheduler is in the folder ./src/spark/src/main/scala/ 
  
### Services
A service is an algorithm based on the distributed delauany triangulation structure.
- Distribured delauany triangulation
  This service aim to build the full distributed delaunay triangulation.
  The c++ core function is in ./services/ddt/src/
  The Spark scheduler is in ./services/ddt/workflow/ 

- Watertight surface reconstruction
  This service aim to performe the surface reconstruction on point cloud.
  The c++ core function is in ./services/wasure/src/
  The Spark scheduler is in ./services/wasure/workflow/ 

