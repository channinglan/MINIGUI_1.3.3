#!/usr/bin


#make clean


export TMP_DIR=/home/sq/mnigui/1.3.3/arm_sample

export CROSS_DIR=/opt/crosstool/gcc-3.4.5-glibc-2.3.5/armv5te-softfloat-linux/
export NFS_DIR=/nfs/arm9_fs_k27src_3.4.5/
#export NFS_DIR=/nfs/arm9_fs_k27src_3.4.5/usr/local/
#export NFS_DIR=/nfs/arm9_fs_k27bsp_3.4.5/
#export NFS_DIR=/nfs/arm9_fs_1.11.2.19/
export CROSS_COMPILE=armv5te-softfloat-linux

#export CROSS_DIR=/opt/crosstool/arm-2008q3/arm-none-linux-gnueabi/
#export NFS_DIR=/nfs/arm9_fs_k27bsp_4.3.2/
#export CROSS_COMPILE=arm-none-linux-gnueabi

echo TMP_DIR=$TMP_DIR
echo CROSS_DIR=$CROSS_DIR
echo NFS_DIR=$NFS_DIR
echo CROSS_COMPILE=$CROSS_COMPILE


echo "configure"
CC=$CROSS_COMPILE-gcc ./configure \
--prefix=$TMP_DIR \
--host=$CROSS_COMPILE \
--target=$CROSS_COMPILE \
--build=i686-pc-linux-gnu  \
--with-osname=linux \
--with-style=classic  \
--with-targetname=fbcon \
--enable-autoial \
--enable-rbf16 \
--disable-vbfsupport \
--enable-smdk2410ial=yes \
--disable-ttfsupport

#--enable-jpgsupport \
#--enable-gifsupport \
#--enable-pngsupport \
#--enable-lite \
#--enable-standalone
#--enable-standalone \
#--disable-ttfsupport \
#--enable-debug > configure.log


echo "make"
make > make.log
echo "make install" #> make_install.log
make install

echo "cp to cross"
cp -r $TMP_DIR/lib  $CROSS_DIR
cp -r $TMP_DIR/include  $CROSS_DIR
ls /$TMP_DIR/lib

echo "cp to nfs"
cp -r $TMP_DIR/lib  $NFS_DIR
cp -r $TMP_DIR/include  $NFS_DIR
#cp -r $TMP_DIR/etc  $NFS_DIR



