#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#define DEV_NAME "/dev/ttyS2"
#define FILE_PATH "/home/root/file.txt"
#define BAUDRATE B9600

int main()
{
    char str[128];

    /*****************************Daemon******************************/
        pid_t pid=0;
        pid_t sid=0;
        pid = fork();
        if (pid < 0) { exit(1); }
        if (pid > 0) { exit(0); }
        umask(0);
        sid = setsid();
        if (sid < 0) { exit(1); }
        chdir("/");
        if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    /*****************************Daemon******************************/

    /*****************************Terminal Config**********************/

    int gps_fd;
    struct termios oldtio,newtio;
    gps_fd = open(DEV_NAME, O_RDWR | O_NOCTTY );
    tcgetattr(gps_fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    tcflush(gps_fd, TCIFLUSH);
    tcsetattr(gps_fd,TCSANOW,&newtio);

    /*****************************Terminal Config**********************/

    //fcntl(gps_fd, F_SETFL, O_NONBLOCK);

    while(1)
    {
        read(gps_fd, str, sizeof(str));

        //gps_parser(str);
        int n, i, j = 1, k,x, count = 0,z=0,ch,p=0,y,timi;
        int c[100];
        char msg[6],lat[20],tim[20],lon[20],date[20],speed[20],alti[20];
        c[0] = 0;
        FILE *fp = NULL;

        printf("%s\n",str);
        n = strlen(str);

        if(n>10)
        {
            for (i = 0; i < n; i++)
            {
                if (str[i] == ',')
                {
                    c[j] = i;
                    count++;
                    if (count == 1)
                    {
                        //printf("Message ID:");
                        for (k = c[j - 1]; k < c[j]; k++)
                        {
                            if (str[k] != ',')
                            {
                                //		printf("%c", str[k]);
                            }
                        }
                        j++;
                        for(x=0;x<=5;x++)
                        {
                            msg[x]=str[x];
                        }
                        if(strcmp("$GPGGA",msg)==0)
                        {
                            ch=1;
                        }
                        else if(strcmp("$GPRMC",msg)==0)
                        {
                            ch=2;
                        }
                        else if(strcmp("$GPVTG",msg)==0)
                        {
                            ch=3;
                        }
                        else if(strcmp("$GPGSA",msg)==0)
                        {
                            ch=4;
                        }
                        else
                        {
                            ch=5;
                        }
                        switch(ch)
                        {
                        case 1:
                            // printf("\n*******FOUND GPGGA*********\n");
                            for (i = 8; i < n; i++)
                            {
                                if (str[i] == ',')
                                {
                                    c[j]=i;
                                    count++;
                                    if (count == 2)
                                    {
                                        //printf("UTC Time:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                tim[z]=str[k];
                                                z++;
                                            }
                                        }
                                        tim[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 3)
                                    {
                                        //printf("Latitude:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                lat[z]=str[k];
                                                z++;
                                            }
                                        }
                                        lat[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 4)
                                    {
                                        //printf("N/S Indicator:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 5)
                                    {
                                        //printf("Longitude:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                lon[z]=str[k];
                                                z++;
                                            }
                                        }
                                        lon[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 6)
                                    {
                                        //printf("E/W Indicator");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 7)
                                    {
                                        //printf("Position Fix Indicator:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 8)
                                    {
                                        //printf("Satellites used:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 9)
                                    {
                                        //printf("HDOP:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 10)
                                    {
                                        //printf("MSL Altitude:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                                alti[z]=str[k];
                                                z++;
                                            }
                                        }
                                        alti[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 11)
                                    {
                                        //printf("Units:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 12)
                                    {
                                        //printf("Geiod Seperation:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 13)
                                    {
                                        //printf("Units:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 14)
                                    {
                                        //printf("Age of Diff.corr:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                }
                            }
                            count=0;
                            j=1;
                            break;
                        case 2:
                            //  printf("\n*******FOUND GPRMC*********\n");
                            for(i=7;i<n;i++)
                            {
                                if(str[i]==',')
                                {   c[j]=i;
                                    count++;
                                    if (count == 2)
                                    {
                                        //printf("UTC Time:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                tim[z]=str[k];
                                                z++;
                                            }
                                        }
                                        tim[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 3)
                                    {
                                        //printf("Status:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 4)
                                    {
                                        //printf("Latitude:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                lat[z]=str[k];
                                                z++;
                                            }
                                        }
                                        lat[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 5)
                                    {
                                        //printf("N/S indicator:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 6)
                                    {
                                        //printf("Longitude:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                lon[z]=str[k];
                                                z++;
                                            }
                                        }
                                        lon[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 7)
                                    {
                                        //printf("E/W indicator:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 8)
                                    {
                                        //printf("Speed over ground:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                speed[z]=str[k];
                                                z++;
                                            }
                                        }
                                        speed[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 9)
                                    {
                                        //printf("Course over ground:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 10)
                                    {
                                        //printf("Date:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                date[z]=str[k];
                                                z++;
                                            }
                                        }
                                        date[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 11)
                                    {
                                        //printf("Magnitude variation:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                }
                            }
                            j=1;
                            count=0;
                            break;
                        case 3:
                            //   printf("\n*******FOUND GPVTG*********\n");
                            for(i=7;i<n;i++)
                            {
                                if(str[i]==',')
                                {   c[j]=i;
                                    count++;
                                    if (count == 2)
                                    {
                                        //printf("Course:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 3)
                                    {
                                        //printf("Reference:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 4)
                                    {
                                        //printf("course:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 5)
                                    {
                                        //printf("Reference:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 6)
                                    {
                                        //printf("Speed:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                speed[z]=str[k];
                                                z++;
                                            }
                                        }
                                        speed[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 7)
                                    {
                                        //printf("Units:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 8)
                                    {
                                        //printf("Speed:");
                                        for (k = c[j - 1],z=0; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                speed[z]=str[k];
                                                z++;
                                            }
                                        }
                                        speed[z]='\0';
                                        j++;
                                        //printf("\n");
                                    }
                                    else if (count == 9)
                                    {
                                        //printf("Units:");
                                        for (k = c[j - 1]; k < c[j]; k++)
                                        {
                                            if (str[k] != ',')
                                            {
                                                //printf("%c", str[k]);
                                            }
                                        }
                                        j++;
                                        //printf("\n");
                                    }
                                }
                            }
                            //printf("Check Sum:");
                            for (k = c[j - 1]; k < n; k++)
                            {
                                if (str[k] != ',')
                                {
                                    //printf("%c", str[k]);
                                }
                            }
                            j=1;
                            count=0;
                            break;
                        case 4:
                            //printf("\n*******FOUND GPGSA*********");
                            break;
                        case 5:
                            //printf("\n*******FOUND GPGSV*********");
                            break;
                        }
                        //printf("\n");
                    }
                }
            }
            count=0;
            j=1;
        }
        printf("Latitude Received: %s\n",lat);
        printf("Longitude Received: %s\n",lon);

        timi=atoi(tim);
        timi+=53000;

        fp = fopen(FILE_PATH,"w");
        fprintf(fp,"latitude        -%s\n",lat);
        fprintf(fp,"longitude       -%s\n",lon);
        fprintf(fp,"time utc        -20%c%c-%c%c-%c%cT%c%c:%c%c:%c%c.00Z\n",date[4],date[5],date[2],date[3],date[0],date[1],tim[0],tim[1],tim[2],tim[3],tim[4],tim[5]);
        fprintf(fp,"altitude (m)    -%s\n",alti);
        fprintf(fp,"speed (m/s)     -%s\n",speed);
        fflush(fp);
        fclose(fp);

        memset(str,'\0',sizeof(str));
    }
    close(gps_fd);
    return 0;
}
