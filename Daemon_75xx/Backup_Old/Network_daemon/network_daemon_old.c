#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/ioctl.h>    // SIOCGIFFLAGS
#include <errno.h>        // errno
#include <netinet/in.h>   // IPPROTO_IP
#include <net/if.h>       // IFF_*, ifreq
#include <arpa/inet.h>
#include <sys/wait.h> /* for wait */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include "iwlib.h"
#include <memory.h>

/* The name of the interface */
#ifndef IW_NAME
#define IW_NAME "wlan0"
#endif

#define SHMSZ     27
#define ERROR(fmt, ...) do { printf(fmt, __VA_ARGS__); return -1; } while(0)

FILE *fp;
FILE *fsim;
FILE *ftower;
FILE *flevel;
FILE *fwifi;
FILE *fcmux;

char buff[255];
char sim[2];
char tower[2];
int file;
int s,z;

int i=0,j=0,a,count,sim_status=0,level=0,ping_value,loop=0,wifi_value=0;
int eth_presence,wlan_presence,pppd_presence,ifup_count=0;
pid_t child1;
pid_t child2;
pid_t pppd;
pid_t cmuxt;
char status_buff[10],ping_value_old[10];


char WAITING[8] = "WAITING";
char *network_ip;

struct ifreq ifr;
struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
//const char * name = "eth0";

void eth0();
void gpio(int,char);
void eth0_up();
void eth0_down();
void eth0_ifup();
void eth0_ifdown();
void ip_check(char *ip);
void ping();

//------ Network interface check function -----------

int CheckLink(char *ifname) {
    int state = -1;
    int socId = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (socId < 0) ERROR("Socket failed. Errno = %d\n", errno);

    struct ifreq if_req;
    (void) strncpy(if_req.ifr_name, ifname, sizeof(if_req.ifr_name));
    int rv = ioctl(socId, SIOCGIFFLAGS, &if_req);
    close(socId);

    if ( rv == -1) ERROR("Ioctl failed. Errno = %d\n", errno);

    return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
}

//-------------signal strength---------------

void signal_strength()
{
    int sockfd;
    struct iw_statistics stats;
    struct iwreq req;
    memset(&stats, 0, sizeof(stats));
    memset(&req, 0, sizeof(req));
    sprintf(req.ifr_name, IW_NAME);
    req.u.data.pointer = &stats;
    req.u.data.length = sizeof(stats);
    #ifdef CLEAR_UPDATED
    req.u.data.flags = 1;
    #endif

/* Any old socket will do, and a datagram socket is pretty cheap */
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("Could not create simple datagram socket");
    exit(EXIT_FAILURE);
    }

    /* Perform the ioctl */
    if(ioctl(sockfd, SIOCGIWSTATS, &req) == -1) {
    perror("Error performing SIOCGIWSTATS");
    close(sockfd);
    exit(EXIT_FAILURE);
    }

    close(sockfd);

level=stats.qual.level;

FILE *fsignal_strength=fopen("/opt/daemon_files/signal_level","w");

if(level>100)
{
fwrite("5",1,2,fsignal_strength);
}
else if(level>90 && level<100)
{
fwrite("4",1,2,fsignal_strength);
}
else if(level>80 && level<90)
{
fwrite("3",1,2,fsignal_strength);
}
else if(level>70 && level<80)
{
fwrite("2",1,2,fsignal_strength);
}
else if(level>60 && level<70)
{
fwrite("1",1,2,fsignal_strength);
}
else if(level>50 && level<60)
{
fwrite("0",1,2,fsignal_strength);
}
else
{
fwrite("0",1,2,fsignal_strength);
}

fwrite("\n",1,2,fsignal_strength);
fclose(fsignal_strength);



}

//------------- ethernet up/down ------------

void eth0_up()
{

int sockfd;
struct ifreq ifr;

sockfd = socket(AF_INET, SOCK_DGRAM, 0);

if (sockfd < 0)
return;

memset(&ifr, 0, sizeof ifr);

strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

ifr.ifr_flags |= IFF_UP;   // up
ioctl(sockfd, SIOCSIFFLAGS, &ifr);

}

void eth0_down()
{

int sockfd;
struct ifreq ifr;

sockfd = socket(AF_INET, SOCK_DGRAM, 0);

if (sockfd < 0)
return;

memset(&ifr, 0, sizeof ifr);

strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);

ifr.ifr_flags |= ~IFF_UP;   // down
ioctl(sockfd, SIOCSIFFLAGS, &ifr);

}


void eth0_ifup()
{

child1=fork();

if(child1==0)
{
static char *argv_eth0_ifup[]={"ifup","eth0",NULL};
execv("/sbin/ifup",argv_eth0_ifup);
}

}

void eth0_ifdown()
{
child1=fork();

if(child1==0)
{
static char *argv_eth0_ifdown[]={"ifdown","eth0",NULL};
execv("/sbin/ifdown",argv_eth0_ifdown);
}

}


//------------- wifi up/down ------------

void wlan0_up()
{

int sockfd;
struct ifreq ifr;

sockfd = socket(AF_INET, SOCK_DGRAM, 0);

if (sockfd < 0)
return;

memset(&ifr, 0, sizeof ifr);

strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

ifr.ifr_flags |= IFF_UP;   // up
ioctl(sockfd, SIOCSIFFLAGS, &ifr);

}

void wlan0_down()
{

int sockfd;
struct ifreq ifr;

sockfd = socket(AF_INET, SOCK_DGRAM, 0);

if (sockfd < 0)
return;

memset(&ifr, 0, sizeof ifr);

strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

ifr.ifr_flags |= ~IFF_UP;   // down
ioctl(sockfd, SIOCSIFFLAGS, &ifr);

}

void wlan0_ifup()
{
child1=fork();

if(child1==0)
{
static char *argv_wlan0_ifup[]={"ifup","wlan0",NULL};
execv("/sbin/ifup",argv_wlan0_ifup);
}

}

void wlan0_ifdown()
{
child1=fork();

if(child1==0)
{
static char *argv_wlan0_ifdown[]={"ifdown","wlan0",NULL};
execv("/sbin/ifdown",argv_wlan0_ifdown);
}

}

//---------- IP check ----------------------

void ip_check(char *ip)
{
	int fd;
	struct ifreq ifr;
	char *name;	
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	//Type of address to retrieve - IPv4 IP address
	ifr.ifr_addr.sa_family = AF_INET;

	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name , ip , IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	//display result
//	printf("%s - %s\n" , ip , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );

name=inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);

FILE *fp=fopen("/opt/daemon_files/ip_address","w");
fwrite(name, 1 , 20, fp);
fclose(fp);

}


//----------------- Killall cmuxt ----------------------

void killall_cmuxt()
{
printf("killall cmux\n");

child1=fork();

if(child1==0)
{
static char *argv_cmuxt[]={"killall","gsmMuxd",NULL};
execv("/usr/bin/killall",argv_cmuxt);
}

}

//--------------- wifi power enable -------------------

void wifi_enable()
{

//gpio(46,'1');
gpio(166,'1');
usleep(200000);
gpio(166,'1');
sleep(4);

}

//--------------- wifi power disable -------------------

void wifi_disable()
{
gpio(166,'0');
}

//--------------- gprs power enable -------------------

void gprs_enable()
{
gpio(42,'1');
gpio(288,'1');
}

//--------------- gprs power disable -------------------

void gprs_disable()
{
gpio(42,'0');
}

//------------ GPIO ------------------------

void gpio(int pin_no,char gpiovalue)
{
char start[100]="/sys/class/gpio/gpio";
char end[100]="/value";
char gpio_pin[5];
char value[5];
sprintf(gpio_pin, "%d", pin_no);

strcat(start,gpio_pin);

strcat(start,end);

//printf("%s\n",start);

FILE *fgpio;

//printf("%c\n",gpiovalue);

fgpio=fopen(start, "w");
fwrite(&gpiovalue,1,1,fgpio);
fclose(fgpio);
//printf("gpio_fixed\n");

}

//---------- Ethernet ----------------------------

ethernet()
{
printf("ethernet2\n");

while(1)
{
eth_presence = CheckLink("eth0");

	if(eth_presence==0)
	{

		system("echo 9 > /opt/daemon_files/ping_status");

		if(ifup_count==0)
		{
			eth0_ifdown();
			sleep(2);
			ifup_count=1;
			eth0_up();
		}

		if(status_buff[3]=='1')      //------  wifi enable command -----------
		{
			if(ifup_count==0)
			{
				eth0_ifdown();
				sleep(2);
				ifup_count=1;
				eth0_up();
			}
			wifi();
			break;
		}

		else if(status_buff[5]=='1')      //------  gprs enable command -----------
		{
			if(ifup_count==0)
			{
				eth0_ifdown();
//				sleep(3);
				usleep(500000);
				ifup_count=1;
				eth0_up();
			}
			gprs();
			break;
		}
	}

	if(eth_presence==1)
	{

		if(ifup_count==1)
		{
			killall_cmuxt();
			sleep(1);
//			gpio(42,'0');
			gpio(166,'0');

			eth0_ifup();
			sleep(3);
			ifup_count=0;
		}
		ip_check("eth0");
		system("sh /opt/daemon_files/ping_test 192.168.0.121 E");
	}
}

}

//---------- Wifi ----------------------------

wifi()
{

wlan0_ifdown();
system("echo 9 > /opt/daemon_files/ping_status");
	eth0_up();
	sleep(1);
	eth0_down();

pppd_presence = CheckLink("ppp0");

if(pppd_presence==1)
{
	system("echo 9 > /opt/daemon_files/ping_status");
	killall_cmuxt();
	sleep(1);
	gpio(42,'0');
}

	wlan_presence = CheckLink("wlan0");

	if(wlan_presence!=1)
	{
		wifi_enable();
	}

	wlan0_ifup();
//	sleep(5);
	sleep(2);

if(status_buff[1]=='1')      //------  eth0 enable command -----------
{
	eth0_up();
//	sleep(2);
}

while(1)
{

eth_presence = CheckLink("eth0");

	if(eth_presence==1)
	{
		if(status_buff[1]=='1')      //------  eth0 enable command -----------
		{
			system("echo 9 > /opt/daemon_files/ping_status");
			wlan0_ifdown();
			sleep(1);
			ethernet();
			break;
		}
		else if(status_buff[5]=='1')      //------  gprs enable command -----------
		{
			system("echo 9 > /opt/daemon_files/ping_status");
			wlan0_ifdown();
			sleep(1);
			gprs();
			break;
		}
	}

	if(eth_presence==0)
	{
		wlan_presence = CheckLink("wlan0");
		if(wlan_presence==1)
		{
			ip_check("wlan0");
			signal_strength();
			system("sh /opt/daemon_files/ping_test 8.8.8.8 W");
		}
		else
		{
			ip_check("wlan0");
			system("echo 9 > /opt/daemon_files/ping_status");
		}
	}
}

}


//---------- GPRS ----------------------------


gprs()
{

	pppd_presence = CheckLink("ppp0");

	if(pppd_presence==1)
	{

		if(status_buff[1]=='1')      //------  eth0 enable command -----------
		{
			eth0_up();
			sleep(3);
		}
		else
		{
			sleep(5);
		}

		fsim=fopen("/sys/class/gpio/gpio110/value", "r");
		fread(sim,1,1,fsim);
		fclose(fsim);

		if(sim[0]=='0')
		{
			sim_status=1;
			system("sh /opt/daemon_files/tower_check");
		}

		else
		{
			sim_status=0;
		}

		if(sim_status==1)
		{
			sleep(3);
			while(1)
			{
				eth_presence = CheckLink("eth0");

				if(eth_presence==0)
				{
					system("sh /opt/daemon_files/tower_check");
					sleep(1);
				}
				else
				{
					if(status_buff[1]=='1')   //------  ethernet enable command ------
					{
						killall_cmuxt();
						ethernet();
						break;
					}
	
					else if(status_buff[3]=='1')   //------  wifi enable command------
					{
						killall_cmuxt();
						wifi();
						break;
					}
				}
				pppd_presence = CheckLink("ppp0");

				if(pppd_presence==1)
				{
					system("sh /opt/daemon_files/ping_test 8.8.8.8 G");
					ip_check("ppp0");
				}
				else
				{
					sleep(8);

					pppd_presence = CheckLink("ppp0");

					if(pppd_presence==1)
					{
						system("sh /opt/daemon_files/ping_test 8.8.8.8 G");
						ip_check("ppp0");
					}
					else
					{
						system("echo 9 > /opt/daemon_files/ping_status");
	//					killall_cmuxt();
//						sleep(1);					
						break;
					}
				}
			}
		}
		else
		{
		system("echo 0 > /opt/daemon_files/tower_value");
		}
	}

	else
	{

	printf("gprs2\n");

		killall_cmuxt();

		eth0_down();
		gprs_enable();

		pppd=fork();

		if(pppd==0)
		{
			cmuxt=fork();

			if(cmuxt==0)
			{
			sleep(1);
			static char *argv[]={"gsmMuxd", "-p", "/dev/ttyS3", "-r", "-s", "/dev/mux", "/dev/ptmx", "/dev/ptmx", "/dev/ptmx",NULL};
			execv("/usr/sbin/gsmMuxd",argv);
			}

			sleep(2);
			static char *argv1[]={"pppd", "call", "gprs",NULL};
			execv("/usr/sbin/pppd",argv1);

		}

		if(status_buff[1]=='1')      //------  eth0 enable command -----------
		{
			eth0_up();
			sleep(5);
		}
		else
		{
			sleep(7);
		}

		system("sh /opt/daemon_files/sim_check");

		fsim=fopen("/opt/daemon_files/sim_value", "r");
		fread(sim,1,1,fsim);
		fclose(fsim);

		if(sim[0]=='1')
		{
			sim_status=1;
			system("sh /opt/daemon_files/tower_check");
		}

		else
		{
			sim_status=0;
		}

		if(sim_status==1)
		{
			sleep(3);
			while(1)
			{
				eth_presence = CheckLink("eth0");

				if(eth_presence==0)
				{
					system("sh /opt/daemon_files/tower_check");
					sleep(1);
				}
				else
				{
					if(status_buff[1]=='1')   //------  ethernet enable command ------
					{
						killall_cmuxt();
						ethernet();
						break;
					}
	
					else if(status_buff[3]=='1')   //------  wifi enable command -----
					{
						killall_cmuxt();
						wifi();
						break;
					}
				}
				pppd_presence = CheckLink("ppp0");

				if(pppd_presence==1)
				{
					system("sh /opt/daemon_files/ping_test 8.8.8.8 G");
						}
				else
				{
					sleep(5);

					pppd_presence = CheckLink("ppp0");

					if(pppd_presence==1)
					{
						system("sh /opt/daemon_files/ping_test 8.8.8.8 G");
					}
					else
					{
						system("echo 9 > /opt/daemon_files/ping_status");
	//					killall_cmuxt();
						sleep(1);					
						break;
					}
				}
			}
		}
		else
		{
		system("echo 0 > /opt/daemon_files/tower_value");
		}

	}
}

//----------- Main --------------------------

int main(int argc, char *argv[]) 
{


    pid_t pid, sid;
 
   //Fork the Parent Process
    pid = fork();
 
    if (pid < 0) { exit(EXIT_FAILURE); }
 
    //We got a good pid, Close the Parent Process
    if (pid > 0) { exit(EXIT_SUCCESS); }
 
    //Change File Mask
    umask(0);
 
    //Create a new Signature Id for our child
    sid = setsid();
    if (sid < 0) { exit(EXIT_FAILURE); }
 
    //Change Directory
    //If we cant find the directory we exit with failure.
    if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
 
    //Close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);


	FILE *fstatus;
	
	fstatus = fopen("/opt/daemon_files/nw_status", "r");
	fread(status_buff, 10, 1, fstatus);
	fclose(fstatus);

	z=0;

while(1)
{	

if(status_buff[1]=='0')
{
system("echo 9 > /opt/daemon_files/ping_status");
if(z==0)
{
	eth0_down();
	z=1;
}
}

if(status_buff[3]=='0')
{
system("echo 9 > /opt/daemon_files/ping_status");
if(z==0 || z==1)
{
	z=2;
}
}

if(status_buff[5]=='0')
{
system("echo 9 > /opt/daemon_files/ping_status");
if(z==0 || z==1 || z==2)
{
	killall_cmuxt();
	z=3;
	sleep(1);
//	gpio(42,'0');
}
}


if(status_buff[1]=='1')      //------  eth0 enable command -----------
{
	ethernet();
}

else if(status_buff[3]=='1')      //------  wifi enable command -----------
{
	wifi();
}

else if(status_buff[5]=='1')       //------  gprs enable command -----------
{
	gprs();
}

sleep(1);

} 

fclose(fp);
    //Close the log
    closelog ();
}


