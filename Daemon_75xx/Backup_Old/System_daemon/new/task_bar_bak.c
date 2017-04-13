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



int i=0,bat_count=0,bat_status_check=0,present_level,level_changes=5,level_up,ntp=0;
char level;

int main(int argc, char *argv[]) {

char c;
int shmid;
key_t key;
char *shm, *s;
	
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

bat_status_check=0;
present_level=5;

while(1)
{

//printf("testing\n");


 if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
s = shm;

		bat_Temp=0;

        	fp_bat = fopen("/sys/class/power_supply/NUC970Bat/present", "r");
//		fread(bat_value, 6, 1, fp_bat);
		fscanf(fp_bat,"%d",&bat_Temp);
		fclose(fp_bat);


bat_level=bat_level+bat_Temp;
//printf("bat_Temp = %d\n",bat_Temp);


	if(bat_count==5)
	{
//		bat_level=bat_level/5;
//		printf("bat_level = %d\n",bat_level);
		if(bat_level<100 && bat_level>=90)
		{
			level=5;
		}
		else if(bat_level<90 && bat_level>=88)
		{
			level=4;
		}
		else if(bat_level<88 && bat_level>=82)
		{
			level=3;
		}
		else if(bat_level<82 && bat_level>=74)
		{
			level=2;
		}
		else if(bat_level<74 && bat_level>=72)
		{	
			level=1;
		}
		else if(bat_level<72)
		{	
			level=9;
		}

		if(level<present_level)
		{
		present_level=level;
		}

		bat_level=0;
		bat_count=0;
	}

bat_count++;


if(present_level==0)
{
task_bar_status[8]='0';
}
else if(present_level==1)
{
task_bar_status[8]='1';
}
else if(present_level==2)
{
task_bar_status[8]='2';
}
else if(present_level==3)
{
task_bar_status[8]='3';
}
else if(present_level==4)
{
task_bar_status[8]='4';
}
else if(present_level==5)
{
task_bar_status[8]='5';
}
else if(present_level==9)
{
task_bar_status[8]='7';
}

fp_status1 = fopen("/sys/class/gpio/gpio290/value", "r");
fread(charger_status1, 1, 1, fp_status1);
fclose(fp_status1);

fp_status2 = fopen("/sys/class/gpio/gpio292/value", "r");
fread(charger_status2, 1, 1, fp_status2);
fclose(fp_status2);

// Charging

// 290 = 0  && 292 = 1

//printf("charger_status1 = %c , charger_status12 = %c \n",charger_status1[0],charger_status2[0]);

if(charger_status1[0] == '0' && charger_status2[0] == '1')
{
task_bar_status[7]='+';
present_level=5;
}

// Discharging

// 290 = 0 && 292 = 0

else
{
task_bar_status[7]='-';
}


/*
//-- Fullbattery --

if(charger_status2[0] == '0' && charger_status3[0] == '1')
{
task_bar_status[7]='+';
task_bar_status[8]='9';

//remote_task_bar[0]='9';
}

// -- Invalid --
else if(charger_status1[0] == '0' && charger_status2[0] == '0' && charger_status3[0] == '0')
{
task_bar_status[7]='-';
task_bar_status[8]='9';

//remote_task_bar[0]='8';
}

// -- Charging --
else if(charger_status1[0] == '0' && charger_status2[0] == '1' && charger_status3[0] == '0')
{
task_bar_status[7]='+';
present_level=5;

//remote_task_bar[0]='7';
}

// -- ADC --
else if(charger_status1[0] == '1' && charger_status2[0] == '1' && charger_status3[0] == '1')
{
task_bar_status[7]='-';

}
*/

fp_tower = fopen("/opt/daemon_files/tower_value", "r");
fread(tower, 1, 1, fp_tower);
fclose(fp_tower);

task_bar_status[1]=tower[0];

printf("tower = %c\n",task_bar_status[1]);
printf("tower = %c\n",tower[0]);

fp_nw = fopen("/opt/daemon_files/ping_status", "r");
fread(nw_status, 1, 1, fp_nw);
fclose(fp_nw);

task_bar_status[5]=nw_status[0];
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
task_bar_status[0]='^';
task_bar_status[2]='~';
//task_bar_status[3]=buff[0];
task_bar_status[3]='0';
task_bar_status[4]='~';
task_bar_status[6]='~';
task_bar_status[9]='!';
//task_bar_status[1]='5';
//task_bar_status[5]='G';
task_bar_status[7]='-';
//task_bar_status[8]='2';

    for (c = 0; c < 10; c++)
	{
        *s++ = task_bar_status[c];
//	printf("%c",task_bar_status[c]);
	}
//	printf("\n");
      *s = '\0';

	sleep(3);
    }

}
