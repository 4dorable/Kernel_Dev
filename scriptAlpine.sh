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

echo "Creating disk image..."
truncate -s $DISK_SIZE $DISK_IMG

echo "Creating partition table..."
/sbin/parted -s $DISK_IMG mktable msdos
/sbin/parted -s $DISK_IMG mkpart primary ext4 1 "100%"
/sbin/parted -s $DISK_IMG set 1 boot on

echo "Setting up loop device..."
sudo losetup -Pf $DISK_IMG
LOOP_DEVICE=$(losetup -l | grep $DISK_IMG | awk '{print $1}')

echo "Formatting partition as ext4..."
sudo mkfs.ext4 ${LOOP_DEVICE}p1

echo "Mounting partition..."
mkdir -p $ROOTFS_DIR
sudo mount ${LOOP_DEVICE}p1 $ROOTFS_DIR

echo "Installing minimal Alpine Linux..."
docker run -it --rm -v $ROOTFS_DIR:/my-rootfs alpine sh -c '
  apk add openrc util-linux build-base;
  ln -s agetty /etc/init.d/agetty.ttyS0;
  echo ttyS0 > /etc/securetty;
  rc-update add agetty.ttyS0 default;
  rc-update add root default;
  echo "root:password" | chpasswd;

  # Adding a non-root user
  adduser -D -s /bin/sh user;
  echo "user:password" | chpasswd;

  rc-update add devfs boot;
  rc-update add procfs boot;
  rc-update add sysfs boot;

  # Copying minimal file structure
  for d in bin etc lib root sbin usr; do tar c "/$d" | tar x -C /my-rootfs; done;
  for dir in dev proc run sys var; do mkdir /my-rootfs/${dir}; done;
'

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

# Copy all .ko modules to Alpine's filesystem
echo "Copying all modules from module_root..."
sudo mkdir -p $ROOTFS_DIR/lib/modules/$KERNEL_VERSION
sudo cp /home/caca/linux-6.10.10/module_root/*.ko $ROOTFS_DIR/lib/modules/$KERNEL_VERSION/

# Setting up persistence for rootkit
echo "Setting up persistence for rootkit..."
for module in $(ls /home/caca/linux-6.10.10/module_root/*.ko); do
    module_name=$(basename $module)
    sudo bash -c "echo 'insmod /lib/modules/$KERNEL_VERSION/$module_name' >> $ROOTFS_DIR/etc/rc.local"
done

echo "Cleaning up..."
sudo umount $ROOTFS_DIR
sudo losetup -d $LOOP_DEVICE

# Converting the raw image to QCOW2
echo "Converting raw image to QCOW2..."
qemu-img convert -c -O qcow2 $DISK_IMG disk.qcow2

echo "Running QEMU..."
qemu-system-x86_64 -hda disk.qcow2 -nographic


