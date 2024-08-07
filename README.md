## Extra-surface
UI and SaaS platform that allows the user to schedule 3D reconstructions (by paying a nominal fee for supporitng the EC2 hosting costs)  of LidarHD tiles on given instance and send results to the user for visualization.  

## Credits:

- [Distributed watertight surface reconstruction, Laurent Caraffa et.al](https://lcaraffa.github.io/edwsr/)  

```
@inproceedings{caraffa:hal-03380593,
  TITLE = {Efficiently Distributed Watertight Surface Reconstruction},
  AUTHOR = {CARAFFA, Laurent and Marchand, Yanis and Br{\'e}dif, Mathieu and Vallet, Bruno},
  URL = {https://hal.archives-ouvertes.fr/hal-03380593},
  BOOKTITLE = {2021 International Conference on 3D Vision (3DV)},
  ADDRESS = {London, United Kingdom},
  YEAR = {2021},
  MONTH = Dec,
  PDF = {https://hal.archives-ouvertes.fr/hal-03380593/file/2021216131.pdf},
  HAL_ID = {hal-03380593},
  HAL_VERSION = {v1},
}
```

## Stack components: 

It consist of the following stack components to run the application:

<img src="doc/database_schema.png"></img>

- [supabase](): For storing the user session database (their payment, current job) , current reconstruction of the tiles status etc. 

- [stripe](): For the payment of the compute costs (fixed per tile in the [frontend]() code).

- [sparkling_washeur](): Consisting of the reconstruction pipeline with dockerised version.

- [resend](): Service to send the email notifications describing user the different state and the retrieval of the results from the backend once its generated.

- [AWS S3](): Storage service in order to store: 
  - The laz files recovered from the diffusion lidarhd text url file (uploaded by the user).
  - The output results after the reconstruction.

- [redis](): For enqueuing the requests with the RQ wrapper to sequentially execute the jobs.

## Configuration: 

### 1. cp .env.example .env with following parameters :

1. .env.example stored from the [backend/api](backend/api/.env.example) needs to be initialized with: 
  - Credentials from the supabase: 
  - Address of the sender email that needs to be cc'd for the notification system.
  - S3 credentials including the bucket address and the directory key.
  - Also insure that you've setup the aws login credentials in the local instance, as [docker-compose](docker-compose.yml) mounts them into the container.

2. For [sparkling-washeur](), define the parameters of the cluster in [algo-env](backend/sparkling_washeur/algo-env.sh).
  - Along with the parameters for the cores for compilation in the docker-compose.
  - Build the enviornment in order to setup the docker container with the conda enviornment for the [sparkling_washeur]().

3. [Frontend](): 
  - Create stripe test developer account and then paste the values.
  - In case of deployment on local keep the NEXT_PUBLIC_SITE_URL by exposing the output following the [given tutorial](https://github.com/vercel/next.js/discussions/16429#discussioncomment-1302156).

### 2. Build and deploy the Microservices. 
docker compose build && docker compose up -d

### 3. run the application on the following endpoints 

| location  | URL |
| ------------- | ------------- |
| frontend  | https://localhost:8080  |
| backend  | https://localhost:8081  |
| sparkling_washeur  | https://localhost:8082  |
| redis|  https://localhost:6379 |
