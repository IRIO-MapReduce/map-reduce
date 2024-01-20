#!/bin/bash
set -e

gcloud compute instance-groups managed delete worker \
    --zone us-central1-a

gcloud compute instance-templates delete worker-template