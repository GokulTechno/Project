source /home/elinux1/nuvoton-cc
cd $PWD/Bluetooth/
arm-poky-linux-gnueabi-gcc -o blued blue_daemon.c
cp blued ../build/
cd ..
cd $PWD/Gpio/
arm-poky-linux-gnueabi-gcc -o gpiod gpio_daemon.c
cp gpiod ../build/
cd ..
cd $PWD/Network/
arm-poky-linux-gnueabi-gcc -o netd network_daemon.c -lpthread
cp netd ../build/
cd ..
cd $PWD/Gps/
arm-poky-linux-gnueabi-gcc -o gpsd gps_daemon.c
cp gpsd ../build/
cd ..
cd $PWD/Taskbar
arm-poky-linux-gnueabi-gcc -o taskd taskbar_daemon.c
cp taskd ../build/
cd ..
cd $PWD/Toolbar
arm-poky-linux-gnueabi-gcc -o toold toolbar.c
cp toold ../build/
cd ..
cd $PWD/Keypad
arm-poky-linux-gnueabi-gcc -o keyd keypad_rotate.c -lpthread
cp keyd ../build/
cd ..
cd $PWD/shutdown
make
cp shutdown ../build/shutd
cd ..
cp build/* ../7510_2.8_Inch/Filesystem/MATCHBOX_14112016/opt/daemon_files/
echo "Copied to 7510 Filesystem"
cp build/* ../7520_3.5_Inch/Filesystem/MATCHBOX_14112016/opt/daemon_files/
echo "Copied to 7520 Filesystem"
