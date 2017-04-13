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

#ifndef IW_NAME
#define IW_NAME "wlan0"
#endif

char read_buffer[30],gsmcsq[8]="AT+CSQ\n",gsmops[10]="AT+COPS?\n",status_buff[10],bstatus_buff[10],bstat_buff[2], ping_buff[10], *name, status_sim[2],read_buff[10];
void ip_check(char *);
FILE *fping,*fgprspwr,*fgprsen,*fstatus,*fsim,*fm66,*fwifien,*fcsq, *fbluestat, *fblue;
int fdt,count,level=0;

int mux_fd;
pid_t wlanconfig,wlanscan,wlanup;

volatile sig_atomic_t thread_stat = 0;
volatile sig_atomic_t  t_stat=0;

void handle_alarm( int sig )
{
    printf("Alarm Called\n");
    thread_stat = 0;
    t_stat=1;
}


#define BAUDRATE B115200
static void restart_pppd(void)
{
    system("killall pppd > /dev/null &");
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
static void operator_check(void)
{
    t_stat=0;
    signal( SIGALRM, handle_alarm );
    struct termios oldtio,newtio;
    mux_fd = open("/dev/mux1", O_RDWR | O_NOCTTY );
    tcgetattr(mux_fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(mux_fd, TCIFLUSH);
    tcsetattr(mux_fd,TCSANOW,&newtio);

    printf("Signal check \n");
    fcntl(mux_fd, F_SETFL, O_NONBLOCK);
    write(mux_fd,gsmops,sizeof(gsmops));

    while(1)
    {
        if ( thread_stat == 0 ) {
            thread_stat=1;
            alarm(4);
        }
        if(t_stat==1){
            t_stat=0;
            break;
        }
        count++;
        read(mux_fd, read_buffer, sizeof(read_buffer));
        if(read_buffer[0]=='+' && read_buffer[1]=='C' && read_buffer[2]=='O' && read_buffer[3]=='P')
        {
            //                printf("CSQ Received!!!\n");
            fdt = open("/opt/daemon_files/rough_files/current_operator", O_RDWR | O_NOCTTY );
            write(fdt,read_buffer,sizeof(read_buffer));
            close(fdt);
            break;
        }
        memset(read_buffer,0,sizeof(read_buffer));

    }
    close(mux_fd);
}
static void signal_check(void)
{
    signal( SIGALRM, handle_alarm );
    t_stat=0;
    struct termios oldtio,newtio;
    mux_fd = open("/dev/mux1", O_RDWR | O_NOCTTY );
    tcgetattr(mux_fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    tcflush(mux_fd, TCIFLUSH);
    tcsetattr(mux_fd,TCSANOW,&newtio);

    printf("Signal check \n");
    fcntl(mux_fd, F_SETFL, O_NONBLOCK);
    write(mux_fd,gsmcsq,sizeof(gsmcsq));

    while(1)
    {
        if ( thread_stat == 0 ) {
            thread_stat=1;
            alarm(4);
        }
        if(t_stat==1)
        {
            t_stat=0;
            break;
        }
        read(mux_fd, read_buffer, sizeof(read_buffer));
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
            fprintf(fd_csq,read_buffer,8);
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
            break;
        }
        memset(read_buffer,0,sizeof(read_buffer));

    }
    close(mux_fd);
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
    pid_t pid = proc_find("pppd");
    if(pid != -1)
    {
        system("killall pppd");
        //system("sleep 1");
        sleep(1);
    }
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

void wifi_signal_strength()
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

    FILE *fsignal_strength=fopen("/usr/share/status/daemon/signal_level","w");

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
    fprintf(fp,"%s",inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    fclose(fp);
    //    printf("IP Address is %s - %s\n" , i_name , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );
    return 0;
}
static void ping(int a)
{
    int status=0;
    switch(a)
    {
    case 1:
        status = pings("8.8.8.8");
        if (status)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","E");
            fclose(fping);
            printf ("\n Exists");

        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","e");
            fclose(fping);
            printf ("\n Not reachable ");

        }

        break;
    case 2:
        status = pings("8.8.8.8");
        if (status)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","W");
            fclose(fping);
            printf ("\n Exists");

        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","w");
            fclose(fping);
            printf ("\n Not reachable ");

        }
        break;
    case 3:
        status = pings("8.8.8.8");
        if (status)
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","G");
            fclose(fping);
            printf ("\n Exists");

        }
        else
        {
            fping = fopen("/opt/daemon_files/ping_status", "w");
            fprintf(fping,"%s","g");
            fclose(fping);
            printf ("\n Not reachable ");

        }
        break;
    }
}

int pings(char *ipaddr) {
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

            if(system("ifconfig | grep \"wlan0\"")!=0)
            {
                wlan0_up();
            }
            else
            {
                printf("Got Wifi link\n");
                ping(2);
                ip_address("wlan0");

            }

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
                    system("gsmMuxd -b 115200 -p /dev/ttyS3 -r -s /dev/mux /dev/ptmx /dev/ptmx /dev/ptmx");

                }
                else
                {
                    signal_check();
                    operator_check();
                    pid_t pid = proc_find("pppd");
                    printf("Pid PPPD:%d\n",pid);
                    if (pid == -1)
                    {
                        system("pppd call gprs");
                        sleep(1);
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
                fprintf(ftower_value,"%s","0");
                fclose(ftower_value);
                FILE *fip_address;
                fip_address = fopen("/opt/daemon_files/ip_address","w");
                fprintf(fip_address,"%s","0.0.0.0");
                fclose(fip_address);
                FILE *fping_status;
                fping_status = fopen("/opt/daemon_files/ping_status","w");
                fprintf(fping_status,"%s"," ");
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
            fprintf(fip_address,"%s","0.0.0.0");
            fclose(fip_address);
            FILE *fping_status;
            fping_status = fopen("/opt/daemon_files/ping_status","w");
            fprintf(fping_status,"%s"," ");
            fclose(fping_status);
            disable_gprs();
            disable_wifi();
            wlan0_down();
        }
        sleep(1);
    }//Ever loop ending
    return 0;
}
