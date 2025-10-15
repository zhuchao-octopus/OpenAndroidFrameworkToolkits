/////////////////////////////////////////////////////////////////////////////////////////////////////////
//How to compile

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake
make
/////////////////////////////////////////////////////////////////////////////////////////////////////////
How to start and control 

sudo systemctl daemon-reload
sudo systemctl start octopus_ipc_server
sudo systemctl status octopus_ipc_server
sudo journalctl --unit octopus_ipc_server --follow

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//How to upgrade on tina linux

adb push ./update.img /tmp/update.img		//将电脑中的update.img文件推送到设备中，注意路径的填写需要根据实际情况而定。
adb shell setprop sys.zkupgrade.flag 255	//置位升级标志
adb shell setprop sys.zkupgrade.dir /tmp	//指定升级文件路径，/tmp需修改为实际使用时升级文件推送到的路径
adb shell setprop ctl.restart zkswe			//重启应用程序，此时系统将会开始升级
