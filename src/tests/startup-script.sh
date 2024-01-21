#!/bin/bash
set -e
sudo apt-get -y update
sudo apt-get -y install nfs-common
sudo mkdir -p /mnt/fs
sudo mount 10.2.214.34:/fs /mnt/fs
sudo chmod -R a+rwx /mnt/fs/
