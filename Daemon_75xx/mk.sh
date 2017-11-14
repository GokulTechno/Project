source /opt/poky/1.4.3/environment-setup-armv5te-poky-linux-gnueabi
#cd $PWD/Bluetooth/
#arm-poky-linux-gnueabi-gcc -o blued blue_daemon.c
#cp blued ../build/
#cd ..
cd $PWD/Gpio/
arm-poky-linux-gnueabi-gcc -o gpiod gpio_daemon.c
cp gpiod ../build/
cd ..
cd $PWD/Network/
arm-poky-linux-gnueabi-gcc -o netd network_daemon.c -lpthread
cp netd ../build/
cd ..
cd $PWD/Gps/
make
cp gpsd ../build/
cd ..
cd $PWD/Taskbar
arm-poky-linux-gnueabi-gcc -o taskd taskbar_daemon.c
cp taskd ../build/
#cd ..
#cd $PWD/Toolbar
#arm-poky-linux-gnueabi-gcc -o toold toolbar.c
#cp toold ../build/
cd ..
cd $PWD/Keypad
arm-poky-linux-gnueabi-gcc -o keyd keypad_rotate.c -lpthread
cp keyd ../build/
cd ..
cd $PWD/rmgmtd
qmake
make
cp rmgmtd ../build
#cd ..
#cd $PWD/shutdown
#make
#cp shutdown ../build/shutd
cd ..
cd $PWD/Timezone
qmake
make
cp timezone ../../7510_2.8_Inch/Filesystem/MATCHBOX_14112016/usr/bin/
cp timezone ../../7520_3.5_Inch/Filesystem/MATCHBOX_14112016/usr/bin/
cd ..
chmod 777 build/*
cp build/* ../7510_2.8_Inch/Filesystem/MATCHBOX_14112016/opt/daemon_files/
cp build/* ../7510_2.8_Inch/LatestUpdate/Filesystem/MATCHBOX_14112016/opt/daemon_files/
echo "Copied to 7510 Filesystem"
cp build/* ../7520_3.5_Inch/Filesystem/MATCHBOX_14112016/opt/daemon_files/
cp build/* ../7520_3.5_Inch/LatestUpdateFilesystem/MATCHBOX_14112016/opt/daemon_files/

echo "Copied to 7520 Filesystem"
