#include <stdio.h>

#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <sys/ioctl.h>

#include <netinet/in.h>

#include <net/if.h>

#include <unistd.h>

#include <arpa/inet.h>

 

static void ip_address(char* i_name)

{

    int n;

    struct ifreq ifr;

    n = socket(AF_INET, SOCK_DGRAM, 0);

    //Type of address to retrieve - IPv4 IP address

    ifr.ifr_addr.sa_family = AF_INET;

    //Copy the interface name in the ifreq structure

    strncpy(ifr.ifr_name , i_name , IFNAMSIZ - 1);

    ioctl(n, SIOCGIFADDR, &ifr);

    close(n);

    //display result
    FILE *fp;
    fp = fopen ("/opt/daemon_files/ip_address","w");
    fprintf(fp,"%s",inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    fclose(fp);
    printf("IP Address is %s - %s\n" , i_name , inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );

}

int main()
{
	ip_address("ppp0");
}
