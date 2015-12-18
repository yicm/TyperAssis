# 基于QT的在线打字练习软件助手

## 简介

&emsp;&emsp;通过基于QT中QTcpServer和QTcpSocket以及UI编程，实现了基于TCP协议的C/S模型在线打字练习软件助手，服务端处理各客户端打字数据，以及显示在线打字客户列表即`实时`更新打字数据。客户端可实现离线打字练习以及在线打字练习，其中在线打字练习可以与在线用户比拼打字速度，并显示排名。

	服务端

![服务端UI界面](http://i.imgur.com/GUYOzKm.png)

	客户端登陆

![客户端登陆界面](http://i.imgur.com/Wzm22aX.png)

	离线打字主界面

![离线打字主界面](http://i.imgur.com/YtQkRGM.png)

	在线打字主界面

![在线打字主界面](http://i.imgur.com/S9NGvun.png)

## 特点

- 界面清爽，操作友好
- 能够在线与人拼打字速度
- 局域网内即可轻松实现运行
- 上线、下线稳定可靠

## 编译与运行

&emsp;&emsp;该打字软件助手是基于Qt Creator 3.4.2 (opensource) Based on Qt 5.5.0 (MSVC 2013, 32 bit)，在QT5.X.X版本以上是均能编译通过的。测试运行只在windows平台上测试过，包括xp,win7,win10，不过如果屏幕的分辨率太高会对打字界面有影响，推荐分辨率为1360x768。

