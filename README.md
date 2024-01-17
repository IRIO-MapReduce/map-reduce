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
docker build -t mapreduce-master:latest -f master/Dockerfile .
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

## Compile packages using vcpkg
Install vcpkg in **DESIRED DIRECTORY**.
```bash
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
```

Install desired packages (for example google_cloud_cpp_compute):
```bash
./vcpkg/vcpkg install google_cloud_cpp_compute
```

In CMakeLists.txt, include line (setting correct path to vcpkg):
```cmake
set(PATH_TO_VCPKG /path/to/vcpkg)
set(CMAKE_TOOLCHAIN_FILE ${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
```

Additionally, remember to add find_package and target_link_libraries as needed, for example:
```cmake
find_package(google_cloud_cpp_compute REQUIRED)
target_link_libraries(main PRIVATE ... google-cloud-cpp::compute)
```