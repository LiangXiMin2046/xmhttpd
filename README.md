# 介绍
xmhttd是一个轻量级的Http服务器程序，是我学习网络编程的一个小作品，目前我的个人主页[liangximin.com](http://liangximin.com/)就部署在这个Http服务器上。我在编写过程中使用了常用的网络编程、操作系统编程知识，并且使用了一些c++11的特性。麻雀虽小五脏却俱全，在短短不到2000行代码里，这个HTTP服务器提供以下几个比较实用的功能：
1. 部署简易，类似Apache，服务器以守护进程的形式运行(运行在80端口)，使用者编译成功之后可以在docs文件夹下放置自己的html文件，以供浏览者使用浏览器或telnet访问。
2. 支持cgi动态文档技术，使用者可以在docs/cgi-bin/文件夹下放置python、perl、shell等脚本语言编写的文件以供调用，为浏览者提供动态服务。
3. 同时支持Epoll和Poll两种I/O复用模型，使用者可以根据情况选用。
4. 利用定时器与简化过的LRU机制，服务器可以自动断开长时间不发来消息的TCP连接以节省资源。

可以使用ab命令测试服务器性能
```
ab -c 500 -n 10000 -k 127.0.0.1:80/
```
在我单核，cpu频率为1Ghz,内存2g，操作系统为CentOs7.0的虚拟机上测试出的Queries Per Second大约为7800。
# 编译方法：
请事先安装好cmake，进入CMakeLists.txt所在文件夹。
```
sudo mkdir build
cd ./build
sudo cmake ..
sudo make
```
可执行文件将会输出到上一级目录，执行
```
./httpd
```
即可运行。
# 注意事项
1. 默认使用epoll实现多路复用，如果想使用poll，请添加自行环境变量“SIMPLEHTTP_USE_POLL”。
2. 测试index.html的cgi时请保证主机默认的python解释器为python3以上版本，否则请安装python3，将其设置为默认python解释器，或者修改hello.py的第一行为`#!/usr/bin/python3`
3. 请为脚本文件赋予可执行权限，`chmod +x 777 filename`
4. 我的编译环境为Centos 7.0，gcc版本4.8.5，其他环境目前尚未测试。
5. 目前只支持GET和POST方法。
# 致谢
感谢[@chenshuo](https://github.com/chenshuo)的书籍与他的[muduo库](https://github.com/chenshuo/muduo)为我在学习网络编程的过程中带来的帮助。
