//***********************************************************************//
//                                                                       //
//                                                                       //
//      PRINTER DRIVER SOURCE FOR IMAGE PRINTING AND TEXT PRINTING       //
//                                                                       //
//                                                                       //
//***********************************************************************//

// This driver source have designed for both image printing and text printing
// Version : V1.0


//-------Kernel Header files included------

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/gpio.h>
#include<linux/delay.h>
#include <linux/cdev.h>
#include <linux/spi/spi.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/file.h>


//-------Header files included------

#include "motor.h"
#include "final_font.h"

//-------macros defined -------

#define SPI_BUFF_SIZE	128
#define USER_BUFF_SIZE	2048

#define SPI_BUS 1
#define SPI_BUS_CS1 0
#define SPI_BUS_SPEED 10000000

//------function decleration--------------

int Noofbytes(void);
void rotate(int);
int rotate_stat;
int bat_present=0;

//------Driver name-----------------------
const char this_driver_name[] = "printer";

//-------------
struct printer_control
{
    struct spi_message msg;
    struct spi_transfer transfer;
    u8 *tx_buff;
    u8 *rx_buff;
};
struct printer_control printer_ctl;

//------------
struct printer_dev {
    struct semaphore spi_sem;
    struct semaphore fop_sem;
    dev_t devt;
    struct cdev cdev;
    struct class *class;
    struct spi_device *spi_device;
    char *user_buff;
    u8 test_data;
};
struct printer_dev printer_dev;

//-------------------------variable declarations---------------------
char **buff,temp_image,q;
int length,i,k,z,n,y=100,g[2000],w,x,odd,even,file=10,low_bat,rotate_pulse_count=1,rotate_loop=0,loop=0;
u8 tmp[48];
u32 *addr=&tmp;
char data_read[100];

char data[200];
int data_size=0,data_length,buffer_check1=0,allignment;
int start=0,end=0,j;
int no_of_lines=0,size,font,fontstyle;
int width=0,height,line_wise=0;
unsigned short int envy[50];
char tmp_buff[48];

//-----------------------paper sensing variable--------------------------

struct file *f,*f1,*fp_paper_sensing,*f_LB;
char buf1[1000000],paper[2];
mm_segment_t fs;
mm_segment_t new;

char *value;
unsigned char data1,data2,data3;
char m;

unsigned int NoOfBytes;
unsigned int Temp=0;
unsigned char Dummy=0;

struct file *f2;
char buf[2046],LB_buf1[5];
char *paper_value;
char paper_buff[6];

unsigned int paper_NoOfBytes;
unsigned int paper_Temp=0;
unsigned char paper_Dummy=0;

//--------------------------To find the no of bytes in the protocol & height of the image--------------

int Noofbytes()
{

    NoOfBytes=((g[2]&0x0F)<<12);
    NoOfBytes|=((g[3]&0x0F)<<8);
    NoOfBytes|=((g[4]&0x0F)<<4);
    NoOfBytes|=(g[5]&0x0F);

    Dummy=NoOfBytes>>12&0x0F;
    Temp=Temp*0x0A+Dummy;
    Dummy=NoOfBytes>>8&0x0F;
    Temp=Temp*0x0A+Dummy;
    Dummy=NoOfBytes>>4&0x0F;
    Temp=Temp*0x0A+Dummy;
    Dummy=NoOfBytes&0x0F;
    Temp=Temp*0x0A+Dummy;
    NoOfBytes=Temp;

    return NoOfBytes;
}

int strtoint(char *data)
{
    int bat;
    bat = ((data[0]-0x30)*10)+(data[1]-0x30);
    return bat;

}

//-----------------------------------Motor Rotation-------------------------------------------------

void rotate(int rotate_loop)
{
//    write_file("/usr/share/state/printer_rotation","");
    if(buf[0]=='1')
    {
        for(loop=0;loop<rotate_loop;loop++)
        {
            if(rotate_pulse_count==1)
            {
                if(bat_present<84)
                {
                    lp_even_rotate();
                }
                else
                {
                    even_rotate();
                }
                rotate_pulse_count=2;
            }
            else if(rotate_pulse_count==2)
            {
                if(bat_present<84)
                {
                    lp_odd_rotate();
                }
                else
                {
                    odd_rotate();
                }
                rotate_pulse_count=1;
            }
        }
    }
}


//-------------------------------The program starts from here------------------------------------------

static void printer_prepare_spi_message(void)
{

    gpio_direction_output(137,1);
    /***************Rotate Status Check***************/

    memset(buf,0x00,128);

    f1 = filp_open("/sys/class/gpio/gpio162/value", O_RDONLY, 0);
    new = get_fs();
    set_fs(get_ds());
    f1->f_op->read(f1, buf, 1, &f1->f_pos);
    set_fs(new);
    filp_close(f1,NULL);

    f_LB = filp_open("/sys/class/power_supply/NUC970Bat/present", O_RDONLY,0);
    new = get_fs();
    set_fs(get_ds());
    f_LB->f_op->read(f_LB, LB_buf1, 2, &f_LB->f_pos);
    set_fs(new);
    filp_close(f_LB,NULL);

    bat_present = strtoint(LB_buf1);

    //    printk("Battery Percentage: %d\n", bat_present);

    /***************Rotate Status Check***************/
    data_read[48]=0,data_read[49]=1,data_read[50]=2,data_read[51]=3,data_read[52]=4,data_read[53]=5,data_read[54]=6,data_read[55]=7,data_read[56]=8,data_read[57]=9,data_read[65]=10,data_read[66]=11,data_read[67]=12,data_read[68]=13,data_read[69]=14,data_read[70]=15;

    paper_Temp=0;
    spi_message_init(&printer_ctl.msg);
    printer_dev.spi_device->bits_per_word=8;

    memset(tmp,0x00,48);

    //	printk(KERN_ALERT "length is ...........   %d  \n",length);

    for(k=0;k<=1;k++)
    {
        g[k]=(**(buff))-32;
        ++(*buff);
    }

    //	paper_sensing();

    //---------------------------------------PROTOCOL starting------------------------------------------

    if(g[0]==94)    //~ -----------start byte of protocol
    {

        paper_Temp=0;

        switch(g[1])
        {

        //********************************** Paper feed **********************************

        case 50: // R - motor rotation

            for(k=2;k<=5;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
            }

            Noofbytes();
            //			printk("No of Bytes = %d\n",Temp);

            for(i=0;i<Temp;i++)
            {
                empty_rotate();
            }
            Temp=0;
            break;

            //******************** ENGLISH PRINTING *************************

        case 37:  //E ----------------------english printing  g[1]

            no_of_lines=0;

            for(k=2;k<=5;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
                //				printk("g[%d] = %d\n",k,g[k]);
            }

            Noofbytes();

            //			printk("No of Bytes = %d\n",Temp);

            for(k=6;k<=Temp;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
                if(g[k]==94)
                {
                    no_of_lines++;
                }
            }

            if(g[Temp-1]==1)
            {
                //				printk("last byte is received...\n");
                buffer_check1=6;

                for(line_wise=0;line_wise<no_of_lines;line_wise++)
                {
                    data_size=1;
                    do
                    {
                        data[data_size]=g[buffer_check1];
                        data_size++;
                        buffer_check1++;
                    }while(g[buffer_check1]!=94);

                    buffer_check1=buffer_check1+1;
                    data_length=data_size;
                    //				printk("Data_length=%d\n",data_length);

                    even=0;
                    odd=1;

                    //---------------------------------------------------------------------------------------------
                    //if((data[data_size-2] > 32) && (data[data_size-2] < 59))
                    if(g[Temp-2]==17)
                    {
                        fontstyle=1;
                        //printk("FontStyle=1\n");
                    }
                    else if(g[Temp-2]==18)
                        //else if((data[data_size-2] > 62) && (data[data_size-2] < 91))
                    {
                        //printk("FontStyle=2\n");
                        fontstyle=2;
                    }
                    else if(g[Temp-2]==19)
                        //else if((data[data_size-2] > 62) && (data[data_size-2] < 91))
                    {
                        //printk("FontStyle=3\n");
                        fontstyle=3;
                    }
                    if(data[data_size-1]==44 || data[data_size-1]== 76)  // left
                    {
                        allignment=1;
                        //printk("Allignment=1\n");
                    }
                    else if(data[data_size-1]==50 || data[data_size-1]== 82)
                    {
                        allignment=2;
                        //printk("Allignment=2\n");
                    }
                    else if(data[data_size-1]==35 || data[data_size-1]== 67)
                    {
                        allignment=3;
                        //printk("Allignment=3\n");
                    }

                    if(data[data_size-3]==50 || data[data_size-3]==82)  // Regular
                    {
                        font=1;
                        //printk("Font=1\n");
                    }
                    else if(data[data_size-3]==34 || data[data_size-3]== 66)  // Bold
                    {
                        font=2;
                        //printk("Font=2\n");
                    }
                    else if(data[data_size-3]==41 || data[data_size-3]== 73)  // Italic
                    {
                        font=3;
                        //printk("Font=3\n");
                    }
                    if(data[data_size-2]==52 || data[data_size-2]==84)  // small
                    {
                        size=1;
                        //printk("Tiny font\n");
                        if(data_size>46)
                        {
                            data_size=46;
                        }
                    }
                    if(data[data_size-2]==51 || data[data_size-2]==83)  // small
                    {
                        size=2;
                        //printk("Smallfont\n");
                        if(data_size>42)
                        {
                            data_size=42;
                        }
                    }
                    else if(data[data_size-2]==45 || data[data_size-2]==77)  //medium
                    {
                        size=3;
                        //printk("Mediumfont\n");
                        if(data_size>36)
                        {
                            data_size=36;
                        }
                    }
                    else if(data[data_size-2]==44 || data[data_size-2]==76)  //large
                    {
                        size=4;
                        //printk("Largefont\n");
                        if(data_size>31)
                        {
                            data_size=31;
                        }
                    }

                    // Tiny font - starts
                    if(size==1)
                    {
                        memset(addr,0x00,sizeof(addr));
                        for(height=0;height<13;height++)
                        {
                            for(width=1;width<=50 && width<(data_size-3);width++)
                            {
                                if(font==1)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=Unispace7reg[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono8reg[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=Consolas8reg[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=Unispace7reg[data[width]][height];
                                        break;
                                    }

                                }
                                else if(font==2)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=Unispace7bold[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono8bold[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=Consolas8bold[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=Unispace7bold[data[width]][height];
                                        break;
                                    }
                                }

                            }

                            // 8
                            int count_2,pixel_array_count=0,temp_var=0, char_pixel_data, bit_position=0;
                            int width_s=8;
                            for(i=1;i<43;i++)
                            {
                                char_pixel_data=envy[i];
                                for(count_2 = 0; count_2 < width_s; count_2++)
                                {
                                    if (char_pixel_data & 0x8000)
                                        temp_var |= 0x01;
                                    else
                                        temp_var |= 0x00;
                                    bit_position++;
                                    if (bit_position > 7)
                                    {
                                        tmp_buff[pixel_array_count] = temp_var;
                                        temp_var = 0;
                                        bit_position = 0;
                                        pixel_array_count++;
                                    }
                                    char_pixel_data = char_pixel_data << 1;
                                    temp_var = temp_var << 1;
                                }
                            }

                            if(data_length>=46)
                            {
                                data_length=46;
                            }

                            if(allignment==1) // left allignment
                            {
                                for(i=0;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[i];
                                }
                            }

                            else if(allignment==2) // right allignment
                            {
                                start=((data_size-4)*8);
                                start=384-start;
                                start=((start/8));

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }
                            }

                            else if(allignment==3) // center allignment
                            {
                                start=((data_size-4)*8);
                                start=384-start;
                                start=((start/2)+(start%2));
                                start=((start/8));
                                end=((data_size-4)*10);
                                end=((end/8)+(end%8));
                                end=(start+end);

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<end;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }

                                for(i=end;i<48;i++)
                                {
                                    tmp[i]=0;
                                }

                            }


                            //                            memset(tmp,0x00,sizeof(tmp));
                            spi_write(printer_dev.spi_device, addr, 48);
                            rotate(2);
                            //                            memset(addr,0x00,sizeof(addr));

                        }  // height - ends
                    }  // Tiny -ends

                    // Small font - starts
                    if(size==2)
                    {
                        memset(addr,0x00,sizeof(addr));
                        for(height=0;height<17;height++)
                        {
                            for(width=1;width<=41 && width<(data_size-3);width++)
                            {

                                if(font==1)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=Unispace10reg[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono11reg[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=Consolas11reg[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=Unispace10reg[data[width]][height];
                                        break;
                                    }

                                }
                                else if(font==2)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=Unispace10bold[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono11bold[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=Consolas11bold[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=Unispace10bold[data[width]][height];
                                        break;
                                    }
                                }

                            }

                            // 10
                            tmp_buff[0]=((envy[1] & 0xff00) >> 8 );
                            tmp_buff[1]=((envy[1] & 0x00c0) | ((envy[2] & 0xfc00) >> 10));
                            tmp_buff[2]=(((envy[2] & 0x03c0) >> 2 ) | ((envy[3] & 0xf000) >> 12));
                            tmp_buff[3]=(((envy[3] & 0x0fc0) >> 4 ) | ((envy[4] & 0xc000) >> 14));
                            tmp_buff[4]=((envy[4] & 0x3fc0) >> 6 );

                            tmp_buff[5]=((envy[5] & 0xff00) >> 8 );
                            tmp_buff[6]=((envy[5] & 0x00c0) | ((envy[6] & 0xfc00) >> 10));
                            tmp_buff[7]=(((envy[6] & 0x03c0) >> 2 ) | ((envy[7] & 0xf000) >> 12));
                            tmp_buff[8]=(((envy[7] & 0x0fc0) >> 4 ) | ((envy[8] & 0xc000) >> 14));
                            tmp_buff[9]=((envy[8] & 0x3fc0) >> 6 );

                            tmp_buff[10]=((envy[9] & 0xff00) >> 8 );
                            tmp_buff[11]=((envy[9] & 0x00c0) | ((envy[10] & 0xfc00) >> 10));
                            tmp_buff[12]=(((envy[10] & 0x03c0) >> 2 ) | ((envy[11] & 0xf000) >> 12));
                            tmp_buff[13]=(((envy[11] & 0x0fc0) >> 4 ) | ((envy[12] & 0xc000) >> 14));
                            tmp_buff[14]=((envy[12] & 0x3fc0) >> 6 );

                            tmp_buff[15]=((envy[13] & 0xff00) >> 8 );
                            tmp_buff[16]=((envy[13] & 0x00c0) | ((envy[14] & 0xfc00) >> 10));
                            tmp_buff[17]=(((envy[14] & 0x03c0) >> 2 ) | ((envy[15] & 0xf000) >> 12));
                            tmp_buff[18]=(((envy[15] & 0x0fc0) >> 4 ) | ((envy[16] & 0xc000) >> 14));
                            tmp_buff[19]=((envy[16] & 0x3fc0) >> 6 );

                            tmp_buff[20]=((envy[17] & 0xff00) >> 8 );
                            tmp_buff[21]=((envy[17] & 0x00c0) | ((envy[18] & 0xfc00) >> 10));
                            tmp_buff[22]=(((envy[18] & 0x03c0) >> 2 ) | ((envy[19] & 0xf000) >> 12));
                            tmp_buff[23]=(((envy[19] & 0x0fc0) >> 4 ) | ((envy[20] & 0xc000) >> 14));
                            tmp_buff[24]=((envy[20] & 0x3fc0) >> 6 );

                            tmp_buff[25]=((envy[21] & 0xff00) >> 8 );
                            tmp_buff[26]=((envy[21] & 0x00c0) | ((envy[22] & 0xfc00) >> 10));
                            tmp_buff[27]=(((envy[22] & 0x03c0) >> 2 ) | ((envy[23] & 0xf000) >> 12));
                            tmp_buff[28]=(((envy[23] & 0x0fc0) >> 4 ) | ((envy[24] & 0xc000) >> 14));
                            tmp_buff[29]=((envy[24] & 0x3fc0) >> 6 );

                            tmp_buff[30]=((envy[25] & 0xff00) >> 8 );
                            tmp_buff[31]=((envy[25] & 0x00c0) | ((envy[26] & 0xfc00) >> 10));
                            tmp_buff[32]=(((envy[26] & 0x03c0) >> 2 ) | ((envy[27] & 0xf000) >> 12));
                            tmp_buff[33]=(((envy[27] & 0x0fc0) >> 4 ) | ((envy[28] & 0xc000) >> 14));
                            tmp_buff[34]=((envy[28] & 0x3fc0) >> 6 );

                            tmp_buff[35]=((envy[29] & 0xff00) >> 8 );
                            tmp_buff[36]=((envy[29] & 0x00c0) | ((envy[30] & 0xfc00) >> 10));
                            tmp_buff[37]=(((envy[30] & 0x03c0) >> 2 ) | ((envy[31] & 0xf000) >> 12));
                            tmp_buff[38]=(((envy[31] & 0x0fc0) >> 4 ) | ((envy[32] & 0xc000) >> 14));
                            tmp_buff[39]=((envy[32] & 0x3fc0) >> 6 );

                            tmp_buff[40]=((envy[33] & 0xff00) >> 8 );
                            tmp_buff[41]=((envy[33] & 0x00c0) | ((envy[34] & 0xfc00) >> 10));
                            tmp_buff[42]=(((envy[34] & 0x03c0) >> 2 ) | ((envy[35] & 0xf000) >> 12));
                            tmp_buff[43]=(((envy[35] & 0x0fc0) >> 4 ) | ((envy[36] & 0xc000) >> 14));
                            tmp_buff[44]=((envy[36] & 0x3fc0) >> 6 );

                            tmp_buff[45]=((envy[37] & 0xff00) >> 8 );
                            tmp_buff[46]=((envy[37] & 0x00c0) | ((envy[38] & 0xfc00) >> 10));
                            tmp_buff[47]=(((envy[38] & 0x03c0) >> 2 ) | ((envy[39] & 0xf000) >> 12));
                            tmp_buff[48]=(((envy[39] & 0x0fc0) >> 4 ) | ((envy[40] & 0xc000) >> 14));
                            tmp_buff[49]=((envy[40] & 0x3fc0) >> 6 );


                            if(data_length>=38)
                            {
                                data_length=38;
                            }

                            if(allignment==1) // left allignment
                            {
                                for(i=0;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[i];
                                }
                            }

                            else if(allignment==2) // right allignment
                            {
                                start=((data_size-4)*10);
                                start=384-start;
                                start=((start/8));

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }
                            }

                            else if(allignment==3) // center allignment
                            {
                                start=((data_size-4)*10);
                                start=384-start;
                                start=((start/2)+(start%2));
                                start=((start/8));
                                end=((data_size-4)*10);
                                end=((end/8)+(end%8));
                                end=(start+end);

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<end;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }

                                for(i=end;i<48;i++)
                                {
                                    tmp[i]=0;
                                }

                            }

                            //                            memset(tmp,0x00,sizeof(tmp));
                            spi_write(printer_dev.spi_device, addr, 48);
                            rotate(2);
                            //                            memset(addr,0x00,sizeof(addr));

                        }  // height - ends
                    }  // small -ends




                    // Medium -starts
                    if(size==3)
                    {
                        memset(addr,0x00,sizeof(addr));
                        for(height=0;height<21;height++)
                        {
                            for(width=1;width<=32 && width<(data_size-3);width++)
                            {

                                if(font==1)
                                {

                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=UnispaceMEDIUMreg[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono13reg[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=ConsolasMreg[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=UnispaceMEDIUMreg[data[width]][height];
                                        break;
                                    }

                                }
                                else if(font==2)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=UnispaceMEDIUMbold[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono13bold[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=ConsolasMbold[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=UnispaceMEDIUMbold[data[width]][height];
                                        break;
                                    }

                                }

                            }

                            /// 12
                            tmp_buff[0]=((envy[1] & 0xff00) >> 8 );
                            tmp_buff[1]=((envy[1] & 0x00f0) | ((envy[2] & 0xf000) >> 12));
                            tmp_buff[2]=((envy[2] & 0x0ff0) >> 4);

                            tmp_buff[3]=((envy[3] & 0xff00) >> 8 );
                            tmp_buff[4]=((envy[3] & 0x00f0) | ((envy[4] & 0xf000) >> 12));
                            tmp_buff[5]=((envy[4] & 0x0ff0) >> 4);

                            tmp_buff[6]=((envy[5] & 0xff00) >> 8 );
                            tmp_buff[7]=((envy[5] & 0x00f0) | ((envy[6] & 0xf000) >> 12));
                            tmp_buff[8]=((envy[6] & 0x0ff0) >> 4);

                            tmp_buff[9]=((envy[7] & 0xff00) >> 8 );
                            tmp_buff[10]=((envy[7] & 0x00f0) | ((envy[8] & 0xf000) >> 12));
                            tmp_buff[11]=((envy[8] & 0x0ff0) >> 4);

                            tmp_buff[12]=((envy[9] & 0xff00) >> 8 );
                            tmp_buff[13]=((envy[9] & 0x00f0) | ((envy[10] & 0xf000) >> 12));
                            tmp_buff[14]=((envy[10] & 0x0ff0) >> 4);

                            tmp_buff[15]=((envy[11] & 0xff00) >> 8 );
                            tmp_buff[16]=((envy[11] & 0x00f0) | ((envy[12] & 0xf000) >> 12));
                            tmp_buff[17]=((envy[12] & 0x0ff0) >> 4);

                            tmp_buff[18]=((envy[13] & 0xff00) >> 8 );
                            tmp_buff[19]=((envy[13] & 0x00f0) | ((envy[14] & 0xf000) >> 12));
                            tmp_buff[20]=((envy[14] & 0x0ff0) >> 4);

                            tmp_buff[21]=((envy[15] & 0xff00) >> 8 );
                            tmp_buff[22]=((envy[15] & 0x00f0) | ((envy[16] & 0xf000) >> 12));
                            tmp_buff[23]=((envy[16] & 0x0ff0) >> 4);

                            tmp_buff[24]=((envy[17] & 0xff00) >> 8 );
                            tmp_buff[25]=((envy[17] & 0x00f0) | ((envy[18] & 0xf000) >> 12));
                            tmp_buff[26]=((envy[18] & 0x0ff0) >> 4);

                            tmp_buff[27]=((envy[19] & 0xff00) >> 8 );
                            tmp_buff[28]=((envy[19] & 0x00f0) | ((envy[20] & 0xf000) >> 12));
                            tmp_buff[29]=((envy[20] & 0x0ff0) >> 4);

                            tmp_buff[30]=((envy[21] & 0xff00) >> 8 );
                            tmp_buff[31]=((envy[21] & 0x00f0) | ((envy[22] & 0xf000) >> 12));
                            tmp_buff[32]=((envy[22] & 0x0ff0) >> 4);

                            tmp_buff[33]=((envy[23] & 0xff00) >> 8 );
                            tmp_buff[34]=((envy[23] & 0x00f0) | ((envy[24] & 0xf000) >> 12));
                            tmp_buff[35]=((envy[24] & 0x0ff0) >> 4);

                            tmp_buff[36]=((envy[25] & 0xff00) >> 8 );
                            tmp_buff[37]=((envy[25] & 0x00f0) | ((envy[26] & 0xf000) >> 12));
                            tmp_buff[38]=((envy[26] & 0x0ff0) >> 4);

                            tmp_buff[39]=((envy[27] & 0xff00) >> 8 );
                            tmp_buff[40]=((envy[27] & 0x00f0) | ((envy[28] & 0xf000) >> 12));
                            tmp_buff[41]=((envy[28] & 0x0ff0) >> 4);

                            tmp_buff[42]=((envy[29] & 0xff00) >> 8 );
                            tmp_buff[43]=((envy[29] & 0x00f0) | ((envy[30] & 0xf000) >> 12));
                            tmp_buff[44]=((envy[30] & 0x0ff0) >> 4);

                            tmp_buff[45]=((envy[31] & 0xff00) >> 8 );
                            tmp_buff[46]=((envy[31] & 0x00f0) | ((envy[32] & 0xf000) >> 12));
                            tmp_buff[47]=((envy[32] & 0x0ff0) >> 4);

                            if(data_length>=32)
                            {
                                data_length=32;
                            }

                            if(allignment==1) // left allignment
                            {
                                for(i=0;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[i];
                                }
                            }

                            else if(allignment==2) // right allignment
                            {
                                start=((data_size-4)*12);
                                start=384-start;
                                start=((start/8));

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }
                            }

                            else if(allignment==3) // center allignment
                            {
                                start=((data_size-4)*12);
                                start=384-start;
                                start=((start/2)+(start%2));
                                start=((start/8));
                                end=((data_size-4)*12);
                                end=((end/8)+(end%8));
                                end=(start+end);

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<end;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }

                                for(i=end;i<48;i++)
                                {
                                    tmp[i]=0;
                                }

                            }


                            //                            memset(tmp,0x00,sizeof(tmp));
                            spi_write(printer_dev.spi_device, addr, 48);
                            rotate(3);
                            //                            memset(addr,0x00,sizeof(addr));
                        }  // height - ends
                    }  // medium -ends


                    // Large - starts
                    if(size==4)
                    {
                        memset(addr,0x00,sizeof(addr));
                        for(height=0;height<26;height++)
                        {
                            for(width=1;width<=31 && width<(data_size-3);width++)
                            {

                                if(font==1)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=UnispaceLARGEreg[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono17reg[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=ConsolasLreg[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=UnispaceLARGEreg[data[width]][height];
                                        break;
                                    }

                                }
                                else if(font==2)
                                {
                                    switch(fontstyle){
                                    case 1:
                                        envy[width]=UnispaceLARGEbold[data[width]][height];
                                        break;
                                    case 2:
                                        envy[width]=BitstreamVeraSansMono17bold[data[width]][height];
                                        break;
                                    case 3:
                                        envy[width]=ConsolasLbold[data[width]][height];
                                        break;
                                    default:
                                        envy[width]=UnispaceLARGEbold[data[width]][height];
                                        break;
                                    }
                                }

                            }
                            int count_2,pixel_array_count=0,temp_var=0, char_pixel_data, bit_position=0;
                            int width_s=14;
                            for(i=1;i<28;i++)
                            {
                                char_pixel_data=envy[i];
                                for(count_2 = 0; count_2 < width_s; count_2++)
                                {
                                    if (char_pixel_data & 0x8000)
                                        temp_var |= 0x01;
                                    else
                                        temp_var |= 0x00;
                                    bit_position++;
                                    if (bit_position > 7)
                                    {
                                        tmp_buff[pixel_array_count] = temp_var;
                                        temp_var = 0;
                                        bit_position = 0;
                                        pixel_array_count++;
                                    }
                                    char_pixel_data = char_pixel_data << 1;
                                    temp_var = temp_var << 1;
                                }
                                /*
        if(bit_position <8)
        {
            temp_var = temp_var << (7 - bit_position);
                            tmp_buff[pixel_array_count] = temp_var;
                temp_var = 0;
                bit_position = 0;
        }
*/
                            }
                            /*
// 16

                                        tmp_buff[0]=((envy[1] & 0xff00) >> 8 );
                                        tmp_buff[1]=(envy[1] & 0x00ff);
                                        tmp_buff[2]=((envy[2] & 0xff00) >> 8 );
                                        tmp_buff[3]=(envy[2] & 0x00ff);
                                        tmp_buff[4]=((envy[3] & 0xff00) >> 8 );
                                        tmp_buff[5]=(envy[3] & 0x00ff);
                                        tmp_buff[6]=((envy[4] & 0xff00) >> 8 );
                                        tmp_buff[7]=(envy[4] & 0x00ff);
                                        tmp_buff[8]=((envy[5] & 0xff00) >> 8 );
                                        tmp_buff[9]=(envy[5] & 0x00ff);
                                        tmp_buff[10]=((envy[6] & 0xff00) >> 8 );
                                        tmp_buff[11]=(envy[6] & 0x00ff);
                                        tmp_buff[12]=((envy[7] & 0xff00) >> 8 );
                                        tmp_buff[13]=(envy[7] & 0x00ff);
                                        tmp_buff[14]=((envy[8] & 0xff00) >> 8 );
                                        tmp_buff[15]=(envy[8] & 0x00ff);
                                        tmp_buff[16]=((envy[9] & 0xff00) >> 8 );
                                        tmp_buff[17]=(envy[9] & 0x00ff);
                                        tmp_buff[18]=((envy[10] & 0xff00) >> 8 );
                                        tmp_buff[19]=(envy[10] & 0x00ff);
                                        tmp_buff[20]=((envy[11] & 0xff00) >> 8 );
                                        tmp_buff[21]=(envy[11] & 0x00ff);
                                        tmp_buff[22]=((envy[12] & 0xff00) >> 8 );
                                        tmp_buff[23]=(envy[12] & 0x00ff);
                                        tmp_buff[24]=((envy[13] & 0xff00) >> 8 );
                                        tmp_buff[25]=(envy[13] & 0x00ff);
                                        tmp_buff[26]=((envy[14] & 0xff00) >> 8 );
                                        tmp_buff[27]=(envy[14] & 0x00ff);
                                        tmp_buff[28]=((envy[15] & 0xff00) >> 8 );
                                        tmp_buff[29]=(envy[15] & 0x00ff);
                                        tmp_buff[30]=((envy[16] & 0xff00) >> 8 );
                                        tmp_buff[31]=(envy[16] & 0x00ff);
                                        tmp_buff[32]=((envy[17] & 0xff00) >> 8 );
                                        tmp_buff[33]=(envy[17] & 0x00ff);
                                        tmp_buff[34]=((envy[18] & 0xff00) >> 8 );
                                        tmp_buff[35]=(envy[18] & 0x00ff);
                                        tmp_buff[36]=((envy[19] & 0xff00) >> 8 );
                                        tmp_buff[37]=(envy[19] & 0x00ff);
                                        tmp_buff[38]=((envy[20] & 0xff00) >> 8 );
                                        tmp_buff[39]=(envy[20] & 0x00ff);
                                        tmp_buff[40]=((envy[21] & 0xff00) >> 8 );
                                        tmp_buff[41]=(envy[21] & 0x00ff);
                                        tmp_buff[42]=((envy[22] & 0xff00) >> 8 );
                                        tmp_buff[43]=(envy[22] & 0x00ff);
                                        tmp_buff[44]=((envy[23] & 0xff00) >> 8 );
                                        tmp_buff[45]=(envy[23] & 0x00ff);
                                        tmp_buff[46]=((envy[24] & 0xff00) >> 8 );
                                        tmp_buff[47]=(envy[24] & 0x00ff);
*/

                            /*
                                tmp_buff[0]=((envy[1] & 0xff00) >> 8 );
                                tmp_buff[1]=(envy[1] & 0x00fe) | ((envy[2] & 0xf000) >> 15);
                                tmp_buff[2]=((envy[2] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[3]=(envy[2] & 0x00fe) | ((envy[3] & 0xf000) >> 15);
                                tmp_buff[4]=((envy[3] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[5]=(envy[3] & 0x00fe) | ((envy[4] & 0xf000) >> 15);
                                tmp_buff[6]=((envy[4] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[7]=(envy[4] & 0x00fe) | ((envy[5] & 0xf000) >> 15);
                                tmp_buff[8]=((envy[5] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[9]=(envy[5] & 0x00fe) | ((envy[6] & 0xf000) >> 15);
                                tmp_buff[10]=((envy[6] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[11]=(envy[6] & 0x00fe) | ((envy[7] & 0xf000) >> 15);
                                tmp_buff[12]=((envy[7] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[13]=(envy[7] & 0x00fe) | ((envy[8] & 0xf000) >> 15);
                                tmp_buff[14]=((envy[8] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[15]=(envy[8] & 0x00fe) | ((envy[9] & 0xf000) >> 15);
                                tmp_buff[16]=((envy[9] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[17]=(envy[9] & 0x00fe) | ((envy[10] & 0xf000) >> 15);
                                tmp_buff[18]=((envy[10] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[19]=(envy[10] & 0x00fe) | ((envy[11] & 0xf000) >> 15);
                                tmp_buff[20]=((envy[11] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[21]=(envy[11] & 0x00fe) | ((envy[12] & 0xf000) >> 15);
                                tmp_buff[22]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[23]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[24]=((envy[13] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[25]=(envy[13] & 0x00fe) | ((envy[14] & 0xf000) >> 15);
                                tmp_buff[26]=((envy[14] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[27]=(envy[14] & 0x00fe) | ((envy[15] & 0xf000) >> 15);
                                tmp_buff[28]=((envy[15] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[29]=(envy[15] & 0x00fe) | ((envy[16] & 0xf000) >> 15);
                                tmp_buff[30]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[31]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[32]=((envy[13] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[33]=(envy[11] & 0x00fe) | ((envy[12] & 0xf000) >> 15);
                                tmp_buff[34]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[35]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[36]=((envy[13] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[37]=(envy[11] & 0x00fe) | ((envy[12] & 0xf000) >> 15);
                                tmp_buff[38]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[39]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[40]=((envy[13] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[41]=(envy[11] & 0x00fe) | ((envy[12] & 0xf000) >> 15);
                                tmp_buff[42]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[43]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[44]=((envy[13] & 0x7f00) << 1 ) >> 8;

                                tmp_buff[45]=(envy[11] & 0x00fe) | ((envy[12] & 0xf000) >> 15);
                                tmp_buff[46]=((envy[12] & 0x7f00) << 1 ) >> 8;
                                tmp_buff[47]=(envy[12] & 0x00fe) | ((envy[13] & 0xf000) >> 15);
                                tmp_buff[48]=((envy[13] & 0x7f00) << 1 ) >> 8;

*/		
                            if(data_length>=31)
                            {
                                data_length=31;
                            }

                            if(allignment==1) // left allignment
                            {
                                for(i=0;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[i];
                                }
                            }

                            else if(allignment==2) // right allignment
                            {
                                start=((data_size-4)*16);
                                start=384-start;
                                start=((start/8));

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<48;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }
                            }

                            else if(allignment==3) // center allignment
                            {
                                start=((data_size-4)*16);
                                start=384-start;
                                start=((start/2)-(start%2));
                                start=((start/8));
                                end=((data_size-4)*16);
                                end=((end/8)+(end%8));
                                end=(start+end);

                                for(i=0;i<start;i++)
                                {
                                    tmp[i]=0;
                                }

                                j=0;

                                for(i=start;i<end;i++)
                                {
                                    tmp[i]=tmp_buff[j];
                                    j++;
                                }

                                for(i=end;i<48;i++)
                                {
                                    tmp[i]=0;
                                }

                            }
                            spi_write(printer_dev.spi_device, addr, 48);
                            rotate(3);
                            //                            memset(addr,0,sizeof(addr));
                            //                            memset(tmp,0,sizeof(tmp));
                        }  // height - ends
                    }  // Large -ends

                    memset(tmp,0x00,sizeof(tmp));
                    spi_write(printer_dev.spi_device, addr, 48);
                    rotate(2);
                    //                    memset(addr,0x00,sizeof(addr));
                }
            }

            Temp=0;
            break;

            // --------------------- IMAGE PRINTING ----------------------------

        case 41: // I - Image printing


            for(k=2;k<=6;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
            }

            low_bat=0;

            Noofbytes();
            //printk("No of Bytes = %d\n",Temp);

            memset(addr,0x00,48);
            memset(tmp,0x00,48);
            memset(buf1,0x00,1000000);

            f = filp_open("/usr/share/status/PRINTER_image", O_RDONLY, 0);

            if(f == NULL)
            {
                printk(KERN_ALERT "filp_open error!!.\n");
            }
            else
            {
                fs = get_fs();
                set_fs(get_ds());
                f->f_op->read(f, buf1, 1000000, &f->f_pos);
                //                vfs_read(f, buf1, 1000000, &f->f_pos);
                set_fs(fs);
                value=buf1;
                filp_close(f, NULL);
            }

            even=0;
            odd=1;

            for(x=0;x<Temp;x++)
            {
                for(i=0;i<49;i++)
                {
                    if((*value)!=10)
                    {
                        data1=*value;
                        value++;
                        y++;
                        data2=*value;
                        value++;
                        y++;

                        data1=data_read[data1];
                        data2=data_read[data2];

                        data3=(((data1&0x0f)<<4) | (data2&0x0f));
                        w=7;
                        q=0;

                        while(w>=0)
                        {
                            temp_image |= (((data3 >> q)&1)<< w);
                            w--;
                            q++;
                        }
                        tmp[i]=temp_image;
                        temp_image=0;

                    }

                    else
                    {
                        i=49;
                        value++;
                    }

                }
                spi_write(printer_dev.spi_device, addr, 48);
                rotate(1);
                //                memset(addr,0x00,sizeof(addr));
                //                memset(tmp,0x00,sizeof(tmp));
            }
            //            memset(addr,0x00,sizeof(addr));
            memset(tmp,0x00,sizeof(tmp));
            spi_write(printer_dev.spi_device, addr, 48);

            rotate(5);

            Temp=0;
            paper_Temp=0;

            break;
            /**************************Image Header Printer*******************/
        case 40: // I - Image printing


            for(k=2;k<=6;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
            }

            low_bat=0;

            Noofbytes();
            //printk("No of Bytes = %d\n",Temp);

            memset(addr,0x00,48);
            memset(tmp,0x00,48);
            memset(buf1,0x00,1000000);

            f = filp_open("/usr/share/status/PRINTER_header", O_RDONLY, 0);

            if(f == NULL)
            {
                printk(KERN_ALERT "filp_open error!!.\n");
            }
            else
            {
                fs = get_fs();
                set_fs(get_ds());
                f->f_op->read(f, buf1, 1000000, &f->f_pos);
                //                vfs_read(f, buf1, 1000000, &f->f_pos);
                set_fs(fs);
                value=buf1;
                filp_close(f, NULL);
            }

            even=0;
            odd=1;

            for(x=0;x<Temp;x++)
            {
                for(i=0;i<49;i++)
                {
                    if((*value)!=10)
                    {
                        data1=*value;
                        value++;
                        y++;
                        data2=*value;
                        value++;
                        y++;

                        data1=data_read[data1];
                        data2=data_read[data2];

                        data3=(((data1&0x0f)<<4) | (data2&0x0f));
                        w=7;
                        q=0;

                        while(w>=0)
                        {
                            temp_image |= (((data3 >> q)&1)<< w);
                            w--;
                            q++;
                        }
                        tmp[i]=temp_image;
                        temp_image=0;

                    }

                    else
                    {
                        i=49;
                        value++;
                    }

                }
                spi_write(printer_dev.spi_device, addr, 48);
                rotate(1);
                //                memset(addr,0x00,sizeof(addr));
                //                memset(tmp,0x00,sizeof(tmp));
            }
            //            memset(addr,0x00,sizeof(addr));
            memset(tmp,0x00,sizeof(tmp));
            spi_write(printer_dev.spi_device, addr, 48);

            rotate(5);

            Temp=0;
            paper_Temp=0;

            break;
            /*****************************Footer Image Printing****************************/
        case 38: // I - Image printing


            for(k=2;k<=6;k++)
            {
                g[k]=(**(buff))-32;
                ++(*buff);
            }

            low_bat=0;

            Noofbytes();
            //printk("No of Bytes = %d\n",Temp);

            memset(addr,0x00,48);
            memset(tmp,0x00,48);
            memset(buf1,0x00,1000000);

            f = filp_open("/usr/share/status/PRINTER_footer", O_RDONLY, 0);

            if(f == NULL)
            {
                printk(KERN_ALERT "filp_open error!!.\n");
            }
            else
            {
                fs = get_fs();
                set_fs(get_ds());
                f->f_op->read(f, buf1, 1000000, &f->f_pos);
                //                vfs_read(f, buf1, 1000000, &f->f_pos);
                set_fs(fs);
                value=buf1;
                filp_close(f, NULL);
            }

            even=0;
            odd=1;

            for(x=0;x<Temp;x++)
            {
                for(i=0;i<49;i++)
                {
                    if((*value)!=10)
                    {
                        data1=*value;
                        value++;
                        y++;
                        data2=*value;
                        value++;
                        y++;

                        data1=data_read[data1];
                        data2=data_read[data2];

                        data3=(((data1&0x0f)<<4) | (data2&0x0f));
                        w=7;
                        q=0;

                        while(w>=0)
                        {
                            temp_image |= (((data3 >> q)&1)<< w);
                            w--;
                            q++;
                        }
                        tmp[i]=temp_image;
                        temp_image=0;

                    }

                    else
                    {
                        i=49;
                        value++;
                    }

                }
                spi_write(printer_dev.spi_device, addr, 48);
                rotate(1);
                //                memset(addr,0x00,sizeof(addr));
                //                memset(tmp,0x00,sizeof(tmp));
            }
            //            memset(addr,0x00,sizeof(addr));
            memset(tmp,0x00,sizeof(tmp));
            spi_write(printer_dev.spi_device, addr, 48);

            rotate(5);

            Temp=0;
            paper_Temp=0;

            break;
        }
    }
    memset(addr,0,sizeof(addr));
    memset(tmp,0,sizeof(tmp));

    memset(printer_ctl.tx_buff, 0, SPI_BUFF_SIZE);
    spi_message_add_tail(&printer_ctl.transfer, &printer_ctl.msg);

    gpio_direction_output(137,0);
    
}




//---------------------------------------SPI system calls------------------------------------------//

static ssize_t printer_write(struct file* F, const char *buf[], size_t count, loff_t *f_pos)
{

    //printk(KERN_ALERT "count is ...........   %d  \n",count);
    //printk(KERN_ALERT "*buf is ...........   %d  \n",*buf);
    //printk(KERN_ALERT "buf+1  is ...........   %d  \n",*(buf+1));
    //printk(KERN_ALERT "checking ...........   %s  \n",buf);

    buff=&buf;
    length=count;



    ssize_t status = 0;

    if (down_interruptible(&printer_dev.spi_sem))
        return -ERESTARTSYS;

    if (!printer_dev.spi_device)
    {		up(&printer_dev.spi_sem);
        return -ENODEV;
    }

    printer_prepare_spi_message();

    status = spi_sync(printer_dev.spi_device, &printer_ctl.msg);
    up(&printer_dev.spi_sem);


    if ( copy_from_user(printer_dev.user_buff, buf, count) )
    {
        return -EFAULT;
    }

    if (status)
    {
        sprintf(printer_dev.user_buff,
                "printer_do_one_message failed : %d\n",
                status);

    }
    else
    {
        sprintf(printer_dev.user_buff,
                "Status: %d\n printing ..........  \n");
    }

    up(&printer_dev.fop_sem);
    return count;
}

//------------------------
static ssize_t printer_read(struct file *filp, char __user *buff, size_t count,
                            loff_t *offp)
{

    size_t len;
    ssize_t status = 0;

    if (!buff)
        return -EFAULT;

    if (*offp > 0)
        return 0;

    if (down_interruptible(&printer_dev.fop_sem))
        return -ERESTARTSYS;
    /*

        status = printer_do_one_message();

        if (status)
        {
                sprintf(printer_dev.user_buff,
                        "printer_do_one_message failed : %d\n",
                        status);
        }
        else
        {
                sprintf(printer_dev.user_buff,
                        "Status: %d\n printing ..........  \n");
        }

*/
    len = strlen(printer_dev.user_buff);

    if (len < count)
        count = len;

    if (copy_to_user(buff, printer_dev.user_buff, count))  {
        printk(KERN_ALERT "printer_read(): copy_to_user() failed\n");
        status = -EFAULT;
    } else {
        *offp += count;
        status = count;
    }

    up(&printer_dev.fop_sem);
    return status;
}

//---------------------
static int printer_open(struct inode *inode, struct file *filp)
{	
    int status = 0;

    if (down_interruptible(&printer_dev.fop_sem))
        return -ERESTARTSYS;

    if (!printer_dev.user_buff)
    {
        printer_dev.user_buff = kmalloc(USER_BUFF_SIZE, GFP_KERNEL);
        if (!printer_dev.user_buff)
            status = -ENOMEM;
    }

    up(&printer_dev.fop_sem);

    return status;
}

//------------------
static int printer_probe(struct spi_device *spi_device)
{
    if (down_interruptible(&printer_dev.spi_sem))
        return -EBUSY;

    printer_dev.spi_device = spi_device;

    up(&printer_dev.spi_sem);

    return 0;
}

//--------------------
static int printer_remove(struct spi_device *spi_device)
{
    if (down_interruptible(&printer_dev.spi_sem))
        return -EBUSY;

    printer_dev.spi_device = NULL;

    up(&printer_dev.spi_sem);

    return 0;
}

//--------------------
static int __init add_printer_device_to_bus(void)
{
    struct spi_master *spi_master;
    struct spi_device *spi_device;
    struct device *pdev;
    char buff[64];
    int status = 0;

    spi_master = spi_busnum_to_master(SPI_BUS);
    if (!spi_master) {
        printk(KERN_ALERT "spi_busnum_to_master(%d) returned NULL\n",
               SPI_BUS);
        printk(KERN_ALERT "Missing modprobe omap2_mcspi?\n");
        return -1;
    }

    spi_device = spi_alloc_device(spi_master);
    if (!spi_device) {
        put_device(&spi_master->dev);
        printk(KERN_ALERT "spi_alloc_device() failed\n");
        return -1;
    }

    spi_device->chip_select = SPI_BUS_CS1;

    /* Check whether this SPI bus.cs is already claimed */
    snprintf(buff, sizeof(buff), "%s.%u",
             dev_name(&spi_device->master->dev),
             spi_device->chip_select);

    pdev = bus_find_device_by_name(spi_device->dev.bus, NULL, buff);
    if (pdev) {
        /* We are not going to use this spi_device, so free it */
        spi_dev_put(spi_device);

        /*
                 * There is already a device configured for this bus.cs
                 * It is okay if it us, otherwise complain and fail.
                 */
        if (pdev->driver && pdev->driver->name &&
                strcmp(this_driver_name, pdev->driver->name)) {
            printk(KERN_ALERT
                   "Driver [%s] already registered for %s\n",
                   pdev->driver->name, buff);
            status = -1;
        }
    } else {
        spi_device->max_speed_hz = SPI_BUS_SPEED;
        //        spi_device->mode = SPI_MODE_3;
        spi_device->mode = 0x00; //Chagned by gokul
        spi_device->bits_per_word = 8;
        spi_device->irq = -1;
        spi_device->controller_state = NULL;
        spi_device->controller_data = NULL;
        strlcpy(spi_device->modalias, this_driver_name, SPI_NAME_SIZE);

        status = spi_add_device(spi_device);
        if (status < 0) {
            spi_dev_put(spi_device);
            printk(KERN_ALERT "spi_add_device() failed: %d\n",
                   status);
        }
    }

    put_device(&spi_master->dev);

    return status;
}

//--------------------
static struct spi_driver printer_driver = {
    .driver = {
        .name =	this_driver_name,
        .owner = THIS_MODULE,
    },
    .probe = printer_probe,
    .remove =printer_remove,

};

//---------------------
static int __init printer_init_spi(void)       // initializing the spi
{
    int error;

    printer_ctl.tx_buff = kmalloc(SPI_BUFF_SIZE, GFP_KERNEL | GFP_DMA);
    if (!printer_ctl.tx_buff) {
        error = -ENOMEM;
        goto printer_init_error;
    }

    printer_ctl.rx_buff = kmalloc(SPI_BUFF_SIZE, GFP_KERNEL | GFP_DMA);
    if (!printer_ctl.rx_buff) {
        error = -ENOMEM;
        goto printer_init_error;
    }

    error = spi_register_driver(&printer_driver);
    if (error < 0) {
        printk(KERN_ALERT "spi_register_driver() failed %d\n", error);
        goto printer_init_error;
    }

    error = add_printer_device_to_bus();
    if (error < 0) {
        printk(KERN_ALERT "add_printer_to_bus() failed\n");
        spi_unregister_driver(&printer_driver);
        goto printer_init_error;
    }

    return 0;

printer_init_error:

    if (printer_ctl.tx_buff) {
        kfree(printer_ctl.tx_buff);
        printer_ctl.tx_buff = 0;
    }

    if (printer_ctl.rx_buff) {
        kfree(printer_ctl.rx_buff);
        printer_ctl.rx_buff = 0;
    }

    return error;
}

//-----------------
static const struct file_operations printer_fops = {
    .owner =	THIS_MODULE,
    .read = 	printer_read,
    .open =		printer_open,
    .write =        printer_write,
};

//-----------------
static int __init printer_init_cdev(void)      //function to create device node
{
    int error;
    printer_dev.devt = MKDEV(0, 0);

    error = alloc_chrdev_region(&printer_dev.devt, 0, 1, this_driver_name);
    if (error < 0) {
        printk(KERN_ALERT "alloc_chrdev_region() failed: %d \n",
               error);
        return -1;
    }

    cdev_init(&printer_dev.cdev, &printer_fops);
    printer_dev.cdev.owner = THIS_MODULE;

    error = cdev_add(&printer_dev.cdev, printer_dev.devt, 1);
    if (error) {
        printk(KERN_ALERT "cdev_add() failed: %d\n", error);
        unregister_chrdev_region(printer_dev.devt, 1);
        return -1;
    }

    return 0;
}

//-------------------
static int __init printer_init_class(void)          //creating class
{
    printer_dev.class = class_create(THIS_MODULE, this_driver_name);

    if (!printer_dev.class) {
        printk(KERN_ALERT "class_create() failed\n");
        return -1;
    }

    if (!device_create(printer_dev.class, NULL, printer_dev.devt, NULL,
                       this_driver_name)) {
        printk(KERN_ALERT "device_create(..., %s) failed\n",
               this_driver_name);
        class_destroy(printer_dev.class);
        return -1;
    }
    return 0;
}

//---------------------
static int __init printer_init(void)   
{
    memset(&printer_dev, 0, sizeof(printer_dev));   //allocating memory for printer_dev
    memset(&printer_ctl, 0, sizeof(printer_ctl));

    sema_init(&printer_dev.spi_sem, 1);      //semaphore initialization function
    sema_init(&printer_dev.fop_sem, 1);

    if (printer_init_cdev() < 0)       //function to create device nodes
        goto fail_1;

    if (printer_init_class() < 0)    //function to create class
        goto fail_2;

    if (printer_init_spi() < 0)      //function to initialize spi
        goto fail_3;

    //--------------------------- GPIO ----------------------------------------//

    gpio_request(130,"motor_line1");      // requesting the pin48 of gpio
    gpio_export(130,true);                // exporting it to the sysfs entry

    gpio_request(131,"motor_line2");
    gpio_export(131,true);

    gpio_request(132,"motor_line3");     // requesting the pin60 of gpio
    gpio_export(132,true);               // exporting it to the sysfs entry

    gpio_request(133,"motor_line4");
    gpio_export(133,true);

    gpio_request(226,"logic power");
    gpio_export(226,true);

    gpio_request(137,"Driver IC enable");
    gpio_export(137,true);

    gpio_request(227,"spi latch");
    gpio_export(227,true);

    gpio_request(134,"strobe1&2");
    gpio_export(134,true);

    gpio_request(135,"strobe3&4");
    gpio_export(135,true);

    gpio_request(136,"strobe5&6");
    gpio_export(136,true);

    gpio_direction_output(227,0); // ------------ SPI latch default---------
    gpio_direction_output(226,1); // --------------logic power -----------
    gpio_direction_output(137,0); // --------------Driver IC enable -----------

    return 0;

fail_3:
    device_destroy(printer_dev.class, printer_dev.devt);
    class_destroy(printer_dev.class);

fail_2:
    cdev_del(&printer_dev.cdev);
    unregister_chrdev_region(printer_dev.devt, 1);

fail_1:
    return -1;
}

//--------------------------------
static void __exit printer_exit(void)    // exit function to free all the resources 
{
    spi_unregister_device(printer_dev.spi_device);
    spi_unregister_driver(&printer_driver);

    device_destroy(printer_dev.class, printer_dev.devt);
    class_destroy(printer_dev.class);

    cdev_del(&printer_dev.cdev);
    unregister_chrdev_region(printer_dev.devt, 1);

    if (printer_ctl.tx_buff)
        kfree(printer_ctl.tx_buff);

    if (printer_ctl.rx_buff)
        kfree(printer_ctl.rx_buff);

    if (printer_dev.user_buff)
        kfree(printer_dev.user_buff);

    gpio_direction_output(226,0);// --------------logic power -----------
    gpio_direction_output(137,0);// --------------Driver IC enable -----------
}
//----------------------------------


module_init(printer_init);  // Driver always starts execution from here ( insmod ./printer.ko)
module_exit(printer_exit);  // Driver exectues this function while exit  (rmmod  printer.ko)           

MODULE_AUTHOR("Elango & SriNavamani");
MODULE_DESCRIPTION("printer module - SPI driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");


