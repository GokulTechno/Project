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

int main(int argc, char *argv[]) 
{
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

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) 
	{
        perror("shmget");
        exit(1);
    	}

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) 
	{
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
 if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) 
	{
        perror("shmat");
        exit(1);
	}
s = shm;
printf("Debug-3");
task_bar_status[0]='^';
task_bar_status[1]='1'; //Tower
task_bar_status[2]='^';
task_bar_status[3]='1';	// GPS Status
task_bar_status[4]='^';
task_bar_status[5]='1';	//Network connectivity
task_bar_status[6]='^';
task_bar_status[7]='+'; //Charging Status
task_bar_status[8]='1'; //Battery Status
task_bar_status[9]='!';

printf("Debug-4");
	
	    for (c = 0; c < 10; c++)
		{
			*s++ = task_bar_status[c];
//			printf("%c",task_bar_status[c]);
		}

		*s = '\0';
		sleep(1);
	}
    fclose(fp);
    closelog ();
}
