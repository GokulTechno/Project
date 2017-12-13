#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
    struct stat st = {0};
    FILE *file;

    if (stat("/media/thumbdrive", &st) == -1) {
        mkdir("/media/thumbdrive", 0700);
    }
    umount("/dev/sda1");
    mount("/dev/sda1","/media/thumbdrive","vfat",0, NULL);

    file = fopen("/media/thumbdrive/patch.clan", "r");
    if (file){
       system("sh /media/thumbdrive/patch.clan L1n@x_P@tc8");
       fclose(file);
    }

    return 0;
}

