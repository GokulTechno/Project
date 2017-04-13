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
#include <memory.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/signal.h>
#include <string.h>
#include <termios.h> 
#include <dirent.h>

FILE *fbluestat, *fblue, *fgprspwr, *fgprsen,*fpp, *fps,*fb;
#define BAUDRATE B115200
int fd,fdt,state_stat,state_ppp,k,con;
 char bton[13]="AT+QBTPWR=1\n",btoff[13]="AT+QBTPWR=0\n",btscan[15]="AT+QBTSCAN=1\n",btscanc[16]="AT+QBTSCANC\n",btstate[13]="AT+QBTSTATE\n",read_buffer[150],gsmcsq[8]="AT+CSQ\n";

static void signal_check(void)
{	
	int fd;
	struct termios oldtio,newtio;
	fd = open("/dev/mux1", O_RDWR | O_NOCTTY ); 
	tcgetattr(fd,&oldtio); /* save current serial port settings */
   	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
   	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   	newtio.c_iflag = IGNPAR | ICRNL;
   	newtio.c_oflag = 0;
   	newtio.c_lflag = ICANON;
    	tcflush(fd, TCIFLUSH);
    	tcsetattr(fd,TCSANOW,&newtio);	

	printf("Signal check \n");
	write(fd,gsmcsq,sizeof(gsmcsq));	
	int state_read =1;
	while(state_read)
	{
		read(fd, read_buffer, sizeof(read_buffer));
//		printf("%s\n",read_buff);
		if(read_buffer[0]=='+' && read_buffer[1]=='C' && read_buffer[2]=='S' && read_buffer[3]=='Q')
		{
//		printf("CSQ Received!!!\n");
			char sig[2];
			sig[0]=read_buffer[6];
			sig[1]=read_buffer[7];
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
close(fd);
}
void handle_alarm( int sig ) {
    state_ppp=0;
}
static void enable_gprs(void)
{
   	fgprspwr = fopen("/sys/class/gpio/gpio42/value", "w");		
	fwrite("1",1,2,fgprspwr);
	fclose(fgprspwr);
	fgprsen = fopen("/sys/class/gpio/gpio288/value", "w");		
	fwrite("1",1,2,fgprsen);
	fclose(fgprsen);
}

static void bt_scan(void)
{
	int fd;
	struct termios oldtio,newtio;
	fd = open("/dev/mux1", O_RDWR | O_NOCTTY ); 
	tcgetattr(fd,&oldtio); /* save current serial port settings */
   	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
   	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   	newtio.c_iflag = IGNPAR | ICRNL;
   	newtio.c_oflag = 0;
   	newtio.c_lflag = ICANON;
    	tcflush(fd, TCIFLUSH);
    	tcsetattr(fd,TCSANOW,&newtio);    

    fps = fopen("/opt/daemon_files/btscan", "w");
    fprintf(fps,"%s","");
    fclose(fps);

    state_stat=1;
    write(fd,btstate,sizeof(btstate));
    while(state_stat)
    {
	printf("Blue State Scan\n");
	read(fd,read_buffer,sizeof(read_buffer));
	if(read_buffer[0]=='+' && read_buffer[1]=='Q' && read_buffer[2] == 'B' && read_buffer[3]=='T' && read_buffer[4]=='S' && read_buffer[5]=='T')
	{
			fpp = fopen("/opt/daemon_files/btpaired", "a");
			fprintf(fpp,"%s",read_buffer);
			fclose(fpp);
	}
	else if(read_buffer[0]=='O' && read_buffer[1]=='K')
	{
		printf("BLUETOOTH STATE CHECK DONE\n");
		state_stat=0;
	}
	memset(read_buffer,0,sizeof(read_buffer));
    }
    fps = fopen("/opt/daemon_files/btscan", "w");
    fprintf(fps,"%s","");
    fclose(fps);

    state_ppp=1;
    write(fd,btscan,sizeof(btscan));
    alarm(5);
    while(state_ppp)
    {
	printf("Blue device Scan\n");	
	read(fd,read_buffer,sizeof(read_buffer));
	if(read_buffer[0]=='+' && read_buffer[1]=='Q' && read_buffer[2] == 'B' && read_buffer[3]=='T' && read_buffer[4]=='S' && read_buffer[5]=='C')
		{
			fps = fopen("/opt/daemon_files/btscan", "a");
			fprintf(fps,"%s",read_buffer);
			fclose(fps);
		}
	memset(read_buffer,0,sizeof(read_buffer));
    }
close(fd);
}

static void blue_en(void)
{
	printf("Test-1\n");
	int fd;
	struct termios oldtio,newtio;
	fd = open("/dev/mux2", O_RDWR | O_NOCTTY ); 
	tcgetattr(fd,&oldtio); /* save current serial port settings */
   	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
   	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   	newtio.c_iflag = IGNPAR | ICRNL;
   	newtio.c_oflag = 0;
   	newtio.c_lflag = ICANON;
    	tcflush(fd, TCIFLUSH);
    	tcsetattr(fd,TCSANOW,&newtio);	

	write(fd,bton,sizeof(bton));
	printf("Test-2\n");	
	int state_read =1;
	while(state_read)
	{
		printf("Test-3\n");
		read(fd, read_buffer, sizeof(read_buffer));
		printf("String:%s\n",read_buffer);
		if(read_buffer[0]=='O' && read_buffer[1]=='K')
		{
		printf("Test-4\n");		
		fb = fopen("/opt/daemon_files/bluestat", "w");
		fprintf(fb,"%s","B");
		fclose(fb);
		state_read=0;
		}
	memset(read_buffer,0,sizeof(read_buffer));
	}
close(fd);
}
static void blue_dis(void)
{	
	int fd;
	struct termios oldtio,newtio;
	fd = open("/dev/mux1", O_RDWR | O_NOCTTY ); 
	tcgetattr(fd,&oldtio); /* save current serial port settings */
   	bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
   	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
   	newtio.c_iflag = IGNPAR | ICRNL;
   	newtio.c_oflag = 0;
   	newtio.c_lflag = ICANON;
    	tcflush(fd, TCIFLUSH);
    	tcsetattr(fd,TCSANOW,&newtio);	

	write(fd,btoff,sizeof(btoff));	
	int state_read =1;
	while(state_read)
	{
		read(fd, read_buffer, sizeof(read_buffer));
		if(read_buffer[0]=='O' && read_buffer[1]=='K')
		{		
		fb = fopen("/opt/daemon_files/bluestat", "w");
		fprintf(fb,"%s","0");
		fclose(fb);
		state_read=0;
		}
		memset(read_buffer,0,sizeof(read_buffer));
	}
close(fd);
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

int main()
{

    fbluestat = fopen("/opt/daemon_files/bluestat", "w");
    fprintf(fbluestat,"%s","");
    fclose(fbluestat);

    fps = fopen("/opt/daemon_files/btscan", "w");
    fprintf(fps,"%s","");
    fclose(fps);

    fpp = fopen("/opt/daemon_files/btpaired", "w");
    fprintf(fpp,"%s","");
    fclose(fpp);

char bstatus_buff[10],bstat_buff[2];
/*
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
*/

//Bluetooth Enable Flow
while(1)
{
signal( SIGALRM, handle_alarm );
fblue = fopen("/opt/daemon_files/blueen","r");
fread(bstatus_buff,2,1,fblue);
fclose(fblue);
printf("While Started...\n");
	if(bstatus_buff[0]=='B' && bstatus_buff[1]=='1')
		{
			printf("Enabling Blue...\n");
			pid_t pid = proc_find("gsmMuxd");
			printf("test-1\n");
			if (pid == -1) 
			{
		           printf("Enabling gsmMuxd...\n");
			   enable_gprs();
			   system("gsmMuxd -b 115200 -p /dev/ttyS3 -r -s /dev/mux /dev/ptmx /dev/ptmx /dev/ptmx");
			   system("sleep 1");
		        } 
			else 
			{
				printf("test-2\n");
				signal_check();
				printf("Signal check completed\n");
		                fbluestat = fopen("/opt/daemon_files/bluestat","r");
				fread(bstat_buff,2,1,fbluestat);
				fclose(fbluestat);
				if(bstat_buff[0]=='0')
				{
				   printf("Blue Enable...\n");
				   blue_en();	
				}
				else
				{
				   printf("Blue Scanning...\n");
				   bt_scan();
				}
        		}
		}
		else if(bstatus_buff[0]=='B' && bstatus_buff[1]=='0')
		{
			fbluestat = fopen("/opt/daemon_files/bluestat","r");
			fread(bstat_buff,2,1,fbluestat);
			fclose(fbluestat);
			printf("Disabling blue...\n");
				pid_t pid = proc_find("gsmMuxd");
				printf("PID GSM:%d",pid);
				if(pid == -1)
				{
					
				}
				else
				{
					signal_check();
				}
				if(bstat_buff[0]=='B')
				{
					blue_dis();
				}
				else
				{
				        
				}
		}
	system("sleep 1");
}
return 0;
}
