#include <netpacket/packet.h> 
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include "sys_posix.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys_debug.h"
#include "app_capture.h"

struct in_mac
{
	unsigned char buf[6];
};

struct netif_name
{
	char buf[16];
};

struct netif_info
{
	int ifindex;
	struct in_mac mac;
	struct netif_name name;
	struct in_addr ip;
	struct in_addr netmask;
	struct in_addr broadcast;
};

struct netif_manager
{
	int ifNum;
	int ifindex;
	unsigned int ip;
	unsigned int isRun;
	unsigned int isExit;
	TASK_ID handle;
	struct netif_info * pNetIfInfo;
};

struct recvinfo
{
	char srcip[32];
	char dstip[32];
	unsigned int srcport;
	unsigned int dstport;
	unsigned short id;
};

struct jt808hdr
{
	unsigned short id;
	unsigned short attribute;
	unsigned char phone[6];
	unsigned short seq;
};

static void app_capture_task(unsigned int ifindex);
static void ether_parse(unsigned char* buf, int len);
static void ppp0_parse(unsigned char* buf, int len);
static void ip_parse(unsigned char* buf, int len);
static void icmp_parse(unsigned char* buf, int len);
static void tcp_parse(unsigned char* buf, int len, struct recvinfo* pinfo);
static void jt808_parse(unsigned char* buf, int len, struct recvinfo* pinfo);

static struct netif_manager g_netManager = {0};

/*
状态迁移：
				未初始化：isExit = 0，isRun = 0
						↑	↓
	app_capture_uninit	↑	↓ app_capture_init
						↑	↓
				就绪：isExit = 1，isRun = 0
						↑	↓
	app_capture_stop	↑	↓ app_capture_start
						↑	↓
				抓包中：isExit = 1，isRun = 1
*/
/*
TODO:
	1. JT808的解析
	2. ppp0无以太帧，eth0有以太网帧。需要做自适应及添加伪头
	3. 通过注册多层IP、TCP、JT808过滤器函数进行过滤
	4. 结果保留到文件/串口输出可选
*/

int app_capture_init()
{
	if (g_netManager.isExit != 0 || g_netManager.isRun != 0)
	{
		APP_CORE_ERROR("network capture already initialized\n");
		return 0;
	}

	int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock < 0)
	{
		APP_CORE_ERROR("raw socket created error\n");
		return -1;
	}
	struct ifconf ifc;
	struct ifreq buf[16];
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	
	struct netif_info tempInfo;
	memset(&tempInfo, 0, sizeof(tempInfo));

	if (ioctl(sock, SIOCGIFCONF, (char *)&ifc))
	{
		APP_CORE_ERROR("ioctl socket error\n");
		close(sock);
		return -1;
	}

	g_netManager.ifNum = ifc.ifc_len / sizeof(struct ifreq);

	if (g_netManager.ifNum <= 0)
	{
		close(sock);
		return -1;
	}

	g_netManager.pNetIfInfo = (struct netif_info *)malloc(sizeof(struct netif_info) * g_netManager.ifNum);
	int i;
	for (i = 0; i < g_netManager.ifNum; i++)
	{
		strncpy(g_netManager.pNetIfInfo[i].name.buf, buf[i].ifr_name, sizeof(struct netif_name));

		if(ioctl(sock, SIOCGIFINDEX, (char *)(&buf[i])))
		{
			continue;
		}

		g_netManager.pNetIfInfo[i].ifindex = buf[i].ifr_ifindex;

		if (ioctl(sock, SIOCGIFHWADDR, (char *)(&buf[i])))
		{
			continue;
		}
		memcpy(g_netManager.pNetIfInfo[i].mac.buf, (unsigned char*)buf[i].ifr_hwaddr.sa_data, sizeof(struct in_mac));

		if (ioctl(sock, SIOCGIFADDR, (char *)&buf[i]))
		{
			continue;
		}
		g_netManager.pNetIfInfo[i].ip = ((struct sockaddr_in *)&(buf[i].ifr_addr))->sin_addr;

		if (ioctl(sock, SIOCGIFNETMASK, &buf[i]))
		{
			continue;
		}
		g_netManager.pNetIfInfo[i].netmask = ((struct sockaddr_in *)&(buf[i].ifr_netmask))->sin_addr;

		if (ioctl(sock, SIOCGIFBRDADDR, (char *)&buf[i]))
		{
			continue;
		}
		g_netManager.pNetIfInfo[i].broadcast = ((struct sockaddr_in *)&(buf[i].ifr_broadaddr))->sin_addr;
	}

	close(sock);
	g_netManager.isExit = 1;
	return 0;
}


int app_capture_uninit()
{
	if (g_netManager.isExit == 1 && g_netManager.isRun == 0)
	{
		if (g_netManager.pNetIfInfo != NULL)
		{
			free(g_netManager.pNetIfInfo);
		}
		memset(&g_netManager, 0, sizeof(struct netif_manager));
		return 0;	
	}
	else
	{
		return -1;
	}
}

int app_capture_start(char* netName, unsigned int ip)
{
	if(netName == NULL)
	{
		APP_CORE_ERROR("network card name err\n");
		return -1;
	}

	int ifindex = -1;
	int i;
	for(i = 0; i < g_netManager.ifNum; i++)
	{
		if (strncmp(g_netManager.pNetIfInfo[i].name.buf, netName, strlen(g_netManager.pNetIfInfo[i].name.buf)) == 0)
		{
			ifindex = g_netManager.pNetIfInfo[i].ifindex;
			break;
		}
	}

	if (ifindex == -1)
	{
		APP_CORE_ERROR("no Network card name : %s\n", netName);
		return -1;
	}
	
	if (g_netManager.isExit == 1 && g_netManager.isRun == 0)
	{
		g_netManager.isRun = 1;
	}
	else
	{
		APP_CORE_ERROR("network captrue start err\n");
		return -1;
	}
	
	g_netManager.ip = ip;
	g_netManager.ifindex = ifindex;
	
	if (sys_pthread_create(&(g_netManager.handle), "captureTask", TASK_PRIORITY_3, SIZE_128KB, (FUNCPTR)app_capture_task, 1, ifindex) != OK)
	{
		APP_CORE_ERROR("create capture Task error\n");
		return -1;
	}
	
	return 0;
}

static void app_task(unsigned int ifindex)
{
	char buf[50] = {0};
	int i = 0;
	while (1)
	{
		if (net_screen_connect_status_get())
		{
			i++;
			snprintf(buf, 50, "capture net screen message test:%d", i);
			app_jt808_screen_text_message_show(buf, FALSE);
		}

		sleep(5);
	}
	
}

int app_test_start()
{
	if (sys_pthread_create(&(g_netManager.handle), "captureTask", TASK_PRIORITY_3, SIZE_128KB, (FUNCPTR)app_task, 1, 1) != OK)
	{
		APP_CORE_ERROR("create capture Task error\n");
		return -1;
	}
}



void app_capture_stop()
{
	if (g_netManager.isExit == 0 && g_netManager.isRun == 0)
	{
		return;
	}

	g_netManager.isRun = 0;
	
	//等待线程退出
	while (g_netManager.isExit != 1)
	{
		sleep(1);
	}
}

static void app_capture_task(unsigned int ifindex)
{
	printf("\033[35m""%s:%d %s ""\033[0m\n",__FILE__, __LINE__, __FUNCTION__);
	//int index = g_netManager.ifindex;
	int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sock < 0)
	{
		g_netManager.isRun = 0;
		return ;
	}
	
	struct timeval stTimeOut = {3, 0};
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &stTimeOut, sizeof(stTimeOut)) < 0)
	{
		close(sock);
		APP_CORE_ERROR("set socket timeout error\n");
		g_netManager.isRun = 0;
		return;
	}

	struct sockaddr_ll addrLL = {0};
	addrLL.sll_family = PF_PACKET;
	addrLL.sll_ifindex = ifindex;
	addrLL.sll_protocol = htons(ETH_P_ALL);
	int ret = bind(sock, (struct sockaddr*)&addrLL, sizeof(addrLL));
	if (ret < 0)
	{
		close(sock);
		g_netManager.isRun = 0;
		APP_CORE_ERROR("socket bind error:%d %d\n", ret, errno);
		
		return;
	}

	g_netManager.isExit = 0;
	while (g_netManager.isRun)
	{
		unsigned char recvbuf[1518] = {0}; //以太网帧最大1518
		struct sockaddr_ll recvAddrLL;
		memset(&recvAddrLL, 0, sizeof(struct sockaddr_ll));
		socklen_t socklen = sizeof(recvAddrLL);
		
		int len = recvfrom(sock, recvbuf, 1518, 0, (struct sockaddr*)&recvAddrLL, &socklen);
		if(g_netManager.isRun == 0)
		{
			break;
		}

		if (len > 0)
		{
			ether_parse(recvbuf, 1518);
		}
		
	}
	close(sock);
	g_netManager.isExit = 1;
}

static char* mac_format(struct in_mac* pMac)
{
	static char _mac[18]; 
	snprintf(_mac, sizeof(_mac), "%02x%c%02x%c%02x%c%02x%c%02x%c%02x",
			pMac->buf[0],':',
			pMac->buf[1],':',
			pMac->buf[2],':',
			pMac->buf[3],':',
			pMac->buf[4],':',
			pMac->buf[5]
		);
	return _mac;
}
static void ether_parse(unsigned char* buf, int len)
{
	struct ethhdr* pEthHdr = (struct ethhdr*)buf;
#if 0	
	struct in_mac srcMac;
	struct in_mac dstMac;
	memcpy(&srcMac, pEthHdr->h_source, sizeof(struct in_mac));
	memcpy(&dstMac, pEthHdr->h_dest, sizeof(struct in_mac));
	printf("%s ", mac_format(&srcMac) );
	printf("-> %s ", mac_format(&dstMac));
	printf(": 0x%x\n", htons(pEthHdr->h_proto));
#endif
	switch (htons(pEthHdr->h_proto))
	{
	case ETH_P_IP:
		ip_parse(buf + sizeof(struct ethhdr), len - sizeof(struct ethhdr) - 4); //-4去掉以太网帧尾
		break;
	case ETH_P_ARP:
		break;
	default:break;
	}
}

static void ppp0_parse(unsigned char* buf, int len)
{
	ip_parse(buf, len);
}

static void ip_parse(unsigned char* buf, int len)
{
	struct iphdr* pIphdr = (struct iphdr*)buf;
	struct in_addr srcip;
	struct in_addr dstip;

	int ip_hlen = pIphdr->ihl << 2;
	int tot_len = ntohs(pIphdr->tot_len);
	srcip.s_addr = pIphdr->saddr;
	dstip.s_addr = pIphdr->daddr;

	//printf("%s -> ", (char*)inet_ntoa(srcip) );
	//printf("%s \n ", (char*)inet_ntoa(dstip) );
	
	switch(pIphdr->protocol)
	{
	case 1:

		icmp_parse(buf + ip_hlen, tot_len - ip_hlen);
		break;
	case 6:	//tcp
		{
			struct recvinfo info;
			memset(&info, 0, sizeof(struct recvinfo));
			memcpy(info.srcip, (char*)inet_ntoa(srcip), sizeof(info.srcip));
			memcpy(info.dstip, (char*)inet_ntoa(dstip), sizeof(info.srcip));

			int ipFilter = g_netManager.ip;

			if (srcip.s_addr == ipFilter || dstip.s_addr == ipFilter)
			{
				tcp_parse(buf + ip_hlen, tot_len - ip_hlen, &info);
			}
		}
		break;
	default:
		break;
	}
}

static void icmp_parse(unsigned char* buf, int len)
{
	struct icmphdr* pIcmphdr = (struct icmphdr*)buf;

	printf("icmp type:%d code:%d %s", pIcmphdr->type, pIcmphdr->code, pIcmphdr->data);
}

static void tcp_parse(unsigned char* buf, int len, struct recvinfo* pinfo)
{
	struct tcphdr* pTcphdr = (struct tcphdr*)buf;
	
	pinfo->srcport = (unsigned int)ntohs(pTcphdr->source);
	pinfo->dstport = (unsigned int)ntohs(pTcphdr->dest);

	if ((unsigned int)(pTcphdr->rst) == 1 && (unsigned int)(pTcphdr->ack) == 1)
	{
		APP_CMD_NOTE("%s:%d->%s:%d RST=1 ACK=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->rst) == 1)
	{
		APP_CMD_NOTE("%s:%d->%s:%d RST=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->syn) == 1 && (unsigned int)(pTcphdr->ack) == 1)
	{

		APP_CMD_NOTE("%s:%d->%s:%d SYN=1 ACK=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->syn) == 1)
	{
		APP_CMD_NOTE("%s:%d->%s:%d SYN=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->fin) == 1 && (unsigned int)(pTcphdr->ack) == 1)
	{
		APP_CMD_NOTE("%s:%d->%s:%d FIN=1 ACK=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->fin) == 1)
	{
		APP_CMD_NOTE("%s:%d->%s:%d FIN=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}

	if ((unsigned int)(pTcphdr->ack) == 1 && ((unsigned int)(pTcphdr->psh) != 1))
	{
		APP_CMD_NOTE("%s:%d->%s:%d ACK=1\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
		return ;
	}


	buf = buf + sizeof(struct tcphdr);

	size_t i = 0;
	fprintf(stderr, "\033[%dm", 35);
	fprintf(stderr, "%s:%d->%s:%d " , pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport);
	for (; i < len - sizeof(struct tcphdr) ; i++)
	{
		fprintf(stderr, "%02x" , buf[i]);
	}
	fprintf(stderr, "\033[0m\n");
	//if (buf[0] == 0x01 && buf[1] == 0x01 && buf[2] == 0x08) //ppp0 TCP帧都带有TCP Option timestamp (12B)
	//{
	//	buf = buf + 12;
	//	jt808_parse(buf, len - sizeof(struct tcphdr) - 12, pinfo);
	//}
	//else
	//{
	//	jt808_parse(buf, len - sizeof(struct tcphdr), pinfo);
	//}
	
}

static void jt808_parse(unsigned char* buf, int len, struct recvinfo* pinfo)
{
	if (len < 1 || buf[0] != 0x7e )
	{
		return;
	}
	
	struct jt808hdr* phdr = (struct jt808hdr*)(buf + 1);

	pinfo->id = ntohs(phdr->id);

	switch (pinfo->id)
	{
	case 0x0200:
		APP_CMD_NOTE("%s:%d->%s:%d msg_id=0x%04x\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport, pinfo->id);
		break;
	case 0x8001:
		APP_CMD_NOTE("%s:%d->%s:%d msg_id=0x%04x\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport, pinfo->id);
		break;
	case 0x1210:
	case 0x1211:
	case 0x9212:
	case 0x9208:
	case 0x0001:
	case 0x0002:
		APP_CMD_NOTE("%s:%d->%s:%d msg_id=0x%04x\n", pinfo->srcip, pinfo->srcport, pinfo->dstip, pinfo->dstport, pinfo->id);
		break;
	default:
		break;
	}
	
}
