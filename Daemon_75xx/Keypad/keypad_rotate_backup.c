#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define SHMSZ     27

char task_bar_status[1];

//volatile sig_atomic_t thread_stat = 0;
//volatile sig_atomic_t  t_stat=0;

//void handle_alarm( int sig )
//{
//    printf("Alarm Called\n");
//    system("sh /usr/share/scripts/backlight 0");
//    thread_stat = 0;
//    t_stat=1;
//}

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


static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

int main(void)
{
    int shmid;
    key_t key;
    char *shm, *s;

    key = 3333;

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    s = shm;
    pid_t child1;

//        pid_t pid, sid;
//        pid = fork();
//        if (pid < 0) { exit(EXIT_FAILURE); }
//        if (pid > 0) { exit(EXIT_SUCCESS); }
//        umask(0);
//        sid = setsid();
//        if (sid < 0) { exit(EXIT_FAILURE); }
//        if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
//        close(STDIN_FILENO);
//        close(STDOUT_FILENO);
//        close(STDERR_FILENO);

    const char *dev = "/dev/input/event0";
    struct input_event ev;
    ssize_t n;
    int fd,buzzer,count=0,countk=0;
    char num_stat[1],alp_stat[1];
    num_stat[0]='1';
    alp_stat[0]='1';
    task_bar_status[0]='3';
    *s++ = task_bar_status[0];
    *s = '\0';

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }
    while (1) {
        if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
            perror("shmat");
            exit(1);
        }
        s = shm;

        //task_bar_status[0]='1'; //Small
        //task_bar_status[0]='2'; //Caps
        //task_bar_status[0]='3'; //nums

        FILE *fp,*fp1;
        fp = fopen("/usr/share/status/KEYPAD_buzzer","r");
        fscanf(fp,"%d",&buzzer);
        fclose(fp);
        n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            else
                break;
        } else
            if (n != sizeof ev) {
                errno = EIO;
                break;
            }
        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
        {
            system("sh /usr/share/scripts/backlight 4");
            printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
            if(ev.value==1)
            {
                if(buzzer==1)
                {
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",1);
                    fclose(fp1);
                    usleep(50000);
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",0);
                    fclose(fp1);
                }
            }
            if((int)ev.code==56 && (int)ev.value==1)
            {
                countk++;
                if(countk==2)
                {
                    if(num_stat[0]=='1')
                    {
                        task_bar_status[0]=alp_stat[0];
                        num_stat[0]='0';
                    }
                    else if(num_stat[0]=='0')
                    {
                        task_bar_status[0]='3';
                        num_stat[0]='1';
                    }
                    countk=0;
                }
            }
            else if((int)ev.code==58 && (int)ev.value==1)
            {
                if(alp_stat[0]=='1')
                {
                    task_bar_status[0]='2';
                    alp_stat[0]='2';
                }
                else if(alp_stat[0]=='2')
                {
                    task_bar_status[0]='1';
                    alp_stat[0]='1';
                }

            }
            if((int)ev.value==2 && (int)ev.code==1)
            {
                count++;
                printf("Count: %d",count);
                if(count==20)
                {
                    printf("LongPress Event\n");
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",1);
                    fclose(fp1);
                    usleep(50000);
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",0);
                    fclose(fp1);
                    usleep(50000);
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",1);
                    fclose(fp1);
                    usleep(50000);
                    fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                    fprintf(fp1,"%d",0);
                    fclose(fp1);
                    pid_t pid = proc_find("shutdown");
                    if(pid==-1)
                    {
                        child1=fork();
                        if(child1==0)
                        {
                            system("export DISPLAY=:0.0;/usr/bin/shutdown;");
                        }
                    }
                    count=0;
                }
            }
            else
            {
                count=0;
            }
        }
        *s++ = task_bar_status[0];
        *s = '\0';
    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
}
