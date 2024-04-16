# 服务器列表
在项目中有些服务器是固定IP地址, 有些是动态的. 当停电检修之后要花很久时间去找自己的服务器, 尤其是不同项目组共用一个机房. tos=Table of Servers, 一个简单的小程序用来放在固定IP地址的服务器上, 他可以接受动态ip地址的服务器注册请求, 同时可以将所有注册的服务器显示在一个页面上

## 使用说明
1. 固定IP地址端
	`make`
	`sudo cp bin/tos /usr/bin`
	`sudo cp init.d.tos.sh /etc/init.d/tos`
	`sudo chmod +x /etc/init.d/tos`
	`sudo update-rc.d tos defaults`
	`sudo /etc/init.d/tos start`
2. 动态IP地址端
	`sudo cp crontable.tos.sh /usr/bin`
	`sudo chmod +x /usr/bin/crontab.tos.sh`
	`sudo vim /etc/crontab`
	添加新行
	`*  *    * * *   root    /usr/bin/crontab.tos.sh`
	`sudo crontab /etc/crontab`

