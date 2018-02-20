/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Publishes a fixed number of simple messages to a topic
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the
 *  GPL v3.0
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <mosquitto.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

// Server connection parameters
#define MQTT_HOSTNAME "106.51.48.231"
#define MQTT_PORT 1883
#define MQTT_USERNAME "clanmqtt"
#define MQTT_PASSWORD "clan123"
//#define MQTT_TOPIC "test"

/*
 * Start here
 */

const char s[2] = "~",sbuf[512];
char *token, *r_machine_id, *cmd, machine_id[10], topic[128], *execmd;

int count=0,internet_connection=0;
FILE *mfile;

#define PACKETSIZE  64
struct packet
{
    struct icmphdr hdr;
    char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;
struct protoent *proto=NULL;
int cnt=1;

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int pingf(char *adress)
{
    const int val=255;
    int i, sd;
    struct packet pckt;
    struct sockaddr_in r_addr;
    int loop;
    struct hostent *hname;
    struct sockaddr_in addr_ping,*addr;

    pid = getpid();
    proto = getprotobyname("ICMP");
    hname = gethostbyname(adress);
    bzero(&addr_ping, sizeof(addr_ping));
    addr_ping.sin_family = hname->h_addrtype;
    addr_ping.sin_port = 0;
    addr_ping.sin_addr.s_addr = *(long*)hname->h_addr;

    addr = &addr_ping;

    sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
    if ( sd < 0 )
    {
        perror("socket");
        close(sd);
        return 1;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        perror("Set TTL option");
        close(sd);
        return 1;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
        perror("Request nonblocking I/O");
        close(sd);
        return 1;
    }
    for (loop=0;loop < 10; loop++)
    {
        int len=sizeof(r_addr);

        if ( recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) > 0 )
        {
            close(sd);
            return 0;
        }

        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = pid;
        for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
            pckt.msg[i] = i+'0';
        pckt.msg[i] = 0;
        pckt.hdr.un.echo.sequence = cnt++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        if ( sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
        {
            perror("sendto");
            close(sd);
            return 1;
        }
        usleep(500000);
    }
    close(sd);
    return 1;
}

/*
 * my_message_callback.
 * Called whenever a new message arrives
 */
void my_message_callback(struct mosquitto *mosq, void *obj,
                         const struct mosquitto_message *message)
{
    FILE *fp;
    char machine_id[10];
    fp = fopen("/usr/share/status/EEPROM-data","r");
    fscanf(fp,"%s",machine_id);
    fclose(fp);

    //    printf("Machine ID: %s\n",machine_id);

    // Note: nothing in the Mosquitto docs or examples suggests that we
    //  must free this message structure after processing it.
    //  printf ("%s\n", (char *)message->payload);

    /* get the first token */
    token = strtok((char *)message->payload, s);

    /* walk through other tokens */
    while( token != NULL ) {
        //        printf( "%s\n", token );
        count++;
        switch(count)
        {
        case 1:
            r_machine_id=token;
            break;
        case 2:
            cmd=token;
            break;
        case 3:
            execmd=token;
        }
        token = strtok(NULL, s);
    }
    //     printf("Remote Machine: %s\n",r_machine_id);
    //     printf("Command Received: %s\n",cmd);
    if(strcmp(machine_id,r_machine_id)==0)
    {
        printf("Machine Id Matched\n");
        if(strcmp(cmd,"verlog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);
        }
        if(strcmp(cmd,"kerlog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);
        }
        if(strcmp(cmd,"applog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);
        }
        if(strcmp(cmd,"dblog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);
        }
        if(strcmp(cmd,"permlog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);

        }
        if(strcmp(cmd,"proclog")==0)
        {
            printf("Received Request\n");
            device_republish(cmd);
        }
        if(strcmp(cmd,"screenshot")==0)
        {
            printf("Received Request\n");
            system("sh /opt/daemon_files/screenshot.sh &");
        }
        if(strcmp(cmd,"execcommand")==0)
        {
            printf("Received Request\n");
            system(execmd);
            printf("Command Executed: %s\n",execmd);
        }
    }
    else
    {
        printf("Machine Id not matched\n");
    }
    count=0;
}

static int run = 1;
void handle_signal(int s)
{
    run = 0;
}

void m_sub_thread_fun(void *vargp)
{
    struct mosquitto *mosq = NULL;
    int rc = 0;

    while(1){
        printf("Thread loop\n");
        // Initialize the Mosquitto library
        if(pingf("106.51.48.231")==0)
        {
            mosquitto_lib_init();

            // Create a new Mosquito runtime instance with a random client ID,
            //  and no application-specific callback data.
            mosq = mosquitto_new (NULL, true, NULL);
            if (!mosq)
            {
                fprintf (stderr, "Can't init Mosquitto library\n");
            }

            // Set up username and password
            mosquitto_username_pw_set (mosq, MQTT_USERNAME, MQTT_PASSWORD);

            // Establish a connection to the MQTT server. Do not use a keep-alive ping
            int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
            if (ret)
            {
                fprintf (stderr, "Can't connect to Mosquitto server\n");
            }

            // Subscribe to the specified topic. Multiple topics can be
            //  subscribed, but only one is used in this simple example.
            //  Note that we don't specify what to do with the received
            //  messages at this point
            ret = mosquitto_subscribe(mosq, NULL, "servercommand", 0);
            if (ret)
            {
                fprintf (stderr, "Can't publish to Mosquitto server\n");
            }

            // Specify the function to call when a new message is received
            mosquitto_message_callback_set (mosq, my_message_callback);

            // Wait for new messages
            while(1){
                rc = mosquitto_loop(mosq, 1, 1);
                if (pingf("106.51.48.251")!=0){
                    sleep(1);
                    break;
                }
            }
            // Tody up. In this simple example, this point is never reached. We can
            //  force the mosquitto_loop_forever call to exit by disconnecting
            //  the session in the message-handling callback if required.
            mosquitto_destroy (mosq);
            mosquitto_lib_cleanup();
        }
        sleep(1);
        printf("Thread working\n");
    }
}

void device_republish(char *command)
{
    struct mosquitto *mosq_sub = NULL;

    char rpayload[20480];

    mosquitto_lib_init();

    // Create a new Mosquito runtime instance with a random client ID,
    //  and no application-specific callback data.
    mosq_sub = mosquitto_new (NULL, true, NULL);
    if (!mosq_sub)
    {
        fprintf (stderr, "Can't initialize Mosquitto library\n");
    }

    mosquitto_username_pw_set (mosq_sub, MQTT_USERNAME, MQTT_PASSWORD);

    // Establish a connection to the MQTT server. Do not use a keep-alive ping
    int ret = mosquitto_connect (mosq_sub, MQTT_HOSTNAME, MQTT_PORT, 0);
    if (ret)
    {
        fprintf (stderr, "Can't connect to Mosquitto server\n");
    }

    if(strcmp(command,"proclog")==0)
    {
        printf("Preparing Proc Log\n");
        system("ps | tail -n 20 > /usr/share/status/debug/proc.log; echo --------------------------------------------------- >> /usr/share/status/debug/proc.log; date >> /usr/share/status/debug/proc.log;echo --------------------------------------------------- >> /usr/share/status/debug/proc.log");
        usleep(700000);

        mfile=fopen("/usr/share/status/debug/proc.log", "r");
        if(mfile==NULL)
        {
            printf("Error in opening file..!!");
        }
        while(fgets(sbuf, 20480, mfile)!=NULL)
        {
            strcat(rpayload,sbuf);
        }
        fclose(mfile);

        strcat(topic,machine_id);
        strcat(topic,"/proc");

        ret = mosquitto_publish (mosq_sub, NULL, topic, strlen(rpayload), rpayload, 0, false);
        if (ret)
        {
            fprintf (stderr, "Can't publish to Mosquitto server\n");
        }
        memset(topic,'\0',sizeof(topic));
        memset(rpayload,'\0',sizeof(rpayload));
    }

    if(strcmp(command,"kerlog")==0)
    {
        printf("Preparing Kernel Log\n");
        system("dmesg | tail -n 100 > /usr/share/status/debug/kernel.log; echo --------------------------------------------------- >> /usr/share/status/debug/kernel.log; date >> /usr/share/status/debug/kernel.log;echo --------------------------------------------------- >> /usr/share/status/debug/kernel.log");
        usleep(700000);

        mfile=fopen("/usr/share/status/debug/kernel.log", "r");
        if(mfile==NULL)
        {
            printf("Error in opening file..!!");
        }
        while(fgets(sbuf, 20480, mfile)!=NULL)
        {
            strcat(rpayload,sbuf);
        }
        fclose(mfile);

        strcat(topic,machine_id);
        strcat(topic,"/dmesg");

        ret = mosquitto_publish (mosq_sub, NULL, topic, strlen(rpayload), rpayload, 0, false);
        if (ret)
        {
            fprintf (stderr, "Can't publish to Mosquitto server\n");
        }
        memset(topic,'\0',sizeof(topic));
        memset(rpayload,'\0',sizeof(rpayload));
    }

    if(strcmp(command,"verlog")==0)
    {
        printf("Preparing Version Log\n");
        usleep(700000);
        mfile=fopen("/usr/share/status/OS-Version", "r");
        if(mfile==NULL)
        {
            printf("Error in opening file..!!");
        }
        while(fgets(sbuf, 20480, mfile)!=NULL)
        {
            strcat(rpayload,sbuf);
        }
        fclose(mfile);

        strcat(topic,machine_id);
        strcat(topic,"/ver");

        ret = mosquitto_publish (mosq_sub, NULL, topic, strlen(rpayload), rpayload, 0, false);
        if (ret)
        {
            fprintf (stderr, "Can't publish to Mosquitto server\n");
        }
        memset(topic,'\0',sizeof(topic));
        memset(rpayload,'\0',sizeof(rpayload));
    }

    if(strcmp(command,"applog")==0)
    {
        printf("Preparing Application Log\n");
        system("cat /home/root/LOG | tail -n 100 > /usr/share/status/debug/app.log; echo --------------------------------------------------- >> /usr/share/status/debug/app.log; date >> /usr/share/status/debug/app.log;echo --------------------------------------------------- >> /usr/share/status/debug/app.log");
        usleep(700000);

        mfile=fopen("/usr/share/status/debug/app.log", "r");
        if(mfile==NULL)
        {
            printf("Error in opening file..!!");
        }
        while(fgets(sbuf, 20480, mfile)!=NULL)
        {
            strcat(rpayload,sbuf);
        }
        fclose(mfile);

        strcat(topic,machine_id);
        strcat(topic,"/applog");

        ret = mosquitto_publish (mosq_sub, NULL, topic, strlen(rpayload), rpayload, 0, false);
        if (ret)
        {
            fprintf (stderr, "Can't publish to Mosquitto server\n");
        }
        memset(topic,'\0',sizeof(topic));
        memset(rpayload,'\0',sizeof(rpayload));
    }
    mosquitto_disconnect (mosq_sub);
    mosquitto_destroy (mosq_sub);
    mosquitto_lib_cleanup();

}

char *my_itoa(int num, char *str)
{
    if(str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

int main (int argc, char *argv[])
{
    int connection_status=0;
    if( argc == 2 ) {
        if(strcmp(argv[1],"-d")==0)
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
        }
    }
    else if( argc > 2 ) {
        printf("Too many arguments supplied.\n");
    }
    else {
        printf("Debug Mode.\n");
    }

    pthread_t m_sub;
    pthread_create(&m_sub,NULL,m_sub_thread_fun,NULL);
    //    pthread_join(m_sub, NULL);

    time_t rawtime;
    struct tm * timeinfo;

    char mpayload[20480], s[512], ping_stat;;
    char mpayloadTemp[20480]; // used to concatenate HW health counters

    while(1){
        printf("Internet Connection: %d\n", internet_connection);
        if (pingf("106.51.48.231")==0)
        {
            struct mosquitto *mosq = NULL;

            mfile=fopen("/usr/share/status/EEPROM-data","r");
            fscanf(mfile,"%s",machine_id);
            fclose(mfile);

            // Initialize the Mosquitto library
            mosquitto_lib_init();

            // Create a new Mosquito runtime instance with a random client ID,
            //  and no application-specific callback data.
            mosq = mosquitto_new (NULL, true, NULL);
            if (!mosq)
            {
                fprintf (stderr, "Can't initialize Mosquitto library\n");
                connection_status=0;
            }
            else
            {
                connection_status=1;
            }

            mosquitto_username_pw_set (mosq, MQTT_USERNAME, MQTT_PASSWORD);

            // Establish a connection to the MQTT server. Do not use a keep-alive ping
            int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
            if (ret)
            {
                fprintf (stderr, "Can't connect to Mosquitto server\n");
                connection_status=0;
            }
            else
            {
                connection_status=1;
            }
            if(connection_status==1)
            {
                //<-------------------------------------Time Topic--------------------------------->
                time ( &rawtime );
                timeinfo = localtime ( &rawtime );
                strcat(mpayload,asctime (timeinfo));
                sleep(5);
                strcat(topic,machine_id);
                strcat(topic,"/time");
                //        printf("Topic: %s\n",topic);
                //        printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                    connection_status=0;
                }
                else
                {
                    connection_status=1;
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Time Topic--------------------------------->

                //<-------------------------------------Charger Topic--------------------------------->
                system("cat /sys/class/gpio/gpio110/value 1> /usr/share/status/debug/chr.log");
                sleep(5);
                mfile = fopen("/usr/share/status/debug/chr.log","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",mpayload);
                    fclose(mfile);
                }

                strcat(topic,machine_id);
                strcat(topic,"/chr");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Charger Topic--------------------------------->

                //<-------------------------------------Printer Paper Topic--------------------------------->
                sleep(5);
                mfile = fopen("/usr/share/status/PRINTER_status","r");
                fscanf(mfile,"%s",mpayload);
                fclose(mfile);

                /** adding paper length to ppstat topic **/

                memset(mpayloadTemp,'0',sizeof(char));
                mfile = fopen("/usr/share/status/debug/printLength.log","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",mpayloadTemp);
                    fclose(mfile);
                }
                strcat(mpayload,"~");
                strcat(mpayload,mpayloadTemp);
                memset(mpayloadTemp,'\0',sizeof(mpayloadTemp));

                /** --------- **/

                strcat(topic,machine_id);
                strcat(topic,"/ppstat");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Printer Paper Topic--------------------------------->

                //<-------------------------------------Network Mode Topic--------------------------------->
                mfile = fopen("/opt/daemon_files/ping_status","r");
                if(mfile)
                {
                    fscanf(mfile,"%c",&ping_stat);
                    fclose(mfile);
                }

                sleep(5);
                mpayload[0]=ping_stat;

                strcat(topic,machine_id);
                strcat(topic,"/nmod");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Network Mode Topic--------------------------------->

                //<-------------------------------------Keypad Mode Topic--------------------------------->
                sleep(5);
                mfile = fopen("/usr/share/status/KEYPAD_mode","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",mpayload);
                    fclose(mfile);
                }

                strcat(topic,machine_id);
                strcat(topic,"/kmod");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Keypad Mode Topic--------------------------------->

                //<-------------------------------------Ram Topic--------------------------------->
                system("free | grep \"Mem\" | awk {'print $3'} > /usr/share/status/debug/ram.log");
                sleep(5);
                mfile = fopen("/usr/share/status/debug/ram.log","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",mpayload);
                    fclose(mfile);
                }

                strcat(topic,machine_id);
                strcat(topic,"/ram");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Ram Topic--------------------------------->

                //<-------------------------------------Cpu Topic--------------------------------->
                system("cpu=`top -n 1 | grep \"[i]dle\" | awk '{print $2}'`; echo ${cpu//%} 1> /usr/share/status/debug/cpu.log");
                sleep(5);
                mfile = fopen("/usr/share/status/debug/cpu.log","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",mpayload);
                    fclose(mfile);
                }

                strcat(topic,machine_id);
                strcat(topic,"/cpu");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Cpu Topic--------------------------------->

                //<-------------------------------------Gprs Operator Topic--------------------------------->
                if(ping_stat=='G' || ping_stat=='g')
                {
                    system("STR=`cat /opt/daemon_files/rough_files/current_operator`; IFS=',' read -ra NAMES <<< \"$STR\"; echo ${NAMES[2]} 1> /usr/share/status/debug/operator.log");
                    sleep(5);
                    mfile = fopen("/usr/share/status/debug/operator.log","r");
                    if(mfile)
                    {
                        fscanf(mfile,"%s",mpayload);
                        fclose(mfile);
                    }

                    strcat(topic,machine_id);
                    strcat(topic,"/noper");
                    //                printf("Topic: %s\n",topic);
                    //                printf("Payload: %s\n",mpayload);
                    ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                    if (ret)
                    {
                        fprintf (stderr, "Can't publish to Mosquitto server\n");
                    }
                    memset(topic,'\0',sizeof(topic));
                    memset(mpayload,'\0',sizeof(mpayload));
                    //<-------------------------------------Gprs Operator Topic--------------------------------->

                    //<-------------------------------------Gprs Signal Topic--------------------------------->

                    system("STR=`cat /opt/daemon_files/rough_files/signal_level`;IFS=','; read -ra NAMES <<< \"$STR\"; echo ${NAMES[0]} 1> /usr/share/status/debug/gsignal.log");
                    sleep(5);
                    mfile = fopen("/usr/share/status/debug/gsignal.log","r");
                    //                    fscanf(mfile,"%s",mpayload);
                    if(mfile)
                    {
                        while(fgets(sbuf, 20480, mfile)!=NULL)
                        {
                            strcat(mpayload,sbuf);
                        }
                        fclose(mfile);
                    }

                    strcat(topic,machine_id);
                    strcat(topic,"/nsig");
                    //                printf("Topic: %s\n",topic);
                    //                printf("Payload: %s\n",mpayload);
                    ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                    if (ret)
                    {
                        fprintf (stderr, "Can't publish to Mosquitto server\n");
                    }
                    memset(topic,'\0',sizeof(topic));
                    memset(mpayload,'\0',sizeof(mpayload));
                }
                //<-------------------------------------Gprs Signal Topic--------------------------------->

                //<-------------------------------------Wifi ssid Topic--------------------------------->

                else if(ping_stat=='W' || ping_stat=='w')
                {
                    system("iwgetid | awk {'print $2'} 1> /usr/share/status/debug/wifissid.log");
                    sleep(5);
                    mfile = fopen("/usr/share/status/debug/wifissid.log","r");
                    if(mfile)
                    {
                        fscanf(mfile,"%s",mpayload);
                        fclose(mfile);
                    }

                    strcat(topic,machine_id);
                    strcat(topic,"/noper");
                    //                printf("Topic: %s\n",topic);
                    //                printf("Payload: %s\n",mpayload);
                    ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                    if (ret)
                    {
                        fprintf (stderr, "Can't publish to Mosquitto server\n");
                    }
                    memset(topic,'\0',sizeof(topic));
                    memset(mpayload,'\0',sizeof(mpayload));
                    //<-------------------------------------Wifi ssid Topic--------------------------------->

                    //<-------------------------------------Wifi Signal Topic--------------------------------->

                    sleep(5);
                    mfile = fopen("/opt/daemon_files/signal_level","r");
                    if(mfile)
                    {
                        fscanf(mfile,"%s",mpayload);
                        fclose(mfile);
                    }


                    strcat(topic,machine_id);
                    strcat(topic,"/nsig");
                    //                printf("Topic: %s\n",topic);
                    //                printf("Payload: %s\n",mpayload);
                    ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                    if (ret)
                    {
                        fprintf (stderr, "Can't publish to Mosquitto server\n");
                    }
                    memset(topic,'\0',sizeof(topic));
                    memset(mpayload,'\0',sizeof(mpayload));
                }
                //<-------------------------------------Wifi Signal Topic--------------------------------->

                //<-------------------------------------Battery Topic--------------------------------->
                int battery_input, percentage;
                char test[3];
                system("cat /sys/class/power_supply/NUC970Bat/present 1> /usr/share/status/debug/bat.log");
                sleep(5);
                mfile = fopen("/usr/share/status/debug/bat.log","r");
                if(mfile)
                {
                    fscanf(mfile,"%s",test);
                    fclose(mfile);
                }

                battery_input = atoi(test);

                percentage = ((battery_input-75)*100)/(98-75);
                //                printf("Battery Per: %d\n",percentage);

                my_itoa(percentage,mpayload);

                strcat(topic,machine_id);
                strcat(topic,"/bat");
                //                printf("Topic: %s\n",topic);
                //                printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Battery Topic--------------------------------->

                //<-------------------------------------Process Topic--------------------------------->
                //                system("ps | tail -n 20 > /usr/share/status/debug/proc.log");
                //                sleep(2);
                //                mfile=fopen("/usr/share/status/debug/proc.log", "r");
                //                if(mfile==NULL)
                //                {
                //                    printf("Error in opening file..!!");
                //                }
                //                while(fgets(s, 20480, mfile)!=NULL)
                //                {
                //                    strcat(mpayload,s);
                //                }
                //                fclose(mfile);
                //                strcat(topic,machine_id);
                //                strcat(topic,"/proc");
                //                //        printf("Topic: %s\n",topic);
                //                //        printf("Payload: %s\n",mpayload);
                //                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                //                if (ret)
                //                {
                //                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                //                }
                //                memset(topic,'\0',sizeof(topic));
                //                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Process Topic--------------------------------->

                //<-------------------------------------Disk Topic--------------------------------->

                system("sh /opt/mem_status");
                sleep(5);

                mfile=fopen("/usr/share/status/mem_state", "r");
                if(mfile==NULL)
                {
                    printf("Error in opening file..!!");
                }
                while(fgets(s, 20480, mfile)!=NULL)
                {
                    strcat(mpayload,s);
                }
                fclose(mfile);

                strcat(topic,machine_id);
                strcat(topic,"/disk");
                //        printf("Topic: %s\n",topic);
                //        printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Disk Topic--------------------------------->

                //<-------------------------------------Version Topic--------------------------------->
                //                sleep(5);
                //                mfile=fopen("/usr/share/status/OS-Version", "r");
                //                if(mfile==NULL)
                //                {
                //                    printf("Error in opening file..!!");
                //                }
                //                while(fgets(s, 20480, mfile)!=NULL)
                //                {
                //                    strcat(mpayload,s);
                //                }
                //                fclose(mfile);

                //                strcat(topic,machine_id);
                //                strcat(topic,"/ver");
                //                //        printf("Topic: %s\n",topic);
                //                //        printf("Payload: %s\n",mpayload);
                //                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                //                if (ret)
                //                {
                //                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                //                }
                //                memset(topic,'\0',sizeof(topic));
                //                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------Version Topic--------------------------------->

                //<-------------------------------------GPS Topic--------------------------------->
                sleep(5);

                if(mfile=fopen("/usr/share/status/GPS_DATA", "r"))
                {
                    if(mfile==NULL)
                    {
                        printf("Error in opening file..!!");
                    }
                    while(fgets(s, 20480, mfile)!=NULL)
                    {
                        strcat(mpayload,s);
                    }
                    fclose(mfile);
                }
                else{
                    strcat(mpayload,"Not Fixed");
                }

                strcat(topic,machine_id);
                strcat(topic,"/gps");
                //        printf("Topic: %s\n",topic);
                //        printf("Payload: %s\n",mpayload);
                ret = mosquitto_publish (mosq, NULL, topic, strlen(mpayload), mpayload, 0, false);
                if (ret)
                {
                    fprintf (stderr, "Can't publish to Mosquitto server\n");
                }
                memset(topic,'\0',sizeof(topic));
                memset(mpayload,'\0',sizeof(mpayload));
                //<-------------------------------------GPS Topic--------------------------------->
            }
            // Tidy up
            mosquitto_disconnect (mosq);
            mosquitto_destroy (mosq);
            mosquitto_lib_cleanup();
        }
        else
        {
            printf("Unable to ping internet\n");
            sleep(2);
        }
    }
    return 0;
}
