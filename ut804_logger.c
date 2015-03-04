/*
* recieve measurment results from UT804 (digital multimeter manufactured by UNI-T Group Limited)
* https://github.com/tmatejuk/ut804_linux_logger.git
*
*/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <ctype.h>   /* for isprint */
#include <stdlib.h>  /* for atoi */
#include <time.h>    /* time handling */

#define MAX_FILE_SIZE_UT804 11

void display_usage(void)
{
	printf("how to use this program:\n");
	printf("run program: ut804_logger -d device \n");
	printf("program options:\n");
	printf("\t\t -d device_interface_name  	- device interface name like /dev/ttyUSB0 \n");
	printf("\t\t -h - this message\n\n");
}

void display_gnu(void)
{
	printf("ut804_logger  Copyright (C) 2013 MXN, katosheen. ");
	printf("This program comes with ABSOLUTELY NO WARRANTY. \n");
	printf("This is free software, and you are welcome to redistribute it under certain conditions, ");
	printf("GNU General Public License <http://www.gnu.org/licenses/>\n\n");
}

int open_port_ut804(char *device) //open and set port for UT804
{
	int fd; 
	struct termios options;

	fd = open(device, O_RDONLY | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		printf("open_port_ut804: Unable to open %s\n",device);
		return (fd);
	}
	else
	{
		printf("open_port_ut804: Open %s\n",device);
		fcntl(fd, F_SETFL, 0);
	}
	
	tcgetattr(fd, &options);		/* Get the current options for the port...  */
	cfsetispeed(&options, B2400);		/* Set the baud rates to 2400...  */
	cfsetospeed(&options, B2400);
	options.c_cflag |= (CLOCAL | CREAD);	/* Enable the receiver and set local mode...  */

	//Odd parity (7O1)
	options.c_cflag |= PARENB;
	options.c_cflag |= PARODD;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS7;

	options.c_cflag &= ~CRTSCTS;			//options.c_cflag &= ~CNEW_RTSCTS;
	options.c_lflag |= (ICANON | ECHO | ECHOE);	//Choosing Canonical Input
	options.c_iflag |= IGNPAR ;			//Setting Input Parity Options , IGNPAR - Ignore parity errors
	options.c_iflag |= (IXON | IXOFF | IXANY);	//Software flow control is enabled using the IXON, IXOFF, and IXANY constants
	options.c_oflag &= ~OPOST;			//Raw output is selected by resetting the OPOST option in the c_oflag member
	tcsetattr(fd, TCSANOW, &options);		//Set the new options for the port... TCSANOW the change occurs immediately.

	return (fd);
}

void clean_buffer_ut804(unsigned char *buffer_ut804)
{
	buffer_ut804[0]=0;
	buffer_ut804[1]=0;
	buffer_ut804[2]=0;
	buffer_ut804[3]=0;
	buffer_ut804[4]=0;
	buffer_ut804[5]=0;
	buffer_ut804[6]=0;
	buffer_ut804[7]=0;
	buffer_ut804[8]=0;
	buffer_ut804[9]=0;
	buffer_ut804[10]=0;
}

//void display_buffer_ut804(unsigned char *buffer_ut804)
void display_buffer_ut804(unsigned char buffer_ut804[MAX_FILE_SIZE_UT804])
{
	//int buffer[6];
	//float value;

	switch ( buffer_ut804[6]-0x30 )
	{
		case  1:	//V=
			if((buffer_ut804[8]-0x30)==5)
				printf("-");
			else
				printf("+");
		case  2:	//V~
			switch ( buffer_ut804[5]-0x30 )
			{
				case  1: //4
					printf("%X", buffer_ut804[0]-0x30);
					printf(".");
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" V ");
				break;
				case  2: //40
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" V ");
				break;
				case  3: //400
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" V ");
				break;
				case  4: //4000
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf(".");
					printf("%X", buffer_ut804[4]-0x30);
					printf(" V ");
				break;
				default: //error
				break;
			}
			break;
		case  3:	//mV
			if((buffer_ut804[8]-0x30)==4)
				printf(" - ");
			else
				printf(" + ");
					
			switch ( buffer_ut804[5]-0x30 )
			{
				case  0: //400m
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" mV");
					break;
				default: //error
					break;
			}
			break;
		case  4:	//Ohm, first column is real data, second colum is in kohm
			if( (buffer_ut804[0]-0x30)>9 )
			{
				printf("   .0L Mohm");
				printf("\t");
				printf("  .0L kohm");
			}
			else
			{
				switch ( buffer_ut804[5]-0x30 )
				{
					case  1: //400ohm
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" ohm");
						printf("\t");
						printf("0");
						printf(".");
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					case  2: //4k
						printf("%X", buffer_ut804[0]-0x30);
						printf(".");
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						printf("\t");
						printf("%X", buffer_ut804[0]-0x30);
						printf(".");
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					case  3: //40k
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf(".");
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						printf("\t");
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf(".");
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					case  4: //400k
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						printf("\t");
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					case  5: //4M
						printf("%X", buffer_ut804[0]-0x30);
						printf(".");
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" Mohm");
						printf("\t");
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf(".");
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					case  6: //40M
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf(".");
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" Mohm");
						printf("\t");
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" kohm");
						break;
					default: //error
						break;
				}
			}
			break;
		case  5:	//F
			if( (buffer_ut804[0]-0x30)>9 )
			{
				printf("   .0L nF");
			}
			else
			{
				switch ( buffer_ut804[5]-0x30 )
				{
					case  1: //40n
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf(".");
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" nF");
						break;
					case  2: //400n
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" nF");
						break;
					case  3: //4u
						printf("%X", buffer_ut804[0]-0x30);
						printf(".");
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" uF");
						break;
					case  4: //40u
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf(".");
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" uF");
						break;
					case  5: //400u
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" uF");
						break;
					case  6: //4m
						printf("%X", buffer_ut804[0]-0x30);
						printf(".");
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" mF");
						break;
					default: //error
						break;
				}
			}
			break;

		case  6:	//C
			if( (buffer_ut804[0]-0x30)>9 )
			{
				printf("   0L. C");
			}
			else
			{
				switch ( buffer_ut804[5]-0x30 )
				{
					case  0: //1000C
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf(".");
						printf("%X", buffer_ut804[4]-0x30);
						printf(" C");
						break;
					default: //error
						break;
				}
			}
			break;

		case  7:	//uA
			if((buffer_ut804[8]-0x30)==1)
				printf(" + ");
			else
				printf(" - ");
	
			switch ( buffer_ut804[5]-0x30 )
			{
				case  0: //400u
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" uA");
				break;
				case  1: //4000u
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf(".");
					printf("%X", buffer_ut804[4]-0x30);
					printf(" uA");
				break;
				default: //error
				break;
			}

			break;
		case  8:	//mA
			if((buffer_ut804[8]-0x30)==1)
				printf(" + ");
			else
				printf(" - ");
	
			switch ( buffer_ut804[5]-0x30 )
			{
				case  0: //40m
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" mA");
				break;
				case  1: //400m
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" mA");
				break;
				default: //error
				break;
			}

			break;

		case  9:	//A
			if((buffer_ut804[8]-0x30)==1)
				printf(" + ");
			else
				printf(" - ");
	
			switch ( buffer_ut804[5]-0x30 )
			{
				case  1: //40
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" A");
				break;
				default: //error
				break;
			}

			break;

		case  10: //0x3A:	//pieps
			if( (buffer_ut804[0]-0x30)==10 )
			{
				printf("   0.L silent");
			}
			else
			{
				switch ( buffer_ut804[5]-0x30 )
				{
					case  0: //400ohm
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf(".");
						printf("%X", buffer_ut804[3]-0x30);
						printf("%X", buffer_ut804[4]-0x30);
						printf(" ohm beep");
						break;
					default: //error
						break;
				}
			}
	
			break;

		case  11: //0x3B:	//diode
			printf(" diode - ");

			break;

		case  12: //0x3C:	//Hz
			switch ( buffer_ut804[5]-0x30 )
			{
				case  0: //40
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" Hz");
					break;
				case  1: //400
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" Hz");
					break;
				case  2: //4k
					printf("%X", buffer_ut804[0]-0x30);
					printf(".");
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" kHz");
					break;
				case  3: //40k
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" kHz");
					break;
				case  4: //400k
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" kHz");
					break;
				case  5: //4M
					printf("%X", buffer_ut804[0]-0x30);
					printf(".");
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" MHz");
					break;
				case  6: //40M
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf(".");
					printf("%X", buffer_ut804[2]-0x30);
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" MHz");
					break;
				case  7: //400M
					printf("%X", buffer_ut804[0]-0x30);
					printf("%X", buffer_ut804[1]-0x30);
					printf("%X", buffer_ut804[2]-0x30);
					printf(".");
					printf("%X", buffer_ut804[3]-0x30);
					printf("%X", buffer_ut804[4]-0x30);
					printf(" MHz");
					break;
				default: //error
					break;
			}
			break;

		case  13: //0x3D:	//F
			if( (buffer_ut804[0]-0x30)>9 )
			{
				printf("   0L. F");
			}
			else
			{
				switch ( buffer_ut804[5]-0x30 )
				{
					case  0: //1000F
						printf("%X", buffer_ut804[0]-0x30);
						printf("%X", buffer_ut804[1]-0x30);
						printf("%X", buffer_ut804[2]-0x30);
						printf("%X", buffer_ut804[3]-0x30);
						printf(".");
						printf("%X", buffer_ut804[4]-0x30);
						printf(" F");
						break;
					default: //error
						break;
				}
			}
			break;

		case  15: //0x3F:	//%
			printf(" 4-20-mA tester - ");

			break;






		default:	//other
			break;
	}
}


int main (int argc, char **argv)
{
	char *dvalue = NULL; //device interface
	int c,j;
	int multimeter_fd;
	unsigned char buffer_ut804[MAX_FILE_SIZE_UT804];
	int continue_main_loop;
	int verbose_flag=0;

	display_gnu();

	opterr = 0;
	while ((c = getopt (argc, argv, "hd:v")) != -1)
	{
		switch (c)
		{
			case 'd':
				dvalue = optarg;
				break;
			case 'v':
				verbose_flag=1;
				break;
			case '?':
				if (optopt == 'd' )
					fprintf (stderr, "Option -%c requires an argument. Try -h for help.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'. Try -h for help.\n", optopt);
				else
					fprintf (stderr, "Unknown option character `\\x%x'. Try -h for help.\n", optopt);
				return 1;
			case 'h':
				display_usage();
				return 0;
			default:
				display_usage();
				return -1;
		}
	}


	//open device descriptors
	multimeter_fd = open_port_ut804(dvalue);
	if(multimeter_fd == (-1))
	{
		printf("\t\t ---> check connection to the device\n\n");
		return -1;
	}



	/* ------==== MAIN LOOP ====------ */
	continue_main_loop = 1;

	while(continue_main_loop)
	{
		j = read(multimeter_fd, buffer_ut804, MAX_FILE_SIZE_UT804);
		if (j < 0)
		{
			fputs("read() from multimeter_fd failed!\n", stderr);
			continue_main_loop = 0;
		}

		if( verbose_flag==1 )
		{
			printf("raw data: ");
			//printf("%d||||||", buffer_ut804[0]);
			printf("%2X|", buffer_ut804[0]);
			printf("%2X|", buffer_ut804[1]);
			printf("%2X|", buffer_ut804[2]);
			printf("%2X|", buffer_ut804[3]);
			printf("%2X|", buffer_ut804[4]);
			printf("%2X|", buffer_ut804[5]);
			printf("****** %2X|", buffer_ut804[6]);
			printf("%2X|", buffer_ut804[7]);
			printf("%2X|", buffer_ut804[8]);
			printf("%2X|", buffer_ut804[9]);
			printf("%2X|", buffer_ut804[10]);
			printf(" \n");
		}

		if( !(buffer_ut804[7]==0 && buffer_ut804[8]==0 && buffer_ut804[9]==0 && buffer_ut804[10]==0) )
		{
			//display buffer_ut804
			display_buffer_ut804(buffer_ut804);
			printf("\n");	

			//clean buffer
			clean_buffer_ut804(buffer_ut804);
		}

	}// while(continue_main_loop) - end

	//close devices descriptors
	close(multimeter_fd);

	return 0;
}

