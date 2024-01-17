#!/bin/bash
set -e

gcloud compute instances create master-vm \
    --zone us-central1-a \
    --image-family=debian-12 \
    --image-project=debian-cloud \
    --metadata-from-file=startup-script=master/startup-script.sh
