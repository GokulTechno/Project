#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
    struct stat st = {0};
    FILE *file;

    if (stat("/media/sdcard", &st) == -1) {
        mkdir("/media/sdcard", 0700);
    }
    umount("/dev/mmcblk0p1");
    mount("/dev/mmcblk0p1","/media/sdcard","vfat",0, NULL);

    file = fopen("/media/sdcard/patch.clan", "r");
    if (file){
       system("sh /media/sdcard/patch.clan L1n@x_P@tc8");
       fclose(file);
    }

    return 0;
}

