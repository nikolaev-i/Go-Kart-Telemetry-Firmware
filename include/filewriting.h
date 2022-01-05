#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "frozen.h"

// Open a txt file for data storage and write message data
void writedata (int number, int data){
    FILE * fPointer;
    if((fPointer = fopen("CAN_DATA.csv","w+"))==NULL){
		//if the file does not exist print the string
		printf("Cannot open text file");
		exit (1);
	}
    fprintf(fPointer,"Data[%d] = %d \r\n",number,data);
    fclose(fPointer); 
}


void writeParametersAll(int _RPM, float _Speed, float _BatteryV, float ControllerT, int SoC, float MotorT)
{
		FILE * fpointer;
		if((fpointer = fopen("CAN_DATA.txt","w+"))==NULL)
	{
		//if the file does not exist print the string
		printf("Cannot open text file");
		exit (1);
		
		
		fprintf(fpointer,"Speed: %f km/h \n", _Speed); 
				fclose(fpointer); 
	}
	
	//TODO Json output format
	
	
}
void writeArray() //Writing to file processed parameters 
{
	FILE * fpointer;
		if((fpointer = fopen("CAN_DATA.txt","w+"))==NULL)
	{
		//if the file does not exist print the string
		printf("Cannot open text file");
		exit (1);
	}
	

		fclose(fpointer);

	
}
//Open a txt file for data storage and write message info - code source wikipedia LoL
void writemessageid (int can_id, int can_dlc){
    time_t current_time;
    char* c_time_string;


    /* Obtain current time. */
    current_time = time(0);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }
    FILE * fPointer;
    if((fPointer = fopen("CAN_DATA.txt","w+"))==NULL){
		//if the file does not exist print the string
		printf("Cannot open text file");
		exit (1);
	}
    fprintf(fPointer,"\nTime:%s",c_time_string);
    fclose(fPointer); 

}
// 99.6            75.6
// 100				0
		//24V
		
