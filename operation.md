win10安装vmware虚拟机 ，安装ubuntu24

设置网络：桥接模式，手动设置IP（和win的ip同网段）,掩码，网关dns，后两个和win的一样，我的win连接是WiFi，通过ipconfig /all 命令查看本地wifi网络信息。

vi 编辑器上下键乱码
sudo apt-get update
sudo apt-get remove vim-common
sudo apt-get install vim


安装ssh
修改ssh 配置可以root用户登录

win 安装winterm 就可以用root 登录虚拟机中的ubuntu系统了。

登录ubuntu,下载bochs：wget https://sourceforge.net/projects/bochs/files/bochs/2.6.11/bochs-2.6.11.tar.gz

configure:
./configure \
--prefix=/opt/bochs \
--enable-debugger \
--enable-disasm \
--enable-iodebug \
--enable-x86-debugger \
--with-x \
--with-x11 \



configure: error: no acceptable C compiler found in $PATH
安装gcc:sudo apt-get install build-essential

ERROR: X windows gui was selected, but X windows libraries were not found
sudo apt-get install libx11-dev xserver-xorg-dev xorg-dev
-------------------configure 完成---


make install

gtk_enh_dbg_osdep.cc:20:21: 致命错误： gtk/gtk.h：没有那个文件或目录:
sudo apt-get install libgtk2.0-dev
make dist-clean
重新configure
重新make install

修改配置文件

启动 bin/bochs
错误：bochsrc.disk:192: invalid choice 'core2_penryn_t9600' parameter 'model'

/opt/bochs/bin/bochs -help cpu
用列出的cpu model修改配置文件中对应的 cpu model

/opt/bochs/bin/bochs -help cpu

继续启动
bochsrc.disk:1095: Bochs is not compiled with gdbstub support
配置文件中注释掉gdb远程调试的配置gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0

继续启动
[GUI  ] Cannot connect to X display

没有在window配置服务来展示linux图形化界面：
win安装xming:
https://sourceforge.net/projects/xming/
不要安装putty，因为已经有了windterm
win开始菜单点击XLaunch默认一路点击下去，托盘出会出现xming的server.
修改ssh配置文件/etc/ssh/ssh_config，去掉一下注释
ForwardX11 yes
ForwardX11Trusted yes

重启ssh：systemctl restart ssh

winterm 连接时，选择x11相关配置：


继续启动bochs ，图形界面成功在win桌面弹出

报错：[HD   ] ata0-0: could not open hard drive image file '30M.sample'
配置文件中注释掉ata0-master: type=disk, mode=flat, path="30M.sample"

继续启动
黑屏，什么都没有
创建启动硬盘：bin/bximage -hd -mode="flat" -size=60 -q hd60M.img
配置文件刚注释的地方加上ata0-master: type=disk, path="hd60M.img", mode=flat, cylinders=121, heads=16, spt=63
继续启动，还是黑屏，在bochs命令行输入c
系统启动，提示No bootable device。
---bochs基本配置完成----------------
图1.4错误，应该是not read the boot disk

解决No bootable device
bios 程序写死在ROM只读存储器。0xFFFF0 是bios程序第一个指令的地址，开机时，cs:ip寄存器被强制写为0xF000：0xFFF0，
组合成0xFFFF0 。bios程序启动，进程硬件检查。
bios最后的动作校验0盘0道1扇区，最后两个字节是否是0x55和0xaa，是则把该扇区的mbr程序加载到物理地址0x7c00,bios跳到
该地址，由mbr接管后续动作。


编写mbr程序
下载nasm:apt-get install nasm

编译：nasm -o mbr.bin mbr.s 

dd if=/opt/mbr.bin of=/opt/bochs/hd60M.img bs=512 count=1  conv=notrunc 



cd /opt/bochs;/opt/bochs/bin/bochs -f /opt/bochs/bochsrc.disk
按换行
输入c
屏幕成功展示出字符串

编译loader:
nasm -I include/ -o /opt/bin/loader.bin loader.S
将loader写道磁盘2扇区：dd if=/opt/bin/loader.bin of=/opt/bochs/hd60M.img bs=512 count=1 seek=2 conv=notrunc

编译mbr:
nasm -I include/ -o /opt/bin/mbr.bin mbr2.s
将mbr写道磁盘0扇区：dd if=/opt/bin/mbr.bin of=/opt/bochs/hd60M.img bs=512 count=1 conv=notrunc
 





