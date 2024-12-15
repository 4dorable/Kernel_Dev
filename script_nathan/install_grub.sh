#!/bin/bash

# Vérifier les privilèges root
if [ "$EUID" -ne 0 ]; then
    echo "Ce script doit être exécuté avec des privilèges root (utilisez sudo)."
    exit
fi

# Mise à jour des paquets et installation de GRUB
echo "Mise à jour des paquets et installation de GRUB..."
sudo apt update
sudo apt install -y grub-pc

# Demande du disque sur lequel installer GRUB
echo "Sur quel disque voulez-vous installer GRUB ? (par exemple : /dev/sda)"
read -r DISK

# Installation de GRUB sur le disque spécifié
echo "Installation de GRUB sur $DISK..."
sudo grub-install "$DISK"

# Mettre à jour la configuration de GRUB
echo "Mise à jour de la configuration de GRUB..."
sudo update-grub

# Modifier le fichier de configuration GRUB pour afficher le menu au démarrage
echo "Configuration de GRUB pour afficher le menu au démarrage..."
sudo sed -i 's/^GRUB_TIMEOUT_STYLE=.*/GRUB_TIMEOUT_STYLE=menu/' /etc/default/grub
sudo sed -i 's/^GRUB_TIMEOUT=.*/GRUB_TIMEOUT=5/' /etc/default/grub

# Appliquer la configuration
echo "Application de la nouvelle configuration GRUB..."
sudo update-grub

echo "Installation et configuration de GRUB terminées. Votre machine affichera le menu GRUB au démarrage."
