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
#include <signal.h>

#define SHMSZ     27
#define MOUSEFILE "/dev/input/event1"
#ifndef EV_SYN
#define EV_SYN 0
#endif

FILE *bfile, *sfile, *shutfile;
FILE *bl, *vmf;

char task_bar_status[1];
pthread_t tid[2];
int a_stat=0,shutdata=0,backlight_status=0;

volatile sig_atomic_t thread_stat = 0;

void standby(int value)
{
    //    switch(value)
    //    {
    //    case 0:
    //        system("/opt/daemon_files/standby.sh start");
    //        break;
    //    case 1:
    //        system("/opt/daemon_files/standby.sh stop");
    //        break;
    //    }
}

void handle_alarm( int sig )
{
    //    printf("Alarm Called\n");
    standby(0);
    system("echo 500 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
    system("echo 3 > /proc/sys/vm/drop_caches");
    backlight_status=0;
}

/*********************************Pthread Job*********************************/

void* doSomeThing(void *arg)
{
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
        //        printf("\n First thread processing\n");
        int fd;
        struct input_event ie;

        if((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
            perror("opening device");
            //exit(EXIT_FAILURE);
        }
        else
        {
            while(read(fd, &ie, sizeof(struct input_event))) {
                //                printf("time %ld.%06ld\ttype %d\tcode %d\tvalue %d\n",
                //                       ie.time.tv_sec, ie.time.tv_usec, ie.type, ie.code, ie.value);
                if(ie.code==330)
                {
                    int bdata;
                    bfile = fopen("/usr/share/status/backlight_read_time","r");
                    if(bfile)
                    {
                        fscanf(bfile,"%d",&bdata);
                        fclose(bfile);
                    }
                    //system("sh /usr/share/scripts/backlight 4");
                    system("echo 500 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
                    //system("sh /opt/daemon_files/standby.sh stop");
                    standby(1);
                    alarm(0);
                    alarm(bdata);
                }

            }
        }

    }

    return NULL;
}

/*********************************Pthread Job*********************************/

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

char standbydata[1];

int main(void)
{
    FILE *keymode,*keysymbol;
    int shmid,CAPS=0;
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

    signal( SIGALRM, handle_alarm );

    const char *dev = "/dev/input/event0";
    struct input_event ev;
    ssize_t n;
    int fd,countk=0;
    char num_stat[1],alp_stat[1];
    num_stat[0]='1';
    alp_stat[0]='1';
    task_bar_status[0]='3';
    *s++ = task_bar_status[0];
    *s = '\0';

    system("echo 90000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");

    int bdata;
    bfile = fopen("/usr/share/status/backlight_read_time","r");
    if(bfile)
    {
        fscanf(bfile,"%d",&bdata);
        fclose(bfile);
    }

    alarm(0);
    alarm(bdata);
    a_stat=1;

    //    int keymode_int;
    //    keymode = fopen("/proc/keypad/KEYPAD_mode","r");
    //    if(keymode)
    //    {
    //        fscanf(keymode,"%d",keymode_int);
    //        fclose(keymode);
    //    }

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }
    FILE *key_config;
    int kdata;
    key_config = fopen("/usr/share/status/KeyConfig","r");
    if(key_config)
    {
        fscanf(key_config,"%d",&kdata);
        fclose(key_config);
    }
    if(kdata==2)
    {
        while (1) {
            if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
                perror("shmat");
                exit(1);
            }
            s = shm;

            //task_bar_status[0]='1'; //Small
            //task_bar_status[0]='2'; //Caps
            //task_bar_status[0]='3'; //nums

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

            int bdata;
            bfile = fopen("/usr/share/status/backlight_read_time","r");
            if(bfile)
            {
                fscanf(bfile,"%d",&bdata);
                fclose(bfile);
            }

            int sfile;

            bfile = fopen("/opt/daemon_files/standby_status", "r");
            if(bfile)
            {
                fscanf(bfile,"%d",&sfile);
                fclose(bfile);
            }


            if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
            {
                alarm(0);
                alarm(bdata);
                //system("sh /usr/share/scripts/backlight 4");

                if(backlight_status==0)
                {
                    system("echo 90000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
                    backlight_status=1;
                }
                if(sfile==1)
                {
                    backlight_status=0;
                    system("echo 0 > /opt/daemon_files/standby_status");
                }

                standby(1);
                //                printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
                if((int)ev.code==58 && (int)ev.value==1)
                {
                    if(CAPS==0)
                    {
                        CAPS=1;
                    }
                    else if(CAPS==1)
                    {
                        CAPS=0;
                    }
                }
                if((int)ev.code==56 && (int)ev.value==1)
                {
                    if(countk==0)
                    {
                        //                        printf("Small set\n");
                        if(CAPS==0)
                        {
                            task_bar_status[0]=0x31;
                            countk=1;
                        }
                        else if(CAPS==1)
                        {
                            task_bar_status[0]=0x32;
                            countk=1;
                        }
                    }
                    else if(countk==1)
                    {
                        //                        printf("Num set\n");
                        task_bar_status[0]=0x33;
                        countk=0;
                    }
                }
                if((int)ev.code==64 && (int)ev.value==1)
                {
                    system("echo 1 > /usr/share/status/KEYPAD_symbol");
                }
            }
            *s++ = task_bar_status[0];
            *s = '\0';
            shmdt(shm);
            shmdt(s);
            usleep(500);
        }
    }
    else if(kdata==3)
    {
        /*****************************Thread Creation*******************/
        int err = pthread_create(&(tid[0]), NULL, &doSomeThing, NULL);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");

        pthread_join(tid[0]);

        /*****************************Thread Creation*******************/
        while (1) {

            if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
                perror("shmat");
                exit(1);
            }
            s = shm;

            //task_bar_status[0]='1'; //Small
            //task_bar_status[0]='2'; //Caps
            //task_bar_status[0]='3'; //nums

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

            int bdata;
            bfile = fopen("/usr/share/status/backlight_read_time","r");
            if(bfile)
            {
                fscanf(bfile,"%d",&bdata);
                fclose(bfile);
            }

            if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
            {
                alarm(0);
                alarm(bdata);
                /*Backlight Set*/
                if(backlight_status==0)
                {
                    system("echo 90000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle");
                    backlight_status=1;
                }
                standby(1);
                //                printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
                if((int)ev.code==56 && (int)ev.value==1)
                {

                    if(num_stat[0]=='1')
                    {
                        task_bar_status[0]=alp_stat[0];
                        num_stat[0]='0';
                    }
                    else if(num_stat[0]=='0')
                    {
                        task_bar_status[0]=0x33;
                        num_stat[0]='1';
                    }

                }
                else if((int)ev.code==58 && (int)ev.value==1)
                {
                    if(alp_stat[0]=='1')
                    {
                        task_bar_status[0]=0x32;
                        alp_stat[0]='2';
                    }
                    else if(alp_stat[0]=='2')
                    {
                        task_bar_status[0]=0x31;
                        alp_stat[0]='1';
                    }

                }
                if((int)ev.code==64 && (int)ev.value==1)
                {
                    system("echo 1 > /usr/share/status/KEYPAD_symbol");
                }
            }
            *s++ = task_bar_status[0];
            *s = '\0';
            shmdt(shm);
            shmdt(s);
        }
    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
}
