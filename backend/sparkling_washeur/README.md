# Backend Repository


## Docker image build description

The image consists of the multiple build stages:
- hdfs - compile hadoop libs, specifically libhdfs.so
- build - core image for the c++, spark iqlib and app JARs code compilation.
- production - the final image that gets run in EMR

<br> N.B. the multi-stage build utilizes cache a lot, clean up periodically to free up storage: 
```bash
sudo docker system df  # get the ttl cache size
sudo docker system prune -f  # remove cache
sudo docker rmi $image_hash  # delete image
```

#### Libraries compilation
The `build` image installs all the necessary dependencies for the cpp/scala code compilation.
You thus don't need to install them on your local host. <br>

However, you can still choose the actual place of the compilation (your local host, or the docker container).
The local host is preferrable due to the better performance. In CICD case, choose the latter. <br>

You can switch the configuration via the following command in the dockerfile (the last one in the build stage):
```bash
RUN ./src/docker/docker_interface.sh compile -j $JOBS
```

After you've configured the compilation place: <br>

1. Create the build container:
```bash
docker build -t build -f Dockerfile.emr --target=build .
```

2. Build
- local: `src/docker/docker_interface.sh compile -j $JOBS`
- container: do nothing, docker will run the build automatically

3. Assemble Scala preprocessing/processing JARs. <br>
You will need the `iqlib.jar` built on the previous step.

4. Upload the iqlib.jar, preprocessing/processing jars to S3 <br>

5. You may now run the JARs in EMR.