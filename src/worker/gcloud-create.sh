#!/bin/bash
set -e

gcloud compute instance-templates create worker-template \
    --machine-type=e2-standard-2 \
    --image-family=debian-12 \
    --image-project=debian-cloud \
    --scopes=compute-ro,logging-write \
    --metadata-from-file=startup-script=worker/startup-script.sh

gcloud compute instance-groups managed create worker \
    --size=3 \
    --zone us-central1-a \
    --template=worker-template

gcloud compute instance-groups managed set-autoscaling worker \
    --zone=us-central1-a \
    --max-num-replicas=10 \
    --min-num-replicas=2 \
    --scale-based-on-cpu \
    --target-cpu-utilization=0.6

