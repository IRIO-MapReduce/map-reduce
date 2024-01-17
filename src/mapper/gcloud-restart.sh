#!/bin/bash
set -e

gcloud compute instance-groups managed rolling-action restart mapper \
    --zone=us-central1-a
