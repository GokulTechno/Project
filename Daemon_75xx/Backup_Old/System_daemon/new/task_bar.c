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
#include <dirent.h>

#define SHMSZ     27


char buff[20];
char bat_value[5];
char charger_status1[2];
char charger_status2[2];
char charger_status3[2];
char task_bar_status[15];
char date_status[20];
char tower[10];
char nw_status[10];
char remote_task_bar;

unsigned int bat_NoOfBytes;
unsigned int bat_Temp=0;
unsigned int bat_level=0;
unsigned char bat_Dummy=0;

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

int i=0,bat_count=0,bat_status_check=0,present_level,level_changes=5,level_up,ntp=0;
char level;

int main() {

    char c;
    int shmid;
    key_t key;
    char *shm, *s;

    FILE *fp_bat;
    FILE *fp_status1;
    FILE *fp_tower;
    FILE *fp_nw;

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


    while(1)
    {
        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }
        s = shm;
        fp_bat = fopen("/sys/class/power_supply/NUC970Bat/present", "r");
        fscanf(fp_bat,"%d",&bat_Temp);
        fclose(fp_bat);
        printf("Battery Tmp:%d\n",bat_Temp);
        if(bat_Temp<100 && bat_Temp>=94)
        {
            level=5;
        }
        else if(bat_Temp<94 && bat_Temp>=89)
        {
            level=4;
        }
        else if(bat_Temp<89 && bat_Temp>=84)
        {
            level=3;
        }
        else if(bat_Temp<84 && bat_Temp>=79)
        {
            level=2;
        }
        else if(bat_Temp<79 && bat_Temp>=74)
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

        printf("Battery Level:%d\n", level);
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

        fp_status1 = fopen("/sys/class/gpio/gpio110/value", "r");
        fread(charger_status1, 1, 1, fp_status1);
        fclose(fp_status1);

        if(charger_status1[0] == '0')
        {
            task_bar_status[7]='+';
            present_level=5;
        }
        else if(charger_status1[0] == '1')
        {
            task_bar_status[7]='-';
        }

        fp_tower = fopen("/opt/daemon_files/tower_value", "r");
        fread(tower, 1, 1, fp_tower);
        fclose(fp_tower);

        task_bar_status[1]=tower[0];

        fp_nw = fopen("/opt/daemon_files/ping_status", "r");
        fread(nw_status, 1, 1, fp_nw);
        fclose(fp_nw);

        task_bar_status[5]=nw_status[0];

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

        for (c = 0; c < 10; c++)
        {
            *s++ = task_bar_status[c];
        }
        *s = '\0';
        sleep(3);
    }

}
