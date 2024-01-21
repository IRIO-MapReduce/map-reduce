#!/bin/bash
set -e

gcloud compute instances delete master-vm \
    --zone=us-central1-a