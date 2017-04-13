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



int i=0,bat_count=0,bat_status_check=0,present_level,level_changes=5,level_up;
char level;

int main(int argc, char *argv[]) {
printf("Debug-1");
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
printf("Debug-2");
 if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }
s = shm;

fp_bat = fopen("/sys/class/power_supply/NUC970Bat/present", "r");
fread(bat_value, 6, 1, fp_bat);
fclose(fp_bat);

printk("Battery Value:%d",bat_value);

/*
	if(bat_status_check==2)
	{
        	fp_bat = fopen("/sys/bus/iio/devices/iio\:device0/in_voltage4_raw", "r");
		fread(bat_value, 6, 1, fp_bat);
		fclose(fp_bat);

		bat_Temp=0;
		bat_NoOfBytes=((bat_value[0]&0x0F)<<12);
		bat_NoOfBytes|=((bat_value[1]&0x0F)<<8);
		bat_NoOfBytes|=((bat_value[2]&0x0F)<<4);
		bat_NoOfBytes|=(bat_value[3]&0x0F);
		bat_Dummy=bat_NoOfBytes>>12&0x0F;
		bat_Temp=bat_Temp*0x0A+bat_Dummy;
		bat_Dummy=bat_NoOfBytes>>8&0x0F;
		bat_Temp=bat_Temp*0x0A+bat_Dummy;
		bat_Dummy=bat_NoOfBytes>>4&0x0F;
		bat_Temp=bat_Temp*0x0A+bat_Dummy;
		bat_Dummy=bat_NoOfBytes&0x0F;
		bat_Temp=bat_Temp*0x0A+bat_Dummy;
		bat_level=bat_level+bat_Temp;
		bat_count++;
		bat_status_check=0;
	}

	if(bat_count==5)
	{
		bat_level=bat_level/5;
//		printf("bat_level = %d\n",bat_level);
		if(bat_level>=3300)
		{
			level=4;
		}
		else if(bat_level<3300 && bat_level>=3200)
		{
			level=3;
		}
		else if(bat_level<3200 && bat_level>=3100)
		{
			level=2;
		}
		else if(bat_level<3100 && bat_level>=3000)
		{
			level=1;
		}
		else if(bat_level<3000 && bat_level>=2900)
		{
			level=0;
		}
		else if(bat_level<2900)
		{	
			level=0;
		}

		if(level<present_level)
		{
//		printf("present_level = %d\n",present_level);
		present_level=level;
		}
		else
		{
//		printf("present_level = %d\n",present_level);
		level=present_level;
		}
		bat_level=0;
		bat_count=0;
	}

//printf("level = %c\n",level);

bat_status_check++;

//task_bar_status[8]=level;

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

fp_status1 = fopen("/sys/class/gpio/gpio51/value", "r");
fread(charger_status1, 1, 1, fp_status1);
fclose(fp_status1);

fp_status2 = fopen("/sys/class/gpio/gpio22/value", "r");
fread(charger_status2, 1, 1, fp_status2);
fclose(fp_status2);

fp_status3 = fopen("/sys/class/gpio/gpio50/value", "r");
fread(charger_status3, 1, 1, fp_status3);
fclose(fp_status3);

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

//remote_task_bar[0]=task_bar_status[8];
}

fp_tower = fopen("/home/root/tower_value", "r");
fread(tower, 1, 1, fp_tower);
fclose(fp_tower);

task_bar_status[1]=tower[0];
//remote_task_bar[1]=tower[0];

fp_nw = fopen("/home/root/ping_status", "r");
fread(nw_status, 1, 1, fp_nw);
fclose(fp_nw);

task_bar_status[5]=nw_status[0];
//remote_task_bar[2]=nw_status[0];

fp = fopen("/home/root/gps_status", "r");
fread(buff, 1, 1, fp);
fclose(fp);
*/

printf("Debug-3");
task_bar_status[0]='^';
task_bar_status[1]='1';
task_bar_status[2]='^';
task_bar_status[3]='1';
task_bar_status[4]='^';
task_bar_status[5]='1';
task_bar_status[6]='^';
task_bar_status[7]='+';
task_bar_status[8]='1';
task_bar_status[9]='!';

	sleep(1);
    }
    fclose(fp);
    closelog ();
}
