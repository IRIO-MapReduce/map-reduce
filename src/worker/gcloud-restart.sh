#!/bin/bash
set -e

gcloud compute instance-groups managed rolling-action restart worker \
    --zone=us-central1-a