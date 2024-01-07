# map-reduce


## Build

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
docker build -t mapreduce-master:latest -f master/Dockerflie .
```

Build the testing client.
```bash
docker build -t test-client:latest -f tests/Dockerfile .
```

## Run

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