#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <pthread.h>

#define SHMSZ     27

char buff[20];
char bat_value[5];
char charger_status1[2];
char charger_status2[2];
char charger_status3[2];
char task_bar_status[15];
char date_status[20];
char tower[5];
char nw_status[5];
char remote_task_bar;

unsigned int bat_NoOfBytes;
unsigned int bat_Temp=0;
unsigned int bat_level=0;
unsigned char bat_Dummy=0;

void *sm_write( void *ptr );
void *bat_stat( void *ptr );
void *chrg_stat( void *ptr );
void *tower_stat( void *ptr );
void *ping_stat( void *ptr );
void *rem_stat( void *ptr );

int i=0,bat_count=0,bat_status_check=0,present_level,level_changes=5,level_up,ntp=0;
char level;

int main(int argc, char *argv[]) {

char c;
key_t key;
char *shm, *s;
int shmid;
	
FILE *fp;
FILE *fp_bat;
FILE *fp_status1;
FILE *fp_status2;
FILE *fp_status3;
FILE *fp_tower;
FILE *fp_nw;
FILE *fp_remote;

key = 2345;

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    s = shm;


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
    
    pthread_t thread1, thread2, thread3, thread4, thread5, thread6;
    int  iret1, iret2, iret3, iret4, iret5, iret6 ;
    
    iret1 = pthread_create( &thread1, NULL, sm_write, NULL);
     if(iret1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }
     
     iret2 = pthread_create( &thread2, NULL, bat_stat, NULL);
     if(iret2)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
         exit(EXIT_FAILURE);
     }
     
     iret3 = pthread_create( &thread3, NULL, chrg_stat, NULL);
     if(iret3)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret3);
         exit(EXIT_FAILURE);
     }
     iret4 = pthread_create( &thread4, NULL, tower_stat, NULL);
     if(iret4)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret4);
         exit(EXIT_FAILURE);
     }
     iret5 = pthread_create( &thread5, NULL, ping_stat, NULL);
     if(iret5)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret5);
         exit(EXIT_FAILURE);
     }
     iret6 = pthread_create( &thread6, NULL, rem_stat, NULL);
     if(iret6)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret6);
         exit(EXIT_FAILURE);
     }

     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL);
     pthread_join( thread3, NULL);
     pthread_join( thread4, NULL);
     pthread_join( thread5, NULL);
     pthread_join( thread6, NULL);
}
void *sm_write(void *ptr)
{
while(1)
{
	int shmid;
	char *shm, *s;
	char c; 
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        	perror("shmat");
        	exit(1);
    	}
	s = shm;
	sleep(1);
	for (c = 0; c < 10; c++)
        {
        	*s++ = task_bar_status[c];
	//      printf("%c",task_bar_status[c]);
        }
	//      printf("\n");
      *s = '\0';
	sleep(1);
}
}

void *bat_stat(void *ptr)
{
	while(1)
	{       
		FILE *fp_bat; 	
		fp_bat = fopen("/sys/class/power_supply/NUC970Bat/present", "r");
//		fread(bat_value, 6, 1, fp_bat);
		fscanf(fp_bat,"%d",&bat_Temp);
		fclose(fp_bat);
		//printf("Battery Percentage=%d", bat_Temp);

		if(bat_Temp<100 && bat_Temp>=94)
		{
			level=5;
		}
		else if(bat_Temp<94 && bat_Temp>=90)
		{
			level=4;
		}
		else if(bat_Temp<90 && bat_Temp>=86)
		{
			level=3;
		}
		else if(bat_Temp<86 && bat_Temp>=82)
		{
			level=2;
		}
		else if(bat_Temp<82 && bat_Temp>=74)
		{	
			level=1;
		}
		else if(bat_Temp<74 && bat_Temp>=72)
		{
			level=0;
		}
		else if(bat_Temp<72)
		{	
			level=9;
		}

//printf("Battery Level:%d", level);
if(level==0)
{
task_bar_status[8]='0';
}
else if(level==1)
{
task_bar_status[8]='1';
}
else if(level==2)
{
task_bar_status[8]='2';
}
else if(level==3)
{
task_bar_status[8]='3';
}
else if(level==4)
{
task_bar_status[8]='4';
}
else if(level==5)
{
task_bar_status[8]='5';
}
else if(level==9)
{
task_bar_status[8]='7';
}
sleep(1);
}
}
//fp_status1 = fopen("/sys/class/gpio/gpio290/value", "r");


void *chrg_stat(void *ptr)
{
while(1)
{
FILE *fp_status1;
fp_status1 = fopen("/opt/daemon_files/chrg_stat", "r");
fread(charger_status1, 1, 1, fp_status1);
fclose(fp_status1);
/*
//fp_status2 = fopen("/sys/class/gpio/gpio292/value", "r");
fp_status2 = fopen("/opt/daemon_files/chrg_stat")
fread(charger_status2, 1, 1, fp_status2);
fclose(fp_status2);
*/
//if(charger_status1[0] == '0' && charger_status2[0] == '1')
if(charger_status1[0] == '1')
{
task_bar_status[7]='+';
present_level=5;
}
else if(charger_status1[0] == '0')
{
task_bar_status[7]='-';
}
else
{
}
sleep(1);
}
}

void *tower_stat(void *ptr)
{
while(1)
{
FILE *fp_tower;
fp_tower = fopen("/opt/daemon_files/tower_value", "r");
fread(tower, 1, 1, fp_tower);
fclose(fp_tower);

task_bar_status[1]=tower[0];

//printf("tower = %c\n",task_bar_status[1]);
//printf("tower = %c\n",tower[0]);
sleep(1);
}
}

void *ping_stat(void *ptr)
{
while(1)
{
FILE *fp_nw;
fp_nw = fopen("/opt/daemon_files/ping_status", "r");
fread(nw_status, 1, 1, fp_nw);
fclose(fp_nw);

task_bar_status[5]=nw_status[0];
sleep(1);
}
}

/*

if(task_bar_status[5]=='E' || task_bar_status[5]=='W' || task_bar_status[5]=='G')
{
	if(ntp==0)
	{
		system("sh /opt/daemon_files/ntp_new.sh &");
		ntp=1;
	}
}

fp = fopen("/opt/daemon_files/gps_status", "r");
fread(buff, 1, 1, fp);
fclose(fp);

*/
void *rem_stat(void *ptr)
{
while(1)
{
task_bar_status[0]='^';
task_bar_status[2]='~';
//task_bar_status[3]=buff[0];
task_bar_status[3]='0';
task_bar_status[4]='~';
task_bar_status[6]='~';
task_bar_status[9]='!';
//task_bar_status[1]='5';
//task_bar_status[5]='G';
//task_bar_status[7]='-';
//task_bar_status[8]='2';
sleep(1);
}
}


