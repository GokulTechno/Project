#define  _GNU_SOURCE
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
#include <linux/poll.h>
#include <arpa/inet.h>


#ifndef IW_NAME
#define IW_NAME "wlan0"
#endif

char read_buffer[30],gsmcsq[8]="AT+CSQ\r",gsmops[10]="AT+COPS?\r",gsmpin[10]="AT+CPIN?\r",status_buff[10],bstatus_buff[10],bstat_buff[2], ping_buff[10], *name,read_buff[10];
int status_sim=0;
void ip_check(char *);
FILE *fping,*fpingg,*fpingng,*fgprspwr,*fgprsen,*fstatus,*fsim,*fm66,*fwifien,*fcsq, *fbluestat, *fblue, *fgpsd;
int fdt,count=0,level=0,pret;
FILE *ftower_value, *fip_address, *fping_status;

int wifi_up_down_status=0, enable_gprs_status=0, enable_wifi_status=0,ntp_status=0,ping_count=0;

int gprs_signal_check_thread_status=0,gprs_ping_thread_status=0,resol=0;
pthread_t tid[2];
int a=0;

#define BAUDRATE B9600
/* change this definition for the correct port */
#define MODEMDEVICE "/dev/mux1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

void signal_handler_IO (int status);   /* definition of signal handler */
int wait_flag=TRUE;                    /* TRUE while no signal received */

int mux_fd;
FILE *wifichk;
char wifistr[4];
char down[4]="down";
pid_t wlanconfig,wlanscan,wlanup;

volatile sig_atomic_t thread_stat = 0;
volatile sig_atomic_t  t_stat=0;

void handle_alarm( int sig )
{
    //    printf("Alarm Called\n");
    thread_stat = 0;
    t_stat=1;
}

void signal_handler_IO(int status){
    wait_flag = FALSE;
}

static void wifi_signal(void)
{
    int end, loop, line;
    char str[512],rssi[3];
    FILE *wifisig;
    FILE *fd = fopen("/proc/net/wireless", "r");
    if(fd)
    {
        line=3;
        for(end = loop = 0;loop<line;++loop){
            if(0==fgets(str, sizeof(str), fd)){//include '\n'
                end = 1;//can't input (EOF)
                break;
            }
        }
        if(!end)
            //            printf("%s\n",str);
            rssi[0]=str[21];
        rssi[1]=str[22];
        rssi[2]='\0';
        int sigw=atoi(rssi);
        //        printf("Wifi Data:%d\n",sigw);
        if(wifisig)
        {
            if(sigw<=20)
            {
                system("echo 5 > /opt/daemon_files/signal_level");
            }
            else if(sigw>=20 && sigw<=40)
            {
                system("echo 4 > /opt/daemon_files/signal_level");
            }
            else if(sigw>=40 && sigw<=60)
            {
                system("echo 3 > /opt/daemon_files/signal_level");
            }
            else if(sigw>=60 && sigw<=80)
            {
                system("echo 2 > /opt/daemon_files/signal_level");
            }
            else
            {
                system("echo 1 > /opt/daemon_files/signal_level");
            }
        }
        fclose(fd);
    }
    sleep(1);
}

static void restart_pppd(void)
{
    //    printf("Restarting PPPD\n");
    system("killall pppd ");
    sleep(1);
    system("pppd call gprs");
    sleep(1);
}

static void enable_gprs(void)
{
    system("echo 1 > /sys/class/gpio/gpio42/value");
    system("echo 1 > /sys/class/gpio/gpio288/value");

    enable_gprs_status=1;
}

static void operator_check(void)
{
    t_stat=0;
    signal( SIGALRM, handle_alarm );

    int fd, res;
    struct termios oldtio,newtio;
    char buf[128];
    struct sigaction saio;

    volatile int STOP=FALSE;

    saio.sa_handler = signal_handler_IO;
    saio.sa_flags=0;
    saio.sa_restorer = NULL;
    sigaction(SIGIO, &saio, NULL);

    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(MODEMDEVICE); exit(-1); }

    fcntl(fd, F_SETFL, O_NONBLOCK);
    //    fcntl(fd, F_SETOWN, getpid());
    //    fcntl(fd, F_SETFL, FASYNC);

    tcgetattr(fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
    newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
    newtio.c_cc[VERASE]   = 0;     /* del */
    newtio.c_cc[VKILL]    = 0;     /* @ */
    newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
    newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
    newtio.c_cc[VSWTC]    = 0;     /* '\0' */
    newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
    newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
    newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
    newtio.c_cc[VEOL]     = 0;     /* '\0' */
    newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
    newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
    newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
    newtio.c_cc[VEOL2]    = 0;     /* '\0' */

    //    printf("Signal check \n");
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    /**************************Sim Detect********************************/
    if(status_sim==0)
    {
        write(fd, gsmpin, sizeof(gsmpin));

        while (STOP==FALSE) {
            if ( thread_stat == 0 ) {
                thread_stat=1;
                alarm(0);
                alarm(3);
            }
            if(t_stat==1){
                t_stat=0;
                STOP=TRUE;
            }
            usleep(50000);
            res = read(fd, buf, sizeof(buf));
            buf[res]=0;
            printf("Data1: %s\n",buf);
            if(buf[7]=='R' && buf[8]=='E' && buf[9]=='A' && buf[10]=='D')
            {
                printf("Sim Detected!!!\n");
                status_sim=1;
                STOP=TRUE;
            }
            else if(buf[5]=='E' && buf[6]=='R' && buf[7]=='R')
            {
                status_sim=0;
                STOP=TRUE;
            }
            memset(buf,0x00,sizeof(buf));
        }
    }

    /**************************Sim Detect********************************/
    if(status_sim==1)
    {
        STOP=FALSE;


        write(fd, gsmops, sizeof(gsmops));

        while (STOP==FALSE) {
            if ( thread_stat == 0 ) {
                thread_stat=1;
                alarm(0);
                alarm(3);
            }
            if(t_stat==1){
                t_stat=0;
                STOP=TRUE;
            }
            usleep(50000);
            res = read(fd, buf, sizeof(buf));
            buf[res]=0;
            printf("Data2: %s\n",buf);
            if(buf[0]=='+' && buf[1]=='C' && buf[2]=='O' && buf[3]=='P')
            {
                printf("COPS Received!!!\n");
                fdt = open("/opt/daemon_files/rough_files/current_operator", O_RDWR | O_NOCTTY );
                write(fdt,buf,sizeof(buf));
                close(fdt);
                STOP=TRUE;
            }
            memset(buf,0x00,sizeof(buf));
        }

        STOP=FALSE;
        write(fd,gsmcsq,sizeof(gsmcsq));

        while(STOP==FALSE)
        {
            if ( thread_stat == 0 ) {
                thread_stat=1;
                alarm(0);
                alarm(3);
            }
            if(t_stat==1)
            {
                t_stat=0;
                STOP=TRUE;
            }
            usleep(50000);
            //        if(wait_flag == FALSE)
            //        {
            res = read(fd, buf, sizeof(buf));
            buf[res]=0;
            printf("Data3: %s\n",buf);
            if(buf[0]=='+' && buf[1]=='C' && buf[2]=='S' && buf[3]=='Q')
            {
                printf("CSQ Received!!!\n");
                char sig[2];
                sig[0]=buf[6];
                sig[1]=buf[7];
                sig[2]='\0';
                FILE *fd_csq;
                fd_csq = fopen("/opt/daemon_files/rough_files/signal_level","w");
                if(fd_csq)
                {
                    fprintf(fd_csq,buf,8);
                    fclose(fd_csq);
                }
                int sig_int=atoi(sig);
                if(sig_int>5 && sig_int<=10)
                {
                    system("echo 6 > /opt/daemon_files/tower_value");
                }
                else if(sig_int>10 && sig_int<=15)
                {
                    system("echo 7 > /opt/daemon_files/tower_value");
                }
                else if(sig_int>15 && sig_int<=20)
                {
                    system("echo 8 > /opt/daemon_files/tower_value");
                }
                else if(sig_int>20 && sig_int<=25)
                {
                    system("echo 9 > /opt/daemon_files/tower_value");
                }
                else if(sig_int>25)
                {
                    system("echo 10 > /opt/daemon_files/tower_value");
                }
                STOP=TRUE;
            }
            memset(buf,0x00,sizeof(buf));
        }
    }
    else
    {
        system("echo 20 > /opt/daemon_files/tower_value");
        system("echo Null > /opt/daemon_files/ip_address");
        system("echo 9 > /opt/daemon_files/ping_status");
    }
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
}

static void disable_gprs(void)
{
    system("killall gsmMuxd ");

    system("echo 0 > /sys/class/gpio/gpio42/value");
    system("echo 0 > /sys/class/gpio/gpio288/value");
    system("echo 0 > /opt/daemon_files/tower_value");
    system("echo Null > /opt/daemon_files/ip_address");
    system("echo 9 > /opt/daemon_files/ping_status");
    status_sim=0;
    enable_gprs_status=0;
    ping_count=0;
}
static void enable_wifi(void)
{
    system("echo 1 > /sys/class/gpio/gpio164/value");
    system("sh /opt/daemon/wifi_driver enable &");

    enable_wifi_status=1;
}
static void disable_wifi(void)
{
    system("ifdown wlan0");
    system("echo Null > /opt/daemon_files/ip_address");
    system("echo 9 > /opt/daemon_files/ping_status");
    system("sh /opt/daemon/wifi_driver disable &");
    system("echo 0 > /sys/class/gpio/gpio164/value");

    enable_wifi_status=0;
    ping_count=0;
}

void wlan0_down(void)
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
    wifi_up_down_status=0;
    ping_count=0;
    close(sockfd);

}
static int ip_address(char* i_name)
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
    if(fp)
    {
        fprintf(fp,"%s",inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
        fclose(fp);
    }
    //    printf("IP Address is %s - %s\n" , i_name , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
    return 0;
}
/****************************************Ping Function***********************************/
#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto=NULL;
int cnt=1;

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int pingf(char *adress)
{
    const int val=255;
    int i, sd;
    struct packet pckt;
    struct sockaddr_in r_addr;
    int loop;
    struct hostent *hname;
    struct sockaddr_in addr_ping,*addr;

    pid = getpid();
    proto = getprotobyname("ICMP");
    hname = gethostbyname(adress);
    bzero(&addr_ping, sizeof(addr_ping));
    addr_ping.sin_family = hname->h_addrtype;
    addr_ping.sin_port = 0;
    addr_ping.sin_addr.s_addr = *(long*)hname->h_addr;

    addr = &addr_ping;

    sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
    if ( sd < 0 )
    {
        perror("socket");
        return 1;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        perror("Set TTL option");
        return 1;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
        perror("Request nonblocking I/O");
        return 1;
    }
    for (loop=0;loop < 10; loop++)
    {
        int len=sizeof(r_addr);

        if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 )
        {
            return 0;
        }

        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = pid;
        for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
            pckt.msg[i] = i+'0';
        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = cnt++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
            perror("sendto");
        usleep(500000);
    }
    close(sd);
    return 1;
}
/*
static void ping(int a)
{

    if(a==1)
    {
        if ((pingf("8.8.8.8") || pingf("208.67.222.222"))==0)
            //        if (pingf("8.8.8.8")==0)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","E");
                fclose(fping);
            }
            printf ("\n Exists");
            ping_count++;
//            if(ntp_status!=1)
//            {
//                system("sh /opt/daemon_files/ntp_new.sh &");
//                ntp_status=1;
//            }
        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","e");
                fclose(fping);
            }
            printf ("\n Not reachable ");
        }

    }
    if(a==2)
    {
        if ((pingf("8.8.8.8") || pingf("208.67.222.222"))==0)
            //        if (pingf("8.8.8.8")==0)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","W");
                fclose(fping);
            }
            printf ("\n Exists");
            ping_count++;
//            if(ntp_status!=1)
//            {
//                system("sh /opt/daemon_files/ntp_new.sh &");
//                ntp_status=1;
//            }

        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","w");
                fclose(fping);
            }
            printf ("\n Not reachable ");
        }

    }
    if(a==3)
    {

        if ((pingf("8.8.8.8") || pingf("208.67.222.222"))==0)
            //        if (pingf("8.8.8.8")==0)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","G");
                fclose(fping);
            }
//            printf ("\n Exists");
            ping_count++;
            count=0;
//            if(ntp_status!=1)
//            {
//                system("sh /opt/daemon_files/ntp_new.sh &");
//                ntp_status=1;
//            }
        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            if(fping)
            {
                fprintf(fping,"%s","g");
                fclose(fping);
            }
//            printf ("\n Not reachable ");
        }

    }
}
*/

static void ping(int a)
{
    switch (a) {
    case 1:
        if ( system("ping -c 3 8.8.8.8 -w 3 > /dev/null") == 0)
        {
            system("echo E > /opt/daemon_files/ping_status");
            printf("Pinging !!!\n");
            //            if(ntp_status!=1)
            //            {
            //                system("sh /opt/daemon_files/ntp_new.sh &");
            //                ntp_status=1;
            //            }
        }
        else
        {
            system("echo e > /opt/daemon_files/ping_status");
        }
        break;
    case 2:
        if ( system("ping -c 3 8.8.8.8 -w 3 > /dev/null") == 0)
        {
            system("echo W > /opt/daemon_files/ping_status");

            printf("Pinging !!!\n");
            //            if(ntp_status!=1)
            //            {
            //                system("sh /opt/daemon_files/ntp_new.sh &");
            //                ntp_status=1;
            //            }
        }
        else
        {
            system("echo w > /opt/daemon_files/ping_status");
        }
        break;
    case 3:
        if ( system("ping -c 3 8.8.8.8 -w 3 > /dev/null") == 0)
        {
            system("echo G > /opt/daemon_files/ping_status");
            ping_count=0;
            printf("Pinging !!!\n");
            //            if(ntp_status!=1)
            //            {
            //                system("sh /opt/daemon_files/ntp_new.sh &");
            //                ntp_status=1;
            //            }
        }
        else
        {
            ping_count++;
            if(ping_count>4)
            {
                system("echo g > /opt/daemon_files/ping_status");
                ping_count=0;
            }
            //            if(count == 2)
            //            {
            //                ping_value=0;
            //                break;
            //            }
            //            //printf("Not Pinging !!!\n");
            //            count++;
            //            if(count == 3)
            //                count=0;
            //            //printf ("\n Not reachable ");
        }
        break;
    }
}

void wlan0_up(void)
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
    wifi_up_down_status=1;
    close(sockfd);
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
int new_ping(char *ipaddr) {
    char *command = NULL;
    FILE *fp;
    int stat = 0;
    asprintf (&command, "%s %s -q 2>&1", "fping", ipaddr);
    fp = popen(command, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute fping command\n");
        free(command);
        return -1;
    }
    stat = pclose(fp);
    free(command);
    return WEXITSTATUS(stat);
}
//----------- Main --------------------------

int main()
{
    pid_t pid, sid;
    pid = fork();
    if (pid < 0) { exit(EXIT_FAILURE); }
    if (pid > 0) { exit(EXIT_SUCCESS); }
    umask(0);
    sid = setsid();
    if (sid < 0) { exit(EXIT_FAILURE); }
    if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while(1)
    {
        fstatus = fopen("/opt/daemon_files/nw_status", "r");
        if(fstatus)
        {
            fread(status_buff, 10, 1, fstatus);
            fclose(fstatus);
        }

        //Ethernet Enable Flow
        if((status_buff[1]=='1' && status_buff[3]=='1' && status_buff[5]=='2') || (status_buff[1]=='1' && status_buff[3]=='1' && status_buff[5]=='0') || (status_buff[1]=='1' && status_buff[3]=='0' && status_buff[5]=='0') || (status_buff[1]=='1' && status_buff[3]=='0' && status_buff[5]=='2'))
        {
            if(enable_wifi_status!=0)
            {
                disable_wifi();
            }
            if(enable_gprs_status!=0)
            {
                disable_gprs();
            }
        }

        //Wifi Enable Flow
        else if((status_buff[1]=='0' && status_buff[3]=='1' && status_buff[5]=='2') || (status_buff[1]=='0' && status_buff[3]=='1' && status_buff[5]=='0'))
        {
            if(enable_gprs_status!=0)
            {
                disable_gprs();
            }

            if(enable_wifi_status!=1)
            {
                enable_wifi();
            }

            wifichk = fopen("/sys/class/net/wlan0/operstate","r");
            if (wifichk){
                fscanf(wifichk,"%s",wifistr);
                fclose(wifichk);
            }else{
                //file doesn't exists or cannot be opened (es. you don't have access permission )
            }


            //            printf("WIFI Status=%s\n",wifistr);
            if(wifistr[0]=='d' && wifistr[3]=='n')
            {
                //                printf("Wifi up..........!!!");
                if(wifi_up_down_status=1)
                {
                    wlan0_up();
                }
            }
            else
            {
                //                printf("Got Wifi link\n");
                wifi_signal();
                ping(2);
                ip_address("wlan0");
            }

        }

        //GPRS Enable Flow
        else if(status_buff[1]=='0' && status_buff[3]=='0' && status_buff[5]=='2')
        {
            //            fsim = fopen("/sys/class/gpio/gpio169/value", "r");
            //            if(fsim)
            //            {
            //                fscanf(fsim,"%s",status_sim);
            //                fclose(fsim);
            //            }

            //            if(status_sim[0]=='0')
            //            {
            enable_gprs();
            pid_t pid = proc_find("gsmMuxd");
            //                printf("Pid GSMMUXD:%d\n",pid);
            if (pid == -1)
            {
                system("gsmMuxd -p /dev/ttyS3 -r -s /dev/mux /dev/ptmx /dev/ptmx");
                sleep(2);
            }
            else
            {
                sleep(1);
                operator_check();
                sleep(1);
                if(status_sim==1)
                {
                    pid_t pid = proc_find("pppd");
                    //                    printf("Pid PPPD:%d\n",pid);
                    if (pid == -1)
                    {
                        system("pppd call gprs");
                        sleep(2);
                    }
                    else
                    {
                        //                        printf("Pinging !!!\n");
                        ping(3);
                        ip_address("ppp0");
                    }
                }
            }
            //            }
            //            else
            //            {
            //                //                printf("Sim not found\n");
            //                system("echo 20 > /opt/daemon_files/tower_value");
            //                system("echo Null > /opt/daemon_files/ip_address");
            //                system("echo 9 > /opt/daemon_files/ping_status");
            //            }
        }
        else if(status_buff[1]=='0' && status_buff[3]=='0' && status_buff[5]=='3')
        {
            //            fsim = fopen("/sys/class/gpio/gpio169/value", "r");
            //            if(fsim)
            //            {
            //                fscanf(fsim,"%s",status_sim);
            //                fclose(fsim);
            //            }
            //            if(status_sim[0]=='0')
            //            {
            //                //GPRS 3G Enable
            //            }
            //            else
            //            {
            //                //                printf("Sim not found\n");
            //                system("echo 20 > /opt/daemon_files/tower_value");
            //                system("echo Null > /opt/daemon_files/ip_address");
            //                system("echo 9 > /opt/daemon_files/ping_status");
            //            }
        }
        else if(status_buff[1]=='0' && status_buff[3]=='0' && status_buff[5]=='0')
        {
            //            printf("Network OFF\n");
            ntp_status=0;
            if(wifi_up_down_status!=0)
            {
                wlan0_down();
            }
            if(enable_gprs_status!=0)
            {
                disable_gprs();
            }
            if(enable_wifi_status!=0)
            {
                disable_wifi();
            }
        }
        count++;
        //        printf("\n\n Count =%d \n\n",count);
        sleep(1);
    }//Ever loop ending
    return 0;
}
