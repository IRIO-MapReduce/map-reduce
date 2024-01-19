# map-reduce

## Building locally

* Set `VCPKG_DIR` environment variable to your path to the vcpkg package manager.
* Go to the `src/master` or `src/worker` directory
* Build the module with CMake in `build` directory:
```bash
cmake .. -DPATH_TO_VCPKG=${VCPKG_DIR}
```

## Gcloud deployment

Initialize gcloud
```bash
gcloud init
```

Set the PROJECT_ID variable
```bash
export PROJECT_ID=$(gcloud config get-value project)
```

### Filestore

Create Filestore instance
```bash
gcloud filestore instances create franek \
    --project=${PROJECT_ID} \
    --location=us-central1-a \
    --tier=BASIC_HDD \
    --file-share=name=fs,capacity=1024 \
    --network=name="default"
```

Create an instance for uploading files
```bash
./fs/gcloud-create.sh
```

### Master

Create master instance on gcloud
```bash
./master/gcloud-create.sh
```

### Mappers

Create mappers MIG on gcloud
```bash
./mapper/gcloud-create.sh
```

### Clean up

```bash
gcloud compute instances delete master-vm
```

```bash
gcloud compute instance-groups managed delete mapper \
    --zone us-central1-a
```

```bash
gcloud compute instance-templates delete mapper-template
```

## Test client

Build the testing client.
```bash
docker build -t test-client:latest -f tests/Dockerfile .
```

Forward traffic 
```bash
gcloud compute ssh master-vm \
    --project ${PROJECT_ID} \
    --zone us-central1-a \
    -- -NL 50051:localhost:50051
```

## Compile packages using vcpkg
Install vcpkg in **DESIRED DIRECTORY**.
```bash
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
```

Install desired packages (for example google_cloud_cpp_compute):
```bash
./vcpkg/vcpkg install google-cloud-cpp[compute]
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