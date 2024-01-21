#!/bin/bash
set -e

gcloud compute instances create client-vm \
    --zone us-central1-a \
    --image-family=debian-12 \
    --image-project=debian-cloud \
    --scopes=compute-ro,logging-write \
    --metadata-from-file=startup-script=tests/startup-script.sh
