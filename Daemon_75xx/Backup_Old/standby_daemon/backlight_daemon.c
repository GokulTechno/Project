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

FILE *fp;
unsigned int user_timing=0;
int back0=0, back1=0, back2=0, back3=0, back4=0, back5=0;

main()
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
		fp = fopen("/usr/share/status/backlight", "r");
		fscanf(fp,"%d",&user_timing);
		fclose(fp);
		
	if(user_timing==0 && back0==0)
		{
			system("/usr/share/scripts/backlight 0");
                        printf("backlight-0\n");
			back0=1;
		}
	else if(user_timing==1 && back1==0)
		{
			system("/usr/share/scripts/backlight 1");
                        printf("backlight-1\n");
			back1=1;
		}
	else if(user_timing==2 && back2==0)
		{
			system("/usr/share/scripts/backlight 2");
                        printf("backlight-2\n");
			back2=1;
		}
	else if(user_timing==3 && back3==0)
                {
                        system("/usr/share/scripts/backlight 3");
                        printf("backlight-3\n");
			back3=1;
                }
	else if(user_timing==4 && back4==0)
                {
                        system("/usr/share/scripts/backlight 4");
                        printf("backlight-4\n");
			back4=1;	
                }
	else if(user_timing==5 && back5==0)
                {
                        system("/usr/share/scripts/backlight 5");
                        printf("backlight-5\n");
			back5=1;
                }
	}
	return 0;
}
