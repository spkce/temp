
本实验将实现如下功能：本实验利用STM32F103战舰V3开发板上的DM9000网络芯片实现联网功能，网络协议栈使用的LWIP,版本为1.4.1。默认开启了DHCP，将开发板连接到路由器上以后就会自动获取IP地址，当DHCP失败以后就会使用默认IP地址，默认IP地址为：192.168.1.30。DHCP成功后就可以在电脑上通过ping命令来测试LWIP移植是否成功。

注意：本例程是不带UCOS的LWIP移植。


            	广州市星翼电子科技有限公司
                电话：020-38271790
                传真：020-36773971
	       	购买：http://shop62103354.taobao.com
                      http://shop62057469.taobao.com
                公司网站：www.alientek.com
         	技术论坛：www.openedv.com
