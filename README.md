# Distributed Delaunay triangulation & surface reconstruction on Spark / hadoop : start release  :)
This project aims to process algorithm based on Delaunay triangulation on distributed infrastructures .
We use Apache Spark for the cloud infrastructure.

To use the code, please reffer to the [user manual](#user-manual) section
For more information about the architecutre and the source code,
reffers to the [dev manual](#dev-manual) section


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

### test examples 
```console
./run_examples.sh

```


