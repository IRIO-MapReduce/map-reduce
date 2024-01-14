# map-reduce

## Docker images

### Build

First build the image with C++ gRPC library.
```bash
docker build -t grpc-cpp-image:latest -f grpc-docker-image/Dockerfile .
```

Then build the project.

```bash
cd src/
```

Build the master container.
```bash
docker build -t mapreduce/master:latest -f master/Dockerfile .
```

Build the testing client.
```bash
docker build -t test-client:latest -f tests/Dockerfile .
```

### Run

Run the master (currently the server of MapReduce).
```bash
docker run --rm -p 50051:50051 mapreduce-master:latest
```

And then run the testing client.
```bash
docker run --rm --network=host test-client:latest
```

After the testing is finished, stop the master container.
```bash
docker container ls # copy the container id of the mapreduce-master container
docker stop [Copied container id]
```

### Push to artifact registry

```
gcloud artifacts repositories create mapreduce --repository-format=docker \
--location=us-central1
```

```
gcloud auth configure-docker us-central1-docker.pkg.dev
```

Tag the images (run for mapper and reducer similarly)
```
docker tag mapreduce/master us-central1-docker.pkg.dev/${PROJECT_ID}/mapreduce/master
```

Push images to the registry (run for mapper and reducer similarly)
```
docker push us-central1-docker.pkg.dev/${PROJECT_ID}/mapreduce/master
```


## Gcloud deployment

To create deployment run the following command from *src* directory

```
gcloud deployment-manager deployments create mapreduce \ 
    --config deployment.yaml
```

To update config changes run 
```
gcloud deployment-manager deployments update mapreduce \
    --config deployment.yaml
```


To create gcloud instance template run
```
gcloud compute instance-templates create-with-container mapper-vm \
    --container-image hub.docker.com/r/pit4h/grpc-cpp-image
```

### Master

```bash
gcloud compute instances create-with-container master-vm \
    --zone us-central1-a \
    --container-image=us-central1-docker.pkg.dev/pb-map-reduce/mapreduce/master
```

You can update the image by using
```
gcloud compute instances update-container master-vm \
    --container-image=us-central1-docker.pkg.dev/pb-map-reduce/mapreduce/master
```

### Mappers

Create an instance template
```bash
gcloud compute instance-templates create-with-container mapper-template \
    --machine-type=e2-standard-2 \
    --container-image=us-central1-docker.pkg.dev/pb-map-reduce/mapreduce/mapper
```

Create Managed Instance Group
```bash
gcloud compute instance-groups managed create mappers \
    --size 5 \
    --template mapper-template \
    --zone us-central1-a
```


### Clean up

```
gcloud compute instances delete master
```

```
gcloud compute instance-groups managed delete mappers \
    --zone us-central1-a
```
