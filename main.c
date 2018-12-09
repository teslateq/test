#include "stm32f4xx.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "basicFunc.h"
#include "Uart.h"
#include "Sim800A.h"
#include "Modbus.h"
#include "rtc_internal.h"
#include "adc.h"
#include "i2c_eeprom.h"

USART_InitTypeDef USART_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
I2C_InitTypeDef   I2C_InitStructure;


void init_value(void);
int JoinValueModbus(volatile uint8_t *data, uint8_t RsAB);
void GET_http(void);
void POST_http(void);
void readSensor(void);
void push_data2web(USART_TypeDef *USARTx);
void load_data_fromServer(void);
void daily_rst(void);
void cne_rst(void);
void read_temp_volt(void);
void del_all_mess(void);
void sms_process(void);
void sms_warning(void);
void parseConfig();
uint8_t check_phone(volatile char *data1, volatile char *str, uint16_t num);

	
#define numLoad 50
#define num_ena_sdt 2
#define DOMAIN "f6555aed.ap.ngrok.io"

int abc;
uint8_t d[100];
uint16_t temp, volt;

int rst;
int rstIWDG;

uint8_t t1[30],t2[30],t3[30],t4[30];
uint8_t h[50];


float src;
uint8_t rstChange[5];
char sdt[20][50] = { {"0767543008"}, {"0903966046"} };
uint8_t passCheck, passCheck1;
uint8_t enaSms[5];

uint8_t T_RH_data[10], T_data[10], RH_data[10];
uint16_t humi, temp;
int len_push;
uint8_t data_sen[100];
char data_send_str[200];

int spiTO;

int intLoopCount = 0;

uint8_t readConfigCount = 0;
char readConfigId[100];
uint8_t readConfigParam[100][6];
uint8_t tempConfigParam[50];
int readPointer;
uint8_t * currentConfigParams;


uint8_t savedReadConfig[1000];

// function 15, inverter
uint16_t freqInv;
uint8_t freq15[10];
uint8_t statusInv[10];

// function 15, adam8024
uint8_t DI[20], DO[20];
uint8_t ADAM8024_out[10], ADAM8024_in[10];

// function 16
uint8_t r1Daviteq[5], r2Daviteq[5], r3Daviteq[5];

// function 02
uint8_t dataIO[10];
uint8_t input485[10];

// function 05
uint8_t relayPanasonic[10];


uint8_t debug;

void initConfig() {
	readConfigId[0] = '0';
	readConfigId[1] = 0;
}

int main(void)
{
	
	SysTick_Config(SystemCoreClock / 1000);
  USART1_Configuration(9600);			//rs485 master
	USART6_Configuration(115200);		//sim
	USART2_Configuration(9600);			//rs485 slave
	ADC_Config();
	I2C2_Configuration();
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);	// 32kHz
	IWDG_SetReload(4090);				//PR: 128 -> 4ms .. RL: 3750 -> 15s
  IWDG_ReloadCounter();
  IWDG_Enable();
	initConfig();

	
	check_statusM95();	
	Delay(1000);			//start-up time
	
	init_value();
		
	
	IWDG_ReloadCounter();
	
  I2C_BufferRead(0, savedReadConfig, 999);
	parseConfig(savedReadConfig);
	
	debug = 1;
		
	while (1)
  {		
		/*** Example function ***/
	
		/* Inverter */
//	//function 15, inverter
//	freqInv = (uint16_t)(55*16384/10);			// 0 - 100
//	freq15[0] = 0x7c;
//	freq15[1] = 0x04;
//	freq15[2] = freqInv;
//	freq15[3] = freqInv >> 8;
//	Send_uart_data15(USART1, 0x09, 0x00, 0x00, 0x00, 0x20, freq15);
//	
//	//function 01, inverter
//	Send_uart_data01(USART1,0x09, 0x00, 0x20, 0x00, 0x10, statusInv);	

//	//function 02, 8 input
//	Send_uart_data02(USART1, 0x02, 0x00, 0x00, 0x00, 0x08, dataIO);
//	split_bit(dataIO[0], input485);
		
		
		/* Testing module */
//	//function 16, daviteq io
//	Delay(5);
		//r1Daviteq[1] = 0x01;
		//Send_uart_data16(USART1,0x01,0x01,0x09,0x00,0x01, r1Daviteq);		// ON
		//Delay(5000);
		//Send_uart_data16(USART1,0x01,0x01,0x0a,0x00,0x01, r2Daviteq);		// ON
		//Delay(5000);
		//Send_uart_data16(USART1,0x01,0x01,0x0b,0x00,0x01, r3Daviteq);		// OFF
		//Delay(5000);
//		
//	// function 15, ADAM8024
	//char x = 0;
	//unsigned char y = 0;
	//uint8_t temp[1];		
	//DO[0] = 0;
	//DO[1] = 1;
	//DO[2] = 1;
	//DO[3] = 1;
	//for(int i=0 ; i<3; i++)					//add data
	//	temp[0] = temp[0]<<1 | DO[i];
	//ADAM8024_out[0] = 0x0c;		//take data
	//Send_uart_data15(USART1,0x0c,0x00,0x00,0x00,0x04, ADAM8024_out);
	//continue;
//	Send_uart_data02(USART1,0x0c,0x00,0x00,0x00,0x08, ADAM8024_in);			
//	split_bit(ADAM8024_in[0], DI);	//split 8 bit ~ 8 input
//	
//	// function 05, 4 output, Relay 1 - 4
//	for(int i=0; i<4; i++) {
//		if(relayPanasonic[i] == 1)			
//			Send_uart_data05(USART1,0x04,0x00,i,0xFF,0x00);
//		else if(relayPanasonic[i] == 0)	
//			Send_uart_data05(USART1,0x04,0x00,i,0x00,0x00);
//	}
	
	//function 06, not used
		
		
		
	ADAM8024_out[0] = 0x0f;		//take data
	Send_uart_data15(USART1,0x0c,0x00,0x00,0x00,0x04, ADAM8024_out);
	
	
	
		IWDG_ReloadCounter();
		while(rstIWDG == 1);
	}
}

void parseConfig(char* config) {	//bug
	/*
	2;2,3,0,0,0,1,;2,3,0,1,0,1,;
	config: <hash>;<count>;[param, param...]
	param: [Add,Func,SA1,SA2,Len1,Len2]
	*/
	int length = strlen(config);
	if (!length) {
		return;
	}
	int count = 0;
	int rowCount = 0;
	int colCount = 0;
	
	char temp[50];
	int tempCount = 0;
	
	int j;
	
	char temp2[10];
	int temp2Count = 0;
	readConfigCount = 0;
	int totalRowCount = 0;
	
	for (int i = 0; i < length; i++) {
		if (config[i] == ';') {  								// split by ;
			count++;
			if (count == 1) { 										// first number if count of read config
				temp[tempCount] = 0;
				strcpy(readConfigId, temp);
			} 
			else if (count == 2) {
				temp[tempCount] = 0;
				totalRowCount = atoi((char*) temp);
			}
			else {																// other part is config param
				for (int j = 0; j < 50; j++) {
					tempConfigParam[j] = 0;
				}
				// 1,3,0,0,0,1,
				colCount = 0;
				temp2Count = 0;
				for(j = 0; j < tempCount; j++) {
					temp2[temp2Count++] = temp[j];
					if (temp[j] == ',') {
						temp2[temp2Count - 1] = 0;
						tempConfigParam[colCount++] = atoi(temp2);
						temp2Count = 0;
					}
				}
				if (tempConfigParam[1] == 0x10) {
					//Send_uart_data16(USART1,0x01,0x01,0x09,0x00,0x01, r1Daviteq);
					Send_uart_data16(USART1, tempConfigParam[0],tempConfigParam[2],tempConfigParam[3],tempConfigParam[4],tempConfigParam[5], tempConfigParam + 6);
					Delay(5);
				} else if (tempConfigParam[1] == 0x0f) {
					// Send_uart_data15(USART1,0x0c,0x00,0x00,0x00,0x04, ADAM8024_out)
					Send_uart_data15(USART1, tempConfigParam[0],tempConfigParam[2],tempConfigParam[3],tempConfigParam[4],tempConfigParam[5], tempConfigParam + 6);
					Delay(5);
				} else if (tempConfigParam[1] == 0x05) {
					// Send_uart_data05(USART1,0x04,0x00,i,0xFF,0x00);
					Send_uart_data05(USART1, tempConfigParam[0],tempConfigParam[2],tempConfigParam[3],tempConfigParam[4],tempConfigParam[5]);
				} else if (tempConfigParam[1] == 0x06) {
					Send_uart_data06(USART1, tempConfigParam[0],tempConfigParam[2],tempConfigParam[3],tempConfigParam[4],tempConfigParam[5]);
				} else {
					for (int j = 0; j < colCount; j++) {
						readConfigParam[readConfigCount][j] = tempConfigParam[j];
					}
					readConfigCount += 1;
				}
				rowCount++;
				
				if (totalRowCount == rowCount) {
					break;
				}
			}
			tempCount = 0;
			continue;
		}
		temp[tempCount++] = config[i];
	}
	
	I2C_BufferWrite(0, config, length);
}


void hexToString(uint8_t* hex, int length) {
	for (int i = 0; i < length; i++) {
		data_send_str[i * 2] = hex2asc( hex[i] >> 4 );
		data_send_str[i * 2 + 1] = hex2asc( hex[i] & 0x0f );
	}
	data_send_str[length * 2] = 0;
}


char onlineConfigData[300];

void push_data2web(USART_TypeDef *USARTx) {	
	if(intLoopCount % 5 != 1) {
		char configUrl[200];
		configUrl[0] = 0;
		strcat(configUrl, "http://");
		strcat(configUrl, DOMAIN);
		strcat(configUrl, "/chip_manager/config?code=XXX&format=json&id=");
		strcat(configUrl, readConfigId);
		request_get(configUrl, (char *)onlineConfigData);
		// request_post("http://2b2ad417.ap.ngrok.io/chip_manager/sensor_data", onlineConfigData); // verifying
		parseConfig(onlineConfigData);	
	}
	
	readPointer = 0;
	for (int i = 0; i < readConfigCount; i++) {
		//2,3,0,0,0,1,;2,3,1,1,0,1,;
		currentConfigParams = readConfigParam[i];
		if (currentConfigParams[1] == 0x01) {
			// Send_uart_data01(USART1,0x09, 0x00, 0x20, 0x00, 0x10, statusInv)
			Send_uart_data01(USART1,currentConfigParams[0], currentConfigParams[2],  currentConfigParams[3], currentConfigParams[4], currentConfigParams[5], data_sen + readPointer);
			readPointer += currentConfigParams[5];
		} else if (currentConfigParams[1] == 0x02 ) {
			//Send_uart_data02(USART1,0x0c,0x00,0x00,0x00,0x08, ADAM8024_in);			
			//split_bit(ADAM8024_in[0], DI);	//split 8 bit ~ 8 input
			Send_uart_data02(USART1, currentConfigParams[0], currentConfigParams[2],  currentConfigParams[3], currentConfigParams[4], currentConfigParams[5], ADAM8024_in);  // funnc 02 bit read
			split_bit(ADAM8024_in[0], data_sen + readPointer);
			readPointer += currentConfigParams[5];
		} else if ((currentConfigParams[1] == 0x03) || (currentConfigParams[1] == 0x04)) {
			Send_uart_data(USART1, currentConfigParams[0], currentConfigParams[1],currentConfigParams[2],  currentConfigParams[3], currentConfigParams[4], currentConfigParams[5], data_sen + readPointer);  // extend read
			readPointer += currentConfigParams[5] * 2;
		}
	}
	
	if (readPointer) {
		hexToString(data_sen, readPointer);
		//hexToString(readConfigParam[i], 6);
		request_post("http://f6555aed.ap.ngrok.io/chip_manager/sensor_data?code=XXX", data_send_str);
	}

	
	//Signal
	GSM("AT+CSQ\n");
	waitForAtResponse("OK");
	
	//Rst
	GSM("AT+CGATT?\n");
	waitForAtResponse("OK");
	
	if(staCgatt[0] != 49){
		stepConn = 0;
		simGPRS = 1;
		loadBias = 0;
	}
}


void init_value(void) {
	// Add '0' to all
	for(int i=0; i<rowW; i++)	{
		for(int j=0; j<colW; j++)
			web[i][j] = '0';
	}
	// Enable rst bias
	web[0][2] = '1';
	// Set bias
	bias = 20;
	// Set load data for the first time
	loadBias = 0;
	//set Health
	for(int i=0; i<50; i++)
		h[i] = '0';
}
void daily_rst(void) {
	tellTime();
	tellDate();
	if(rst == 1 || (aShowTime[0] == 23 && aShowTime[1] == 59 && (aShowTime[2] >= 00 && aShowTime[2] <= 2))   )
	{
		Delay(1000);
		NVIC_SystemReset();
	}
}
void cne_rst(void) {
	cntSum++;
	if(oldRead == web[0][0])	cntEqual++;
	else	cntNEqual++;
	oldRead = web[0][0];
	
	if(cntSum == 100) {
		if(cntNEqual <= bias) {
			// if(web[0][2] == '1')
				NVIC_SystemReset();
		}
		cntSum = 0;
		cntEqual = 0;
		cntNEqual = 0;
	}
}
void read_temp_volt(void) {
	temp = adc[0];
	volt = adc[1];
	h[0] = temp>>8;
	h[1] = temp;
	h[2] = volt>>8;
	h[3] = volt;
	src = adc[1]*51.0/4095.0;
}
int JoinValueModbus(volatile uint8_t *data, uint8_t RsAB) {
	int databack;
	
	if(RsAB == 0)
		databack = data[0]*100 + data[1]*10 + data[2];
	else if(RsAB == 1)
		databack = data[0]*100000 + data[1]*10000 + data[2]*1000 + data[3]*100 + data[4]*10 + data[5];	
	
	return databack;
}

void del_all_mess(void) {
	GSMD("AT+CMGDA=\"DEL ALL\"\n",1000);
	for(int i=0; i<20; i++) {
		sms_phone[i] = 0;
		sms_mess[i] = 0;
		for(int j=0; j<20; j++)
			sms_mess_split[i][j] = 0;
		num_sdt = 0;
		len_sdt = 0;
	}
}
uint8_t check_phone(volatile char *data1, volatile char *str, uint16_t num) {
	for(uint16_t i; i<num; i++) {
		if(data1[i] != str[i])
			return 1;
	}
	return 0;
}
void sms_process(void) {
	if( strcmp((char*)sms_phone, "\"+84767543008\"") == 0 ) {						// if sdt: TRUE
			passCheck = 1;
			
		if(strcmp((char*)sms_mess, "*rst#") == 0) {															// rst
			passCheck = 2;
			
			enaSms[0] = 1;
			//I2C_BufferWrite(I2C2, EE_Write, 0x01, enaSms, 1);
			
			// respone Ok
			GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[0]); GSMD("\"\n",50);
			GSMD("rst: Ok",50);
			Uart_putc(USART6,(char)26); Delay(5000);
			IWDG_ReloadCounter();
			del_all_mess();
		}
		else if(strcmp((char*)sms_mess_split[1], "ds") == 0) {									// change phone
			passCheck = 3;
			num_sdt = as2i(sms_mess_split[2][0]);
			if(num_sdt != 0) {
				for(int i=0; i<len_sdt; i++) 
					sdt[num_sdt][i] = sms_mess_split[3][i];
				
				//I2C_BufferWrite(I2C2, EE_Write, (num_sdt+2), (unsigned char*)sdt[num_sdt], len_sdt);
				
				GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[0]); 	GSMD("\"\n",50);
				GSMB((unsigned char*)sms_mess_split[2], 1); 	GSM("-");					// num_sdt,
				GSMB((unsigned char*)sdt[num_sdt], len_sdt); 	GSMD(": Ok",70);	// sdt: Ok
				Uart_putc(USART6,(char)26); Delay(5000);
				IWDG_ReloadCounter();
			}
			del_all_mess();
		}
		else
			del_all_mess();
	}
	else if( check_phone(sms_phone,"\"+84",4) == 0 ) {									// if sdt: FALSE
		del_all_mess();
		passCheck1 = 1;
	}
}

void sms_warning(void) {
	for(int i=0; i<num_ena_sdt; i++) {
		GSM("AT+CMGS=\""); Uart_puts(USART6,sdt[i]); GSMD("\"\n",50);
		GSMD("Pin mode: On",100);
		Uart_putc(USART6,(char)26); Delay(5000);
		IWDG_ReloadCounter();
	}
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
