#!/bin/bash
set -e

gcloud compute instance-templates create mapper-template \
    --machine-type=e2-standard-2 \
    --image-family=debian-12 \
    --image-project=debian-cloud \
    --metadata-from-file=startup-script=mapper/startup-script.sh

gcloud compute instance-groups managed create mapper \
    --size 2 \
    --template mapper-template \
    --zone us-central1-a
