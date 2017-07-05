#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>  
#include <fcntl.h>   
#include <errno.h>   
#include <termios.h> 

#define DEV_NAME "/dev/mux1"
#define BAUDRATE B115200
#define dist "/home/root/bluedata"

void Bluetooth_init(void);
void Bluetooth_pair(void);
void Bluetooth_spp_pair(void);
void Bluetooth_print_protocol(void);

FILE *fp, *pfile;
int fd,res,i,j,k,l,m=0,n=0,count,state;
struct termios oldtio,newtio;
unsigned char char1[150],str[50],msg[120];
int fonttype, fontsize, fontallignment, fontstyle, printermode=0;
unsigned char bton[13]="AT+QBTPWR=1\n",btoff[13]="AT+QBTPWR=0\n",btvisb[14]="AT+QBTVISB=1\n",btname[26]="AT+QBTNAME=\"BluePrint\"\n",btpairacpt[17]="AT+QBTPAIRCNF=1\n",btppset[18]="AT+QBTCONN=1,0,2\n",btsppacpt[16]="AT+QBTACPT=1,2\n";

int main()
{


/*
//Daemon
	pid_t pid=0;
	pid_t sid=0;

	pid = fork();

	if (pid < 0) { exit(1); }

	if (pid > 0) { exit(0); }

	umask(0);
	sid = setsid();

	if (sid < 0) { exit(1); }
	
	chdir("/");	

//	if ((chdir("/")) < 0) { exit(EXIT_FAILURE); }

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
*/  
while(1)
{// Ever loop started
    Bluetooth_init();   
}// Ever loop
return 0;
}

void Bluetooth_init(void)
{
    fd = open(DEV_NAME, O_RDWR | O_NOCTTY );  
    tcgetattr(fd,&oldtio); /* save current serial port settings */
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);    

    write(fd,btoff,sizeof(btoff));
    sleep(3);
    write(fd,bton,sizeof(bton));
    printf("BT Enabled...\n");
    sleep(2);
    k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	//printf("%s",char1);
	if(char1[0]=='O' && char1[1]=='K')
	{
	printf("Success\n");
	state=0;
	}
	for(i=0;i<sizeof(char1);i++)
	{
		char1[i]='\0';
	}
    }

    write(fd,btvisb,sizeof(btvisb));
    printf("BT Visibility Enabled...\n");
 //   sleep(1);

    k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	//printf("%s",char1);
	if(char1[0]=='O' && char1[1]=='K')
	{
	printf("Success\n");
	state=0;
	}
	for(i=0;i<sizeof(char1);i++)
	{
		char1[i]='\0';
	}
    }

    write(fd,btname,sizeof(btname));
    printf("BT Name Changed...\n");
 //   sleep(1);
    k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	//printf("%s",char1);
	if(char1[0]=='O' && char1[1]=='K')
	{
	printf("Success\n");
	state=0;
	}
	for(i=0;i<sizeof(char1);i++)
	{
		char1[i]='\0';
	}
    }
    printf("Waiting for pair request...\n");
    Bluetooth_pair();
};

void Bluetooth_pair(void){

		k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	//printf("%s",char1);
	if(char1[10]=='p' && char1[11]=='a' && char1[12]=='i' && char1[13]=='r')
	{
/*
	l=17;int n=1,nstate=1;
		while(n=nstate)
		{
		char1[l]=str[m];
		m++;l++;
		if(char1[l]=='"')
		nstate=0;
		}
		str[m]='\0';
*/
	printf("Pair Request Received \n",str);
	write(fd,btpairacpt,sizeof(btpairacpt));
        printf("BT Pair Accepted...\n");
	state=0;
	}

	else if((char1[0]=='+' && char1[1]=='Q' && char1[2]=='B' && char1[3]=='T' && char1[4]=='A' && char1[5]=='C' && char1[6]=='P' && char1[7]=='T') || (char1[0]=='+' && char1[1]=='Q' && char1[2]=='B' && char1[3]=='T' && char1[4]=='C' && char1[5]=='O' && char1[6]=='N' && char1[7]=='N'))
	{
		printf("Pairing Connected Device\n",str);
		state=0;
	}
	memset(char1,'\0',sizeof(char1));
    }
    
    
  //  sleep(1);  
    
    write(fd,btppset,sizeof(btppset));
    printf("BT SPP Profile Set...\n");
 //   sleep(1);
	Bluetooth_spp_pair();
	};

void Bluetooth_spp_pair(void)
{
	k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	sleep(1);
	//printf("%s",char1);
	if((char1[0]=='+' && char1[1]=='Q' && char1[2]=='B' && char1[3]=='T' && char1[4]=='I' && char1[5]=='N' && char1[6]=='D') && (char1[10]=='c' && char1[11]=='o' && char1[12]=='n' && char1[13]=='n'))
	{
	printf("SPP Profile Request Received\n");
	write(fd,btppset,sizeof(btppset));
	sleep(1);
	write(fd,btsppacpt,sizeof(btsppacpt));
        printf("BT SPP Profile accepted and Set to transperent mode...\n");
	state=0;
	}
	memset(char1,'\0',sizeof(char1));
    }  
  sleep(1); 

  
    k=1;state=1;
    while(k=state)
    {	
	read(fd,char1,sizeof(char1));
	//printf("%s",char1);
	if(char1[0]=='C' && char1[1]=='O' && char1[2]=='N' && char1[3]=='N' && char1[4]=='E' && char1[5]=='C' && char1[6]=='T')
	{
	printf("SPP Client Connected\n");
	write(fd,"Please Enter the command to print\n",33);
	state=0;
	}
	for(i=0;i<sizeof(char1);i++)
	{
		char1[i]='\0';
	}
    }
	Bluetooth_print_protocol();
};


void Bluetooth_print_protocol(void)
{
	k=1;state=1;
    while(k=state)
	{
	  read(fd,char1,sizeof(char1));
 	if(char1[0]=='C' && char1[1]=='L' && char1[2]=='O' && char1[3]=='S' && char1[4]=='E' && char1[5]=='D')
	  {
		printf("Client Disconnected\n");
		state=0;
	  }
		//printf("%s\n",char1);

if(char1[0]=='E' && char1[1]=='S' && char1[2]=='C' &&  char1[4]=='@')
{
	//printf("Command Received\n");
	/*
		fonttype:	Regular=1
				Bold=2
				Italic=3
		fontsize:	Small=1
				Medium=2
				Large=3
		fontallignment: Left=1
				Center=2
				Right=3
	*/
	fonttype=1;fontsize=2; fontallignment=1;fontstyle=1;
	printermode=1;
}
else if(char1[0]=='E' && char1[1]=='S' && char1[2]=='C' &&  char1[4]=='a')
{
	switch(char1[6])
	{
		case '0':
			fontallignment=1;
		break;
		case '1':
			fontallignment=2;
		break;
		case '2':
			fontallignment=3;
		break;
	}
}
else if(char1[0]=='E' && char1[1]=='S' && char1[2]=='C' &&  char1[4]=='!')
{
	switch(char1[6])
	{
		case '1':
			fonttype=3;
		break;
		case '2':
			fonttype=1;
		break;
		case '3':
			fonttype=2;
		break;
		case '4':
			fontsize=3;
		break;
		case '5':
			fontsize=2;
		break;
		case '6':
			fontsize=1;
		break;
	}
}
else if(char1[0]=='L' && char1[1]=='F')
{
	switch(char1[3])
	{
	case '1':
		system("echo ~R0030 > /dev/printer");
	break;
	case '2':
		system("echo ~R0040 > /dev/printer");
	break;
	default:
		system("echo ~R0020 > /dev/printer");
	break;
	}
}
else if(printermode==1 && strlen(char1) > 3)
{
	int len,i;
	char count[4],final[100];
	memset(count,'\0',sizeof(count));
	memset(final,'\0',sizeof(final));
	char1[strlen(char1)]='\0';
	char1[strlen(char1)-1]='\0';
	len=strlen(char1)+12;
//Calculating Total character length to print	
	if(len<100)
	{
	sprintf(count, "00%d", len);
	}
	else if(len<1000)
	{
	sprintf(count, "0%d", len);
	}
	else if(len<10000)
	{
	sprintf(count, "%d", len);
	}
	/*
		fonttype:	Regular=1
				Bold=2
				Italic=3
		fontsize:	Small=1
				Medium=2
				Large=3
		fontallignment: Left=1
				Center=2
				Right=3
	*/
//*********************************Protocol starting here**********************************
	strcat(final,"~E");
	strcat(final,count);
	strcat(final,char1);
//Setting Font Type(Regualar, Bold, Italic)
	if(fonttype==1)
	{
		strcat(final,"R");
	}
	else if(fonttype==2)
	{
		strcat(final,"B");
	}
	else if(fonttype==3)
	{
		strcat(final,"I");
	}
//Setting Font Size(Small, Medium, Large)
	if(fontsize==1)
	{
		strcat(final,"S");
	}
	else if(fontsize==2)
	{
		strcat(final,"M");
	}
	else if(fontsize==3)
	{
		strcat(final,"L");
	}
//Setting Font Allignment(Left, Center, Right);
	if(fontallignment==1)
	{
		strcat(final,"L");
	}
	else if(fontallignment==2)
	{
		strcat(final,"C");
	}
	else if(fontallignment==3)
	{
		strcat(final,"R");
	}

	strcat(final,"~");
	if(fontstyle==1)
	{
	strcat(final,"1");
	}
	else if(fontstyle==2)
	{
	strcat(final,"2");
	}
	strcat(final,"!");
	int finlen=strlen(final);
//Protocol ends here
/*
	for(i=1;i<=7;i++)
	{
		final[finlen-i]='\0';
	}
*/
	printf("%s\n", final);
	pfile = fopen("/dev/printer", "w+");
	fwrite(final,1,sizeof(final),pfile);
	fclose(pfile);
	//system(final);
printermode=0;	
}
		for(i=0;i<sizeof(char1);i++)
		{
			char1[i]='\0';
		}
	}
    close(fd);
};

