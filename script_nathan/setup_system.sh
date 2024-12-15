#!/bin/bash

# Définir le répertoire de travail comme celui où le script est lancé
BUILD_DIR=$(pwd)
KERNEL_DIR="$BUILD_DIR/linux-6.10.10"
INITRAMFS_DIR="$BUILD_DIR/initramfs"
BUSYBOX_DIR="$BUILD_DIR/busybox"
MY_INIT_LOOP_DIR="$BUILD_DIR/my_init_loop"
NUM_THREADS=4  # Nombre fixe de threads pour la compilation

echo "Le script est lancé depuis : $BUILD_DIR"

# Mettre à jour les paquets
echo "Mise à jour des paquets..."
sudo apt update && sudo apt upgrade -y

# Installer les dépendances nécessaires
echo "Installation des dépendances..."
sudo apt install -y build-essential make gcc libncurses-dev bison flex libssl-dev libelf-dev \
                qemu-system-x86 util-linux grub-pc-bin git docker.io wget

# Télécharger le noyau Linux si le répertoire n'existe pas
if [ ! -d "$KERNEL_DIR" ]; then
    echo "Téléchargement du noyau Linux 6.10.10..."
    wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.10.10.tar.xz -P "$BUILD_DIR"
    echo "Extraction du noyau Linux..."
    tar -xf "$BUILD_DIR/linux-6.10.10.tar.xz" -C "$BUILD_DIR"
else
    echo "Le noyau Linux est déjà présent dans $KERNEL_DIR"
fi

# Cloner BusyBox dans le dossier build si nécessaire
if [ ! -d "$BUSYBOX_DIR" ]; then
    echo "Clonage de BusyBox..."
    git clone git://git.busybox.net/busybox "$BUSYBOX_DIR"
else
    echo "Le répertoire BusyBox existe déjà dans $BUSYBOX_DIR."
fi

# Créer les répertoires de travail si nécessaire
echo "Création des répertoires si nécessaire..."
mkdir -p "$INITRAMFS_DIR"
mkdir -p "$MY_INIT_LOOP_DIR"

# Compiler BusyBox
if [ -d "$BUSYBOX_DIR" ]; then
    echo "Compilation de BusyBox..."
    cd "$BUSYBOX_DIR"
    make defconfig
    make -j"$NUM_THREADS"
    make install
else
    echo "Erreur : le répertoire BusyBox n'existe pas dans $BUSYBOX_DIR."
    exit 1
fi

# Copier les fichiers BusyBox dans initramfs
echo "Copie des fichiers BusyBox dans initramfs..."
cp -a "$BUSYBOX_DIR/_install/"* "$INITRAMFS_DIR"

# Créer le fichier main.c si manquant
if [ ! -f "$MY_INIT_LOOP_DIR/main.c" ]; then
    echo "Création du fichier main.c manquant dans my_init_loop..."
    cat > "$MY_INIT_LOOP_DIR/main.c" <<EOL
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("ECOLE 2600 MY INIT\\n");
    while (1) {
        pid_t pid = fork();
        int status = 0;
        if (pid) {
            waitpid(pid, &status, 0);
            printf("Respawn\\n");
        } else {
            char *tab[] = {"/usr/bin/setsid", "cttyhack", "sh", NULL};
            execv("/usr/bin/setsid", tab);
        }
    }
}
EOL
fi

# Compiler init_loop
echo "Compilation de init_loop..."
gcc "$MY_INIT_LOOP_DIR/main.c" -o "$INITRAMFS_DIR/init_loop"
if [ $? -eq 0 ]; then
    echo "init_loop compilé avec succès."
else
    echo "Erreur lors de la compilation de init_loop."
    exit 1
fi

# Créer le fichier init s'il est manquant
if [ ! -f "$INITRAMFS_DIR/init" ]; then
    echo "Création du fichier init dans initramfs..."
    cat > "$INITRAMFS_DIR/init" <<EOL
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
echo "Boot took \$(cut -d' ' -f1 /proc/uptime) seconds"
cat <<EOF
 ___________            .__           ________  _______________  _______
 \\_   _____/ ____  ____ |  |   ____   \\_____  \\/  _____/\\   _  \\ \\   _  \\
  |    __)__/ ___\\/  _ \\|  | _/ __ \\   /  ____/   __  \\ /  /_\\  \\/  /_\\  \\
  |        \\  \\__(  <_> )  |\\  ___/  /       \\  |__\\  \\\\  \\_/   \\  \\_/   \\
 /_______  /\\___  >____/|____/\\___  > \\_______ \\_____  / \\_____  /\\_____  /
         \\/     \\/                \\/          \\/     \\/        \\/       \\/
Welcome to "Ecole 2600 linux"
EOF
/init_loop
EOL
fi

# Rendre le fichier init exécutable
echo "Rendre le script init exécutable..."
chmod +x "$INITRAMFS_DIR/init"

# Copier les bibliothèques nécessaires dans initramfs
echo "Copie des bibliothèques dans initramfs..."
mkdir -p "$INITRAMFS_DIR/lib/x86_64-linux-gnu/"
mkdir -p "$INITRAMFS_DIR/lib64"
cp /lib/x86_64-linux-gnu/libc.so.6 "$INITRAMFS_DIR/lib/x86_64-linux-gnu/"
cp /lib/x86_64-linux-gnu/libm.so.6 "$INITRAMFS_DIR/lib/x86_64-linux-gnu/"
cp /lib/x86_64-linux-gnu/libresolv.so.2 "$INITRAMFS_DIR/lib/x86_64-linux-gnu/"
cp /lib64/ld-linux-x86-64.so.2 "$INITRAMFS_DIR/lib64"

# Modifier les permissions pour permettre l'accès sans sudo
echo "Modification des permissions des fichiers et dossiers..."
chmod -R u+rwX,go+rX "$INITRAMFS_DIR"
chmod -R u+rwX,go+rX "$MY_INIT_LOOP_DIR"
chmod -R u+rwX,go+rX "$BUSYBOX_DIR"
chmod -R u+rwX,go+rX "$KERNEL_DIR"

# Créer une image CPIO de l'initramfs
echo "Création de l'image CPIO de l'initramfs..."
cd "$INITRAMFS_DIR"
find . -print0 | cpio --null -ov --format=newc | gzip -9 > "$BUILD_DIR/initramfs.cpio.gz"

# Vérifier la présence du fichier Makefile dans le répertoire du noyau avant la compilation
cd "$KERNEL_DIR"
if [ ! -f Makefile ]; then
    echo "Erreur : le fichier Makefile est introuvable dans $KERNEL_DIR. Assurez-vous que le noyau a été extrait correctement."
    exit 1
fi

# Compiler le noyau Linux
echo "Compilation du noyau Linux..."
make defconfig
make -j"$NUM_THREADS"
if [ $? -eq 0 ]; then
    echo "Noyau compilé avec succès."
else
    echo "Erreur lors de la compilation du noyau."
    exit 1
fi

# Lancer QEMU avec le noyau compilé et initramfs
echo "Démarrage de QEMU avec le noyau et initramfs..."
qemu-system-x86_64 -kernel "$KERNEL_DIR/arch/x86/boot/bzImage" \
                   -initrd "$BUILD_DIR/initramfs.cpio.gz" \
                   -append "root=/dev/ram0 console=ttyS0" -nographic
