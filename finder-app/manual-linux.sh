#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here

    echo "[assignment3_part2] Deep clean the kernel build tree: mrproper"
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    echo "[assignment3_part2] Configure for our virt arm dev board (QEMU sim): defconfig"
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    echo "[assignment3_part2] Fixing multiple definition of yylloc dtc-lexer.l"
    sed -i 's/YYLTYPE yylloc/extern YYLTYPE yylloc/' scripts/dtc/dtc-lexer.l
    # echo "[assignment3_part2] Fixing multiple definition of yylloc dtc-lexer.lex.c"
    # sed -i 's/YYLTYPE yylloc/\/\/YYLTYPE yylloc/' scripts/dtc/dtc-lexer.lex.c

    echo "[assignment3_part2] Build a kernel image for booting with QEM: all"
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    # echo "[assignment3_part2] Build kernel modules: modules"
    # make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    echo "[assignment3_part2] Build the devicetree: dtbs"
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/arm64/boot/Image "${OUTDIR}/"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

mkdir ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir usr/bin usr/lib usr/sbin
mkdir var/log


# https://busybox.net/FAQ.html
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} distclean
    make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
echo "[assignment3_part2] Make busybox"
make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
# https://busybox.net/downloads/BusyBox.html
echo "[assignment3_part2] Install busybox"
make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX=${OUTDIR}/rootfs install

echo "Library dependencies"

# TODO: Add library dependencies to rootfs
# The below doesn't work because additional lib files are needed.
# Need to figure out how to determine the dependencies to cover those too.
# echo "[assignment3_part2] Program interpreter grep:"
# ${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
# PI_NAMES=$(${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter" | cut -d':' -f2 | tr -d ' ]')
# cp -a ${SYSROOT}/${PI_NAMES} ${OUTDIR}/rootfs/lib

# echo "[assignment3_part2] Shared library grep:"
# ${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"
# SL_NAMES=$(${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"| cut -d':' -f2 | cut -d'[' -f 2 | tr -d ' ]')
# cd ${SYSROOT}/lib64
# cp -a ${SL_NAMES} ${OUTDIR}/rootfs/lib64

cd $OUTDIR/rootfs
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 lib

cp -a $SYSROOT/lib64/ld-2.31.so lib64
cp -a $SYSROOT/lib64/libc.so.6 lib64 ##
cp -a $SYSROOT/lib64/libc-2.31.so lib64
cp -a $SYSROOT/lib64/libm.so.6 lib64 ##
cp -a $SYSROOT/lib64/libm-2.31.so lib64
cp -a $SYSROOT/lib64/libresolv.so.2 lib64 ##
cp -a $SYSROOT/lib64/libresolv-2.31.so lib64

# TODO: Make device nodes
cd ${OUTDIR}/rootfs
echo "[assignment3_part2] Make null device"
sudo mknod -m 666 dev/null c 1 3
echo "[assignment3_part2] Make console device"
sudo mknod -m 666 dev/tty c 5 1

# cd ${OUTDIR}/linux-stable
# echo "[assignment3_part2] Install modules:"
# make -j8 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} INSTALL_MOD_PATH=${OUTDIR}/rootfs modules_install

# TODO: Clean and build the writer utility
echo "[assignment3_part2] Clean and build the writer utility"
cd ${FINDER_APP_DIR}
make -j8 CROSS_COMPILE=${CROSS_COMPILE} clean
make -j8 CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "[assignment3_part2] Copy finder files to /home directory"
cp writer ${OUTDIR}/rootfs/home
cp finder.sh ${OUTDIR}/rootfs/home
cp finder-test.sh ${OUTDIR}/rootfs/home
cp autorun-qemu.sh ${OUTDIR}/rootfs/home

mkdir -p ${OUTDIR}/rootfs/home/conf
cp conf/username.txt ${OUTDIR}/rootfs/home/conf/

# TODO: Chown the root directory
echo "[assignment3_part2] Chown root directory"
cd ${OUTDIR}/rootfs
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
echo "[assignment3_part2] Create initramfs.cpio.gz"
find . -print0 | cpio --null --create --verbose --format=newc | gzip --best > ${OUTDIR}/initramfs.cpio.gz
# echo "[assignment3_part2] Create initramfs.cpio.gz"
# gzip -f ${OUTDIR}/initramfs.cpio

echo "[assignment3_part2] DONE!!"
