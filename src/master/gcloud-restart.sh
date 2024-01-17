#!/bin/bash
set -e

gcloud compute instances stop master-vm

gcloud compute instances start master-vm