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
#include <sys/wait.h> 	  /* for wait */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include "iwlib.h"
#include <memory.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/signal.h>
#include <string.h>
#include <termios.h> 
#include <dirent.h>

char status_buff[10],bstatus_buff[10],bstat_buff[2], ping_buff[10], *name, status_sim[2],read_buff[10];
void ip_check(char *);
FILE *fping,*fgprspwr,*fgprsen,*fstatus,*fsim,*fm66,*fwifien,*fcsq, *fbluestat, *fblue;
int count;

#define BAUDRATE B115200
static void restart_pppd(void)
{
	system("killall pppd > /dev/null");
	system("pppd call gprs");
}

static void enable_gprs(void)
{
		fgprspwr = fopen("/sys/class/gpio/gpio42/value", "w");		
		fwrite("1",1,2,fgprspwr);
		fclose(fgprspwr);
		//system("sleep 1");
		sleep(1);
		fgprsen = fopen("/sys/class/gpio/gpio288/value", "w");		
		fwrite("1",1,2,fgprsen);
		fclose(fgprsen);
		//system("sleep 1");
		sleep(1);
}
static void signal_check(void)
{

        struct termios oldtio,newtio;
        mux_fd[0].fd = open("/dev/mux1", O_RDWR | O_NOCTTY );
        tcgetattr(mux_fd[0].fd,&oldtio); /* save current serial port settings */
        bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR | ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = ICANON;
        tcflush(mux_fd[0].fd, TCIFLUSH);
        tcsetattr(mux_fd[0].fd,TCSANOW,&newtio);

        printf("Signal check \n");
        mux_fd[0].events = POLLIN | POLLRDNORM;
        int wrval1 = poll(mux_fd, 1, 1000);
        if (wrval1 && (mux_fd[0].revents & POLLRDNORM)) {
        write(mux_fd[0].fd,gsmcsq,sizeof(gsmcsq));
        }
        int state_read =1;
        while(state_read)
        {
                mux_fd[0].events = POLLIN | POLLRDNORM;
                int retval1 = poll(mux_fd, 1, 1000);
                if (retval1 && (mux_fd[0].revents & POLLRDNORM)) {
                read(mux_fd[0].fd, read_buffer, sizeof(read_buffer));
                }
//		printf("%s\n",read_buff);
                if(read_buffer[0]=='+' && read_buffer[1]=='C' && read_buffer[2]=='S' && read_buffer[3]=='Q')
                {
//                      printf("CSQ Received!!!\n");
                        char sig[2];
                        sig[0]=read_buffer[6];
                        sig[1]=read_buffer[7];
                        sig[2]='\0';
                        FILE *fd_csq;
                        fd_csq = fopen("/opt/daemon_files/rough_files/signal_level","w");
                        fprintf(fd_csq,sig,2);
                        fclose(fd_csq);
                        int sig_int=atoi(sig);
                  fdt = open("/opt/daemon_files/tower_value", O_RDWR | O_NOCTTY );
                   if(sig_int>5 && sig_int<=10)
                        {
                                write(fdt,"1",2);
                        }
                   else if(sig_int>10 && sig_int<=15)
                        {
                                write(fdt,"2",2);
                        }
                   else if(sig_int>15 && sig_int<=20)
                        {
                                write(fdt,"3",2);
                        }
                   else if(sig_int>20 && sig_int<=25)
                        {
                                write(fdt,"4",2);
                        }
                   else if(sig_int>25 && sig_int<=30)
                        {
                                write(fdt,"5",2);
                        }
                close(fdt);
                state_read=0;
                }
                memset(read_buffer,0,sizeof(read_buffer));
        }
close(mux_fd[0].fd);
}
static void disable_gprs(void)
{
/*
   		fgprspwr = fopen("/sys/class/gpio/gpio42/value", "w");		
		fwrite("0",1,2,fgprspwr);
		fclose(fgprspwr);
		fgprsen = fopen("/sys/class/gpio/gpio288/value", "w");		
		fwrite("0",1,2,fgprsen);
		fclose(fgprsen);
*/
		system("killall pppd");
		//system("sleep 1");
		sleep(1);
}
static void enable_wifi(void)
{
   		fwifien = fopen("/sys/class/gpio/gpio164/value", "w");		
		fwrite("1",1,2,fwifien);
		fclose(fwifien);
}
static void disable_wifi(void)
{
   		fwifien = fopen("/sys/class/gpio/gpio164/value", "w");		
		fwrite("0",1,2,fwifien);
		fclose(fwifien);
}
static void ip_address(char* i_name)
{
    int n;
    struct ifreq ifr;
    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , i_name , IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    //display result
    FILE *fp;
    fp = fopen ("/opt/daemon_files/ip_address","w");
    fprintf(fp,"%s",inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    fclose(fp);
//    printf("IP Address is %s - %s\n" , i_name , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
}
static void ping(int a)
{
    if(a==1)
	{
	int state =1;	
		while(state)
    		{	
    		if ( system("ping -c 2 8.8.8.8 -w 2 > /dev/null") == 0)
    		{
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","E");
			fclose(fping);
        		printf ("\n Exists");
        		state=0;
    		}
    		else
    		{
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","e");
			fclose(fping);
       			printf ("\n Not reachable ");
    		}
    		}
	}
	if(a==2)
	{
	int state =1;	
		while(state)
    		{	
    		if ( system("ping -c 2 8.8.8.8 -w 2 > /dev/null") == 0)
    		{
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","W");
			fclose(fping);
        		printf ("\n Exists");
        		state=0;
    		}
    		else
    		{
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","w");
			fclose(fping);
       			printf ("\n Not reachable ");
    		}
    		}
	}
	if(a==3)
	{
	int state =1;	
		while(state)
    		{	
    		if ( system("ping -c 2 8.8.8.8 -w 2 > /dev/null") == 0)
    		{
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","G");
			fclose(fping);
        		printf ("\n Exists");
        		state=0;
			count=0;
    		}
    		else
    		{
			count++;
			if(count == 3)
			{
			  restart_pppd();
			}
			fping = fopen("/opt/daemon_files/ping_status", "w");		
			fprintf(fping,"%s","g");
			fclose(fping);
       			printf ("\n Not reachable ");
			state=0;
    		}
    		}
	}
}

pid_t proc_find(const char* name) 
{
    DIR* dir;
    struct dirent* ent;
    char* endptr;
    char buf[512];

    if (!(dir = opendir("/proc"))) {
        perror("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {
        /* if endptr is not a null character, the directory is not
         * entirely numeric, so ignore it */
        long lpid = strtol(ent->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        /* try to open the cmdline file */
        snprintf(buf, sizeof(buf), "/proc/%ld/cmdline", lpid);
        FILE* fp = fopen(buf, "r");

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                /* check the first token in the file, the program name */
                char* first = strtok(buf, " ");
                if (!strcmp(first, name)) {
                    fclose(fp);
                    closedir(dir);
                    return (pid_t)lpid;
                }
            }
            fclose(fp);
        }

    }

    closedir(dir);
    return -1;
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

//Ever loop starting 
while(1)
{
	fstatus = fopen("/opt/daemon_files/nw_status", "r");
	fread(status_buff, 10, 1, fstatus);
	fclose(fstatus);
//Ethernet Enable Flow
if((status_buff[1]=='1' && status_buff[3]=='1' && status_buff[5]=='2') || (status_buff[1]=='1' && status_buff[3]=='1' && status_buff[5]=='0') || (status_buff[1]=='1' && status_buff[3]=='0' && status_buff[5]=='0') || (status_buff[1]=='1' && status_buff[3]=='0' && status_buff[5]=='2'))
	{
		disable_wifi();	
		disable_gprs();
	}
//Wifi Enable Flow
else if((status_buff[1]=='0' && status_buff[3]=='1' && status_buff[5]=='2') || (status_buff[1]=='0' && status_buff[3]=='1' && status_buff[5]=='0'))
	{
		disable_gprs();
		enable_wifi();
		system("ifup wlan0 > /dev/null");
		//system("sleep 2");
		sleep(1);
		system("iwlist wlan0 scan");
		ping(2);	
	}
//GPRS Enable Flow
else if(status_buff[1]=='0' && status_buff[3]=='0' && status_buff[5]=='2')
	{
		fsim = fopen("/sys/class/gpio/gpio169/value", "r");
		fscanf(fsim,"%s",status_sim);
		fclose(fsim);
		if(status_sim[0]=='0')
		{
			enable_gprs();
			pid_t pid = proc_find("gsmMuxd");
			printf("Pid GSMMUXD:%d\n",pid);
		        if (pid == -1) 
			{
                            system("gsmMuxd -b 115200 -p /dev/ttyS3 -r -s /dev/mux /dev/ptmx /dev/ptmx /dev/ptmx /dev/ptmx");
		        } 
			else 
			{
                                signal_check();
				pid_t pid = proc_find("pppd");
				printf("Pid PPPD:%d\n",pid);
			        if (pid == -1) 
				{
				 system("pppd call gprs");
				}
				else
				{
				 printf("Pinging !!!\n");
				 ping(3);
			 	 ip_address("ppp0");
				}
		        }	
		}
		else
		{
			printf("Sim not found\n");
			FILE *ftower_value;
			ftower_value = fopen("/opt/daemon_files/tower_value","w");
			fprintf(ftower_value,"%s"," ");fprintf(ftower_value,"%s","0");
			fclose(ftower_value);
			FILE *fip_address;
			fip_address = fopen("/opt/daemon_files/ip_address","w");
			fprintf(ftower_value,"%s"," ");
			fclose(fip_address);
			FILE *fping_status;
			fping_status = fopen("/opt/daemon_files/ping_status","w");
			fprintf(ftower_value,"%s"," ");
			fclose(fping_status);
		}
	}
else if(status_buff[1]=='0' && status_buff[3]=='0' && status_buff[5]=='0')
	{
		FILE *ftower_value;
		ftower_value = fopen("/opt/daemon_files/tower_value","w");
		fprintf(ftower_value,"%s"," ");
		fclose(ftower_value);
		FILE *fip_address;
		fip_address = fopen("/opt/daemon_files/ip_address","w");
		fprintf(ftower_value,"%s"," ");
		fclose(fip_address);
		FILE *fping_status;
		fping_status = fopen("/opt/daemon_files/ping_status","w");
		fprintf(ftower_value,"%s"," ");
		fclose(fping_status);
		disable_gprs();		
		//disable_wifi();
	}
//system("sleep 1");
sleep(1);
}//Ever loop ending
	return 0;
}
