#!/bin/bash

echo "Initalisation Reseaux pour communiquer entre Cible et Attaquante..."

sudo ip link add name virbr0 type bridge
sudo ip link set virbr0 up
sudo ip link set enp0s8 master virbr0
sudo ip addr add 192.168.100.1/24 dev virbr0

