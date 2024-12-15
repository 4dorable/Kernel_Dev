 #!/bin/bash

set -e

# Fonction pour vérifier si les commandes nécessaires sont installées
command_exists() {
  command -v "$1" >/dev/null 2>&1
}

# Liste des commandes requises
REQUIRED_COMMANDS="truncate parted losetup mkfs.ext4 mount docker grub-install qemu-system-x86_64"
for cmd in $REQUIRED_COMMANDS; do
  if ! command_exists $cmd; then
    echo "Erreur : La commande '$cmd' est requise mais n'est pas installée."
    exit 1
  fi
done

# Supprimer une ancienne image disque si elle existe
rm -f disk.img

# Compiler le noyau Linux
cd linux-6.10.10
make -j 16
cd ..

# Créer une image disque de 450MB
truncate -s 200M disk.img

# Créer une table de partition amorçable en mode BIOS
parted -s ./disk.img mktable msdos
parted -s ./disk.img mkpart primary ext4 1 "100%"
parted -s ./disk.img set 1 boot on

# Configurer un périphérique loop
LOOP_DEVICE=$(sudo losetup -P --show -f disk.img)
echo "Périphérique loop utilisé : $LOOP_DEVICE"
sleep 1

# Déterminer le nom de la partition
PARTITION="${LOOP_DEVICE}p1"
if [ ! -b "$PARTITION" ]; then
  PARTITION="${LOOP_DEVICE}1"
fi
echo "Partition utilisée : $PARTITION"

# Formater la partition en ext4
sudo mkfs.ext4 "$PARTITION"
mkdir -p /tmp/my-rootfs
sudo mount "$PARTITION" /tmp/my-rootfs

# Construire l'image Docker Alpine avec le module
docker build . -t my_alpine

# Copier les fichiers système requis dans le système de fichiers cible
docker run --rm -v /tmp/my-rootfs:/my-rootfs my_alpine /bin/sh -c \
    'mkdir -p /my-rootfs/{etc,bin,sbin,lib,usr,var,proc,sys,dev,run,run/openrc} && \
     cp -a /bin /etc /lib /sbin /usr /my-rootfs'

# Créer les répertoires nécessaires pour OpenRC et les systèmes pseudo
sudo mkdir -p /tmp/my-rootfs/run/openrc
sudo mkdir -p /tmp/my-rootfs/dev/pts
sudo mkdir -p /tmp/my-rootfs/dev/shm
sudo mkdir -p /tmp/my-rootfs/proc
sudo mkdir -p /tmp/my-rootfs/sys

# Vérifier que le noyau est bien compilé
if [ ! -f linux-6.10.10/arch/x86/boot/bzImage ]; then
  echo "Erreur : Le noyau bzImage n'a pas été trouvé."
  exit 1
fi

# Configurer le répertoire de démarrage et copier le noyau compilé
sudo mkdir -p /tmp/my-rootfs/boot/grub
sudo cp linux-6.10.10/arch/x86/boot/bzImage /tmp/my-rootfs/boot/vmlinuz

# Créer le fichier grub.cfg pour configurer le démarrage avec QEMU
cat << 'EOF' | sudo tee /tmp/my-rootfs/boot/grub/grub.cfg
serial
terminal_input serial
terminal_output serial
set root=(hd0,1)
menuentry "Linux2600" {
 linux /boot/vmlinuz root=/dev/sda1 rw console=ttyS0 init=/bin/sh
}
EOF

# Installer Grub pour le démarrage BIOS
sudo grub-install --target=i386-pc --boot-directory=/tmp/my-rootfs/boot --force $LOOP_DEVICE

# Démonter la partition et détacher le périphérique loop
sudo umount /tmp/my-rootfs
sudo losetup -d $LOOP_DEVICE

# Exécuter QEMU avec le noyau spécifié et en indiquant le format raw de l'image disque
share_folder="/tmp/qemu-share"
mkdir -p $share_folder

echo "Running QEMU..."
qemu-system-x86_64 \
    -kernel linux-6.10.10/arch/x86/boot/bzImage \
    -append "root=/dev/sda1 console=ttyS0 init=/bin/sh" \
    -drive file=disk.img,format=raw \
    -nographic \
    -netdev user,id=net0,hostfwd=tcp::5555-:12345 -device e1000,netdev=net0 \
    -virtfs local,path=$share_folder,mount_tag=host0,security_model=passthrough,id=foobar \
    -fsdev local,id=fsdev-root,path=/tmp/my-rootfs,security_model=mapped \
    -device virtio-9p-pci,fsdev=fsdev-root,mount_tag=rootfs

