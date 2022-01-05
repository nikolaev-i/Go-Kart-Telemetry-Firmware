//Kinetik eGo-kart third canbus read and log


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <wiringPi.h>
#include <sys/time.h>
#include "filewriting.h"
#include "frozen.h"


#define LOWER 75.6F // Lowest battery level, used in SoC calculation


// TODO: opravi koeficientite
//      UNIXTIME
//      Buton za vkl/izkl?
//      Korektna tablica
//      Ako ima vreme sync trigger
//      Drugi quality of life neshta


struct tm *loctime;
struct timeval start_time;
long mili_time, seconds, useconds;
int32_t minute,second;
char flip = 0;

// Variables for saving data from different ECU modules
uint16_t Applied_voltage = 0; 
uint16_t Battery_Voltage = 0;
float Battery_Voltage_f = 0;
int16_t  _Id  = 0;
int16_t  _Iq = 0; 
float _Id_f = 0;
float _Iq_f = 0;
float Battery_current_f= 0;
float Torque_actual_f = 0;
float Applied_voltage_f = 0;
int8_t motor_temperature = 0;
int16_t Battery_current = 0;
int16_t Torque_actual = 0;
int8_t  Controller_temperature = 0; // 4602h TODO
int8_t Cell_temperature[6] = {0,0,0,0,0,0}; // Temperature from the termistor module, 0-5 temperature for each cell, 6 Highest temperature 
int8_t Hi_temp = 0;
int32_t Velocity = 0;
int32_t Current_speed = 0;
int SoC = 0;
int8_t Heat_Sink = 0;
int8_t Direction = 0;


/*-----------------------------------------------defines------------------------------------------------------*/

//Sevcon
#define TPDO1 0x00000319U  
#define TPDO2 0x00000217U   // Ud, Uq, Voltage Modulation, Capacitor Voltage
#define TPDO3 0x00000205U  // 
#define TPDO4 0x00000377U  // Motor Temp, Battery Current, Torque Demand, Torque Actual
#define TPDO5 0x00000135U  // uint_32 Max_Velocity, uint_32 Velocity, Controller Temperature


//Temperature module
#define TEMP1 0x00000140U




#define TCNST 0.021352 //Speed constant 3rd chassi 05.03.2021
#define Coef116 0.0625  // 1/16 koeficient ot Master Object Dictionary
#define Coef025 0.25 // 0.25V koeficient ot Master Object Dictionary
 
int main()
{
    

    time_t curtime;
    /*--------------------------defaul variables------------------------*/
    int s, nbytes, i, ret;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    
    
    
    memset(&frame, 0, sizeof(struct can_frame));
    
    system("sudo ip link set can0 type can bitrate 1000000"); //TODO: put a placeholder for the baudrate
    system("sudo ifconfig can0 up");
    printf("Running\r\n");
        
   



    //1.Create socket
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {
        perror("socket PF_CAN failed");
        return 1;
    }
    
    //2.Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(s, SIOCGIFINDEX, &ifr);
    if (ret < 0) {
        perror("ioctl failed");
        return 1;
    }

    //4.Filters    
  
    struct can_filter rfilter[6];
    rfilter[1].can_id   = TPDO5;
    rfilter[1].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[2].can_id   = TPDO2;
    rfilter[2].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[3].can_id = TPDO3;
    rfilter[3].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[0].can_id = TPDO4;
    rfilter[0].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[4].can_id = TPDO1;
    rfilter[4].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
    rfilter[5].can_id = TEMP1;
    rfilter[5].can_mask = CAN_EFF_MASK;

    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
 
    //3.Bind the socket to can0

    addr.can_family = PF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind failed");
        return 1;
    }
    


    //6.Open a file to write in
   // /*
     FILE * fPointer;
    if((fPointer=fopen("CAN_TEST.csv","a+"))==NULL){
		//if the file does not exist print the string
		printf("Cannot open text file");
		return 1;
	}
    fopen("CAN_TEST.csv","a+");
    fprintf(fPointer,"Time, Direction, Velocity, Battery voltage, Battery current, Actual Torque,Applied Voltage,Controller Temp, Motor Temp, Motor Id, Motor Iq,T1,T2,T3,T4,T5,T6\n"); // Capacitor Voltgage >> Battery Voltage
    fclose(fPointer);
  // */ 
    //7.Receive data and write to file
						//For refactoring 
    while(1) {
        nbytes = read(s, &frame, sizeof(frame));
        if(nbytes > 0) {
           curtime = time(NULL);
        loctime = localtime(&curtime);
        gettimeofday(&start_time, NULL);
             
			switch (frame.can_id)   //Processing incoming frames 
			{
                
                case TPDO1:
				_Id = frame.data[3]; 
                _Id <<= 8;
                _Id += frame.data[2];

                _Iq = frame.data[5];
                _Iq <<= 8;
                _Iq += frame.data[4];

                Heat_Sink = frame.data[6];
                Direction = frame.data[7];
                _Iq_f = (float)_Iq * Coef116;
                _Id_f = (float)_Id * Coef116;



              
                break;

				case TPDO2:
				Applied_voltage = frame.data[5]; 
                Applied_voltage <<= 8;
                Applied_voltage += frame.data[4];
                Applied_voltage_f = Applied_voltage * 0.390625;

                


				Battery_Voltage = frame.data[7];
                Battery_Voltage <<= 8;
                Battery_Voltage += frame.data[6];
                Battery_Voltage_f = (float)Battery_Voltage * Coef116;
                SoC = (int)(Battery_Voltage_f - LOWER) * 100 / 24;
                
                
                break;

                case TPDO4:

                motor_temperature = frame.data[0];
                Battery_current = frame.data[3];
                Battery_current <<= 8;
                Battery_current += frame.data[2];
                Battery_current_f = Battery_current * Coef116;
                Torque_actual = frame.data[6];
                Torque_actual <<= 8;
                Torque_actual += frame.data[7];
                Torque_actual_f = Torque_actual * Coef116;

                case TPDO5:
                    Velocity = frame.data[7];
                    Velocity <<= 8;
                    Velocity += frame.data[6];
                    Velocity <<= 8;
                    Velocity += frame.data[5];
                    Velocity <<= 8;
                    Velocity += frame.data[4];
                    Current_speed = (int)Velocity*TCNST;
                break;	
               

                case TEMP1:
                Cell_temperature[0] = frame.data[0];
                Cell_temperature[1] = frame.data[1];
                Cell_temperature[2] = frame.data[2];
                Cell_temperature[3] = frame.data[3];
                Cell_temperature[4] = frame.data[4];
                Cell_temperature[5] = frame.data[5];
                Hi_temp = frame.data[6];
                
                    break;
               //TODO

            
                	

				default:
                
					break;
				
					//timestamp, rpm , battery voltage, battery current, actual torque, voltage modulation, controller temperature, motor temperature, motor current id, motor current iq, cell temperature - 1-6,
			}
            if (Cell_temperature[0]!=0)
            {

             fopen("CAN_TEST.csv","a+");
            fprintf(fPointer,"%lu,%d,%d,%.3f,%.3f,%.3f,%d,%d,%d,%.3f,%.3f,%d,%d,%d,%d,%d,%d\n",(unsigned long)time(NULL),Direction,Velocity,Battery_Voltage_f,Battery_current_f, Torque_actual_f, Applied_voltage, Heat_Sink, motor_temperature, _Id_f, _Iq_f, Cell_temperature[0],Cell_temperature[1],Cell_temperature[2],Cell_temperature[3],Cell_temperature[4],Cell_temperature[5]); //TODO vnimavai s tipovete danni otdelno
             fclose(fPointer);
             
             i = start_time.tv_usec;
             second = loctime->tm_sec;
            
            for (int x = 0; x <= 5; ++x)
            {   
                Cell_temperature[x] = 0;
            }
             if (second == 58 )
             {  ++minute;
                 second = 0;
                 flip = 1;
            
                 }
              
             
             
            i = start_time.tv_usec;
             second = loctime->tm_sec;
            }

		  json_fprintf("test.json", "{ Speed: %d, ControllerTemp: %d, MotorTemp: %d, SoC: %d, BatteryVoltage: %f, Coolant: %d  }",Current_speed , Heat_Sink, motor_temperature, SoC, Battery_Voltage_f, Hi_temp );// Coolant -> Highest Temperature
          
        }
    }
    
    //8.Close the socket and can0 
    close(s);
    system("sudo ifconfig can0 down");
     fclose(fPointer);
          

    return 0;
}






