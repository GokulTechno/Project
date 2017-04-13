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

#define MOUSEFILE "/dev/input/event0"

#ifndef EV_SYN
#define EV_SYN 0
#endif
#define SHMSZ     27

int count=0;

int main()
{

char c;
int shmid;
key_t key;
char *shm, *s;
	
key = 1111;

    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    s = shm;

    int fd;
    struct input_event ie;

    if((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
        perror("opening device");
        exit(EXIT_FAILURE);
    }

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

    while(read(fd, &ie, sizeof(struct input_event))) {
//        printf("time %ld.%06ld\ttype %d\tcode %d\tvalue %d\n",
//               ie.time.tv_sec, ie.time.tv_usec, ie.type, ie.code, ie.value);


 if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

s = shm;
*s++ = '^';
*s = '\0';


}
    return 0;
}
