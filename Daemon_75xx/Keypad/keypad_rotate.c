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

FILE *bfile, *sfile;

char task_bar_status[1];
pthread_t tid[1];
int a_stat=0;

volatile sig_atomic_t thread_stat = 0;
void handle_alarm( int sig )
{
    printf("Alarm Called\n");
    system("sh /usr/share/scripts/backlight 0");
    //    system("var=`ps | grep '[X]org' | awk '{print $1}'`;kill -STOP $var");
}

/*********************************Pthread Job*********************************/

void* doSomeThing(void *arg)
{
    pthread_t id = pthread_self();

    if(pthread_equal(id,tid[0]))
    {
        printf("\n First thread processing\n");
        int fd;
        struct input_event ie;

        if((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
            perror("opening device");
            //exit(EXIT_FAILURE);
        }
        else
        {
            while(read(fd, &ie, sizeof(struct input_event))) {
                printf("time %ld.%06ld\ttype %d\tcode %d\tvalue %d\n",
                       ie.time.tv_sec, ie.time.tv_usec, ie.type, ie.code, ie.value);
                if(ie.code==330)
                {
                    int bdata;
                    bfile = fopen("/usr/share/status/backlight_read_time","r");
                    fscanf(bfile,"%d",&bdata);
                    fclose(bfile);
                    system("sh /usr/share/scripts/backlight 4");
                    //                    system("var=`ps | grep '[X]org' | awk '{print $1}'`;kill -CONT $var");
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
    int fd,buzzer,count=0,countk=0;
    char num_stat[1],alp_stat[1];
    num_stat[0]='1';
    alp_stat[0]='1';
    task_bar_status[0]='3';
    *s++ = task_bar_status[0];
    *s = '\0';

    int bdata;
    bfile = fopen("/usr/share/status/backlight_read_time","r");
    fscanf(bfile,"%d",&bdata);
    fclose(bfile);
    alarm(0);
    alarm(bdata);
    a_stat=1;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }
    FILE *key_config;
    int kdata;
    key_config = fopen("/usr/share/status/KeyConfig","r");
    fscanf(key_config,"%d",&kdata);
    fclose(key_config);
    if(kdata==2)
    {
        printf("2.8 Display\n");
        while (1) {
            FILE *fp,*fp1;

            if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
                perror("shmat");
                exit(1);
            }
            s = shm;

            //task_bar_status[0]='1'; //Small
            //task_bar_status[0]='2'; //Caps
            //task_bar_status[0]='3'; //nums


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

            int bdata;
            bfile = fopen("/usr/share/status/backlight_read_time","r");
            fscanf(bfile,"%d",&bdata);
            fclose(bfile);

            if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
            {
                alarm(0);
                alarm(bdata);
                system("sh /usr/share/scripts/backlight 4");
                //                system("var=`ps | grep '[X]org' | awk '{print $1}'`;kill -CONT $var");
                printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
                if(ev.value==1)
                {
                    if(buzzer==1)
                    {
                        //                        fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                        //                        fprintf(fp1,"%d",1);
                        //                        fclose(fp1);
                        //                        usleep(50000);
                        //                        fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                        //                        fprintf(fp1,"%d",0);
                        //                        fclose(fp1);
                    }
                }
                if(((int)ev.code==56 || (int)ev.code==58 ) && (int)ev.value==1)
                {
                    if(countk==0)
                    {
                        printf("Small set\n");
                        task_bar_status[0]='1';
                        countk=1;
                    }
                    else if(countk==1)
                    {
                        printf("Caps set\n");
                        task_bar_status[0]='2';
                        countk=2;
                    }
                    else if(countk==2)
                    {
                        printf("Num set\n");
                        task_bar_status[0]='3';
                        countk=0;
                    }
                }
                if((int)ev.value==2 && (int)ev.code==1)
                {
                    count++;
                    printf("Count: %d\n",count);
                    if(count==7)
                    {

                        system("export DISPLAY=:0.0;var=`cat /opt/daemon_files/winid`;xdotool windowunmap $var;xdotool windowmap $var;");
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

                        //system("/usr/bin/shutdown &");
                        //                        pid_t child1 = proc_find("/usr/bin/shutdown");
                        //                        printf("Pid Shutdown:%d\n",child1);
                        //                        if (child1 == -1)
                        //                        {
                        //                            //system("export DISPLAY=:0.0; /usr/bin/shutdown &");
                        //                            system("sh /opt/daemon_files/min.sh 1");
                        //                            sfile = fopen("/usr/share/status/callshutdown","w");
                        //                            fprintf(sfile,"%d",1);
                        //                            fclose(sfile);
                        //                        }
                    }
                }
                if((int)ev.value==0 && (int)ev.code==1)
                {
                    count=0;
                }
            }
            *s++ = task_bar_status[0];
            *s = '\0';
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
        /*****************************Thread Creation*******************/
        while (1) {
            FILE *fp,*fp1;

            if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
                perror("shmat");
                exit(1);
            }
            s = shm;

            //task_bar_status[0]='1'; //Small
            //task_bar_status[0]='2'; //Caps
            //task_bar_status[0]='3'; //nums


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

            int bdata;
            bfile = fopen("/usr/share/status/backlight_read_time","r");
            fscanf(bfile,"%d",&bdata);
            fclose(bfile);

            if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
            {
                alarm(0);
                alarm(bdata);
                system("sh /usr/share/scripts/backlight 4");
                //                system("var=`ps | grep '[X]org' | awk '{print $1}'`;kill -CONT $var");
                printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);
                if(ev.value==1)
                {
                    if(buzzer==1)
                    {
                        //                        fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                        //                        fprintf(fp1,"%d",1);
                        //                        fclose(fp1);
                        //                        usleep(50000);
                        //                        fp1 = fopen("/sys/class/gpio/gpio195/value","w");
                        //                        fprintf(fp1,"%d",0);
                        //                        fclose(fp1);
                    }
                }
                if((int)ev.code==56 && (int)ev.value==1)
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
                    printf("Count: %d\n",count);
                    if(count==7)
                    {

                        system("export DISPLAY=:0.0;var=`cat /opt/daemon_files/winid`;xdotool windowunmap $var;xdotool windowmap $var;");
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

                        //system("/usr/bin/shutdown &");
                        //                        pid_t child1 = proc_find("/usr/bin/shutdown");
                        //                        printf("Pid Shutdown:%d\n",child1);
                        //                        if (child1 == -1)
                        //                        {
                        //                            //system("export DISPLAY=:0.0; /usr/bin/shutdown &");
                        //                            system("sh /opt/daemon_files/min.sh 1");
                        //                            sfile = fopen("/usr/share/status/callshutdown","w");
                        //                            fprintf(sfile,"%d",1);
                        //                            fclose(sfile);
                        //                        }
                    }
                }
                if((int)ev.value==0 && (int)ev.code==1)
                {
                    count=0;
                }
            }
            *s++ = task_bar_status[0];
            *s = '\0';
        }
    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
}
