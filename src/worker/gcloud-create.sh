#!/bin/bash
set -e

gcloud compute instance-templates create worker-template \
    --machine-type=e2-standard-2 \
    --image-family=debian-12 \
    --image-project=debian-cloud \
    --scopes=compute-ro \
    --metadata-from-file=startup-script=worker/startup-script.sh

gcloud compute instance-groups managed create worker \
    --size=2 \
    --zone us-central1-a \
    --template=worker-template

