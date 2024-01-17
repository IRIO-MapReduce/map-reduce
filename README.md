# map-reduce

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
