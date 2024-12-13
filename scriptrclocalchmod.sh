#!/bin/bash
# Build your kernel before running this script

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_bzImage>"
    exit 1
fi

KERNEL_PATH="$1"
DISK_IMG="disk.img"
DISK_SIZE="450M"
ROOTFS_DIR="/tmp/my-rootfs"
LOOP_DEVICE=""
KERNEL_VERSION="6.10.10"  # Change this to your kernel version

# Create disk image
echo "Creating disk image..."
truncate -s $DISK_SIZE $DISK_IMG

# Create partition table
echo "Creating partition table..."
/sbin/parted -s $DISK_IMG mktable msdos
/sbin/parted -s $DISK_IMG mkpart primary ext4 1 "100%"
/sbin/parted -s $DISK_IMG set 1 boot on

# Setup loop device
echo "Setting up loop device..."
sudo losetup -Pf $DISK_IMG
LOOP_DEVICE=$(losetup -l | grep $DISK_IMG | awk '{print $1}')

# Format the partition
echo "Formatting partition as ext4..."
sudo mkfs.ext4 ${LOOP_DEVICE}p1

# Mount partition
echo "Mounting partition..."
mkdir -p $ROOTFS_DIR
sudo mount ${LOOP_DEVICE}p1 $ROOTFS_DIR

# Install minimal Alpine Linux
echo "Installing minimal Alpine Linux..."
docker run -it --rm -v $ROOTFS_DIR:/my-rootfs alpine sh -c '
  apk add openrc util-linux build-base sudo;
  ln -s agetty /etc/init.d/agetty.ttyS0;
  echo ttyS0 > /etc/securetty;
  rc-update add agetty.ttyS0 default;
  rc-update add root default;
  echo "root:password" | chpasswd;

  # Adding a non-root user with sudo access
  adduser -D -s /bin/sh user;
  echo "user:password" | chpasswd;
  addgroup user wheel;

  # Ensure sudoers configuration allows wheel group
  echo "%wheel ALL=(ALL) ALL" >> /etc/sudoers;

  rc-update add devfs boot;
  rc-update add procfs boot;
  rc-update add sysfs boot;

  # Configure networking
  echo "auto eth0
iface eth0 inet static
address 192.168.100.2
netmask 255.255.255.0
gateway 192.168.100.1
dns-nameservers 8.8.8.8" > /etc/network/interfaces;
  ln -s /etc/init.d/networking /etc/runlevels/default/networking;
  rc-update add networking default;

  # Copy minimal file structure
  for d in bin etc lib root sbin usr; do tar c "/$d" | tar x -C /my-rootfs; done;
  for dir in dev proc run sys var; do mkdir /my-rootfs/${dir}; done;
'

# Install GRUB and kernel
echo "Installing GRUB and Kernel..."
sudo mkdir -p $ROOTFS_DIR/boot/grub
sudo cp $KERNEL_PATH $ROOTFS_DIR/boot/vmlinuz

cat <<EOF | sudo tee $ROOTFS_DIR/boot/grub/grub.cfg
serial
terminal_input serial
terminal_output serial
set root=(hd0,1)
menuentry "Linux $KERNEL_VERSION" {
    linux /boot/vmlinuz root=/dev/sda1 console=ttyS0 noapic
}
EOF

sudo grub-install --directory=/usr/lib/grub/i386-pc --boot-directory=$ROOTFS_DIR/boot $LOOP_DEVICE

# Copy kernel modules
echo "Copying kernel modules..."
sudo mkdir -p $ROOTFS_DIR/lib/modules/$KERNEL_VERSION
sudo cp /home/caca/linux-6.10.10/module_root/*.ko $ROOTFS_DIR/lib/modules/$KERNEL_VERSION/

# Set up /etc/rc.local for module insertion
echo "Setting up /etc/rc.local for module persistence..."
sudo bash -c "echo '#!/bin/sh' > $ROOTFS_DIR/etc/rc.local"
sudo chmod +x $ROOTFS_DIR/etc/rc.local

for module in $(ls /home/caca/linux-6.10.10/module_root/*.ko); do
    module_name=$(basename $module)
    echo "Adding $module_name to rc.local..."
    sudo bash -c "echo 'insmod /lib/modules/$KERNEL_VERSION/$module_name' >> $ROOTFS_DIR/etc/rc.local"
done

sudo bash -c "echo 'exit 0' >> $ROOTFS_DIR/etc/rc.local"

# Finalize root filesystem
echo "Creating /dev/tmp directory in the root filesystem..."
sudo mkdir -p $ROOTFS_DIR/dev/tmp
sudo chmod 1777 $ROOTFS_DIR/dev/tmp

# Clean up
echo "Cleaning up..."
sudo umount $ROOTFS_DIR
sudo losetup -d $LOOP_DEVICE

# Convert raw image to QCOW2
echo "Converting raw image to QCOW2..."
qemu-img convert -c -O qcow2 $DISK_IMG disk.qcow2

# Run QEMU
echo "Running QEMU..."
qemu-system-x86_64 -hda disk.qcow2 -nographic -netdev bridge,id=net0,br=virbr0 -device virtio-net-pci,netdev=net0

