#include <stdint.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#ifndef EV_SYN
#define EV_SYN 0
#endif
#define SHMSZ     27

unsigned int user_timing=0,value=0,backlight_dim=500;  // 500 --> 5 sec

FILE *fp;

main()
{
    int shmid;
    key_t key;
    char *shm, *s;

    key = 1111;

    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

        	fp = fopen("/opt/daemon_files/standby", "r");
		fscanf(fp,"%d",&user_timing);
		fclose(fp);

user_timing=user_timing*100;

printf("user_timing = %d\n",user_timing);
/*
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
*/
while(1)
{

value=0;

while (*shm != '^')
{
usleep(10000);
value++;

if(value==backlight_dim)
{
system("sh /opt/power_standby.sh 2");
}

if(value==user_timing)
{
system("sh /opt/power_standby.sh 1");
break;
}

if(value>60000)
{
value=backlight_dim+1;
}

}

if(value>500)
{
system("sh /opt/power_standby.sh 3");
}

	s = shm;
	*s++ = '0';
	*s = '\0';
}

    exit(0);
}

