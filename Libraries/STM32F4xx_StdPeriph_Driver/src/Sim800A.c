#include <stm32f4xx.h>
#include "Sim800A.h"
#include <stdio.h>

uint8_t stepConn = 0;
uint8_t statusM95;
uint8_t simGPRS;
int simDr = 1;
uint8_t simEna;
volatile uint8_t staM95;
volatile int16_t pos = 0;
volatile char buffer[300];
volatile char save_data[500];
volatile int pos_save;
volatile uint8_t httpAction[20];

enum _parseStateSim {
  PS_DETECT_MSG_TYPE,
	PS_CHECK_STA,
	PS_CHECK_OK,
  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT,
	
	PS_CREG_N,
	PS_CREG_STAT,
	
	PS_CGATT_STAT,
	
	PS_CSQSpace,
	PS_CSQ,
	
	PS_CMGR_READ,
	PS_CMGR_PHONE,
	PS_CMGR_NULL_DATETIME,
	PS_CMGR_MESSAGE,
	PS_CMGR_WAIT,
};
volatile uint8_t parseStateSim = PS_DETECT_MSG_TYPE;

volatile uint8_t staCreg[5];
volatile uint8_t staCgatt[5];
volatile uint8_t staCSQ[5];
volatile uint8_t Signal;
volatile uint8_t cntLostSignal;
volatile uint8_t cntLS;

volatile char sms_phone[50];
volatile char sms_mess[200];
volatile char sms_mess_split[50][50];
volatile uint8_t pos_phone, pos_mess;
volatile uint16_t sms_row, sms_col, num_sdt, len_sdt;

uint8_t oldRead;
int cntEqual;
int cntNEqual;
int cntSum;
int bias;
int bias1st;
int loadBias;
volatile int httpWaiting = 0;
volatile char* httpWaitMessage;
volatile int getAtRemainResponse = 0;
volatile int getAtResponseSize = 0;

// Take from server
volatile uint8_t web[rowW][colW];
volatile int row,col;

volatile uint8_t result_web[50];
volatile uint8_t responeOk;


void USART6_Configuration(unsigned int BaudRate)
{	//for sim
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	//E2 for PWK
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
  /* Configure USART Tx as alternate function  */ //B6:Tx  B7:Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6); 
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); 
	
  USART_InitStructure.USART_BaudRate = BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	
  USART_Init(USART6, &USART_InitStructure);
  
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
  USART_Cmd(USART6, ENABLE);
}
void GSM(volatile char* msg) {
	Uart_puts(USART6, msg);
}
void GSMD(volatile char* msg, int waitMs) {
	Uart_puts(USART6, msg);
	Delay(waitMs);
}

void GSMB(uint8_t* msg, uint8_t len) {
	for(int i=0; i<len; i++)
		Uart_putc(USART6, msg[i]);
}
void GSM_hex(uint8_t data) {
	uint8_t d1, d2;
	d1 = hex2asc( data >> 4 );
	d2 = hex2asc( data & 0x0f );
	Uart_putc(USART6, d1);
	Uart_putc(USART6, d2);
}
void GSMH(uint8_t* data, uint32_t len) {
	for(int i=0; i<len; i++)
		GSM_hex(data[i]);
}


void resetBuffer(void) {
  memset((char*)buffer, 0, sizeof(buffer));
  pos = 0;
}
int ascii2baud(uint8_t ch) {
	int baudback;
	switch(ch)
	{
		case '0':{ baudback = 4800; break; }
		case '1':{ baudback = 9600; break; }
		case '2':{ baudback = 14400; break; }
		case '3':{ baudback = 19200; break; }
		case '4':{ baudback = 28800; break; }
		case '5':{ baudback = 38400; break; }
		case '6':{ baudback = 56000; break; }
		case '7':{ baudback = 57600; break; }
		case '8':{ baudback = 115200; break; }
		case '9':{ baudback = 128000; break; }
		case 'A':{ baudback = 256000; break; } 
		default:{ baudback = 88; break; }
	}
	return baudback;
}
void PWK_On(void){
	GPIO_SetBits(GPIOD,GPIO_Pin_12);
}
void PWK_Off(void){
	GPIO_ResetBits(GPIOD,GPIO_Pin_12);
}

void check_statusM95(void) {
		staM95 = 0;
		GSMD("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 0) {
			simEna++;
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(100);
		}
		else if(statusM95 == 1) {
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(3000);
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(100);
		}
}

void check_statusM95_main(void) {
		staM95 = 0;
		GSMD("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 0) {
			simEna++;
			PWK_On();
			Delay(1500);
			PWK_Off();
			Delay(100);
		}
		staM95 = 0;
		GSMD("AT\n",100);
		statusM95 = staM95;
		if(statusM95 == 1)
			stepConn = 1;
}
void check_signal(void) {
	GSMD("AT+CREG?\n",3000);
	if(staCreg[1] == 49)
		stepConn = 2;
}

void waitForAtResponse(char * message) {
	httpWaitMessage = message;
	httpWaiting = 1;
	while (httpWaiting);
}


void check_gprs(void) {
	GSMD("AT+SAPBR=3,1,\"APN\",\"m-wap\"\n",500);
	GSMD("AT+SAPBR=1,1\n",1000);
	GSMD("AT+CGATT?\n",500);	
	if(staCgatt[0] == 49) {
		Delay(5000);
		stepConn = 3;
	}
	
	GSM("AT+HTTPINIT\n");
	waitForAtResponse("OK");
}

void waitForRequestDone(void) {
	GSM("AT+HTTPSTATUS?");
	httpWaiting = 1;
	while(httpWaiting);
}


char * getRemainAtRespone(void) {
	getAtRemainResponse = 1;
	while(getAtRemainResponse);
	return (char*)buffer;
}

int getResponseSize(char* actionResponse) {
	int found = 0;
	int length = strlen(actionResponse);
	for (int i = 0; i < length; i++) {
		if (actionResponse[i] == ',') {
			found++;
			if (found == 1) {
				char status[4];
				for (int j = i + 1; j < length; j++) {
					status[j - i - 1] = actionResponse[j];
					if (actionResponse[i] == ',') {
						break;
					}
				}
				status[3] = 0;
				
				if (strcmp(status, "204") == 0) {
					return 0;
				}
				if (strcmp(status, "500") == 0) {
					return 0;
				}
				if (strcmp(status, "502") == 0) {
					return 0;
				}
			}
			if (found == 2) {
				char sizeStr[20];
				for (int j = i + 1; j < length; j++) {
					sizeStr[j - i - 1] = actionResponse[j];
				}
				sizeStr[length - i] = 0;
				return atoi(sizeStr);
			}
		}
	}
	return 10000;
}


void getAtResponseWithSize(char * data, int size) {
	resetBuffer();
	getAtResponseSize = size;
	while(getAtResponseSize);
	
	int length = strlen((char*) buffer);
	for (int i = 0; i < length; i++) {
		data[i] = buffer[i];
	}
	data[length] = 0;
}


void request_get(char* url, char* data) {
	
	GSM("AT+HTTPPARA=\"CID\",1\n");
	waitForAtResponse("OK");
	
	GSM("AT+HTTPPARA=\"URL\",\"");
	GSM(url);
	GSM("\"\n");
	waitForAtResponse("OK");
	
	httpAction[0] = 0;
	GSM("AT+HTTPACTION=0\n");
	waitForAtResponse("+HTTPACTION:"); // send success
	
	char *remainATResponse = getRemainAtRespone();
	
	int responseSize = getResponseSize(remainATResponse);
	
	if (!responseSize) {
		data[0] = 0;
		return;
	}
	
	char length[10];
	sprintf(length, "%d", responseSize);
	GSM("AT+HTTPREAD=0,");
	GSM((char *)length);
	GSM("\n");
	waitForAtResponse("+HTTPREAD:");
	
	char* readDataLength = getRemainAtRespone();
	
	
	getAtResponseWithSize(data, atoi(readDataLength));
	waitForAtResponse("OK");
}

void request_post(char* url, char* data) {
	GSM("AT+HTTPPARA=\"CID\",1\n");
	waitForAtResponse("OK");

	GSM("AT+HTTPPARA=\"URL\",\"");
	GSM(url);
	GSM("\"\n");
	waitForAtResponse("OK");
	
	GSM("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"\n");
	waitForAtResponse("OK");
	GSM("AT+HTTPDATA=");
	
	char length[10];
	sprintf(length, "%d", strlen(data) + 5);
	GSM((char *)length);
	GSM(",5000\n");
	waitForAtResponse("DOWNLOAD");
	
	GSM("data=");
	GSM(data);
	GSM("\n");
	waitForAtResponse("OK");
	
	httpAction[0] = 1;
	GSM("AT+HTTPACTION=1\n");
	waitForAtResponse("+HTTPACTION:");
	//waitForRequestDone();
}

void USART6_IRQHandler(void) //for NVIC_RXNE
{
	//-------------------------------------------------Start
	if( USART_GetITStatus(USART6, USART_IT_RXNE))
	{
		char b = USART6->DR;
		buffer[pos++] = b;
		
		if (httpWaiting) {
			if ((strcmp((char*) buffer, httpWaitMessage) == 0)) {
				httpWaiting = 0;
				resetBuffer();
				return;
			}
			if ( b == '\n' )	//01 03 00 00 00 02 c4 0b
				resetBuffer();
			return;
		}
		
		if (getAtRemainResponse) {
			if (b == '\n') {
				getAtRemainResponse = 0;
				parseStateSim = PS_HTTPACTION_TYPE;
			}
			return;
		}
		
		if (getAtResponseSize) {
			getAtResponseSize--;
			return;
		}
		
		if ( pos >= sizeof(buffer) )
			resetBuffer(); // just to be safe
		
		switch(parseStateSim){
			case PS_DETECT_MSG_TYPE: 
				{
					if ( b == '\n' )	//01 03 00 00 00 02 c4 0b
						resetBuffer();
					else {
						if ( pos == 3 && strcmp((char*)buffer, "AT+") == 0 ) {
							parseStateSim = PS_IGNORING_COMMAND_ECHO;
						}
						else if ( b == ':' ) {
							if ( strcmp((char*)buffer, "+CREG:") == 0) {
								parseStateSim = PS_CREG_N;
							}
							else if ( strcmp((char*)buffer, "+CGATT:") == 0) {
								parseStateSim = PS_CGATT_STAT;
							}
							else if ( strcmp((char*)buffer, "+CSQ:") == 0) {
								parseStateSim = PS_CSQSpace;
							}
							else if ( strcmp((char*)buffer, "+CMGR:") == 0) {
								parseStateSim = PS_CMGR_READ;
							}
							resetBuffer();
						}
						
						if ( pos == 3 && strcmp((char*)buffer, "AT\r") == 0 ) {
							parseStateSim = PS_CHECK_STA;
						}
					}
				}
				break;
				
			case PS_CHECK_STA:
				{
					if ( b == '\n' ){
						staM95 = 1;
						parseStateSim = PS_CHECK_OK;
						resetBuffer();
					}
				}
				break;	
				
			case PS_CHECK_OK:
				{
					if(b == '\n'){
						if(strcmp((char*)buffer, "OK") == 0){
							staM95 = 1;
						}
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_IGNORING_COMMAND_ECHO:
				{
					if ( b == '\n' ){
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
						
			case PS_CREG_N:
				{
					if ( b == ',' ){
						staCreg[0] = (uint8_t)buffer[1];
						parseStateSim = PS_CREG_STAT;
						resetBuffer();
					}
				}
				break;
				
			case PS_CREG_STAT:
				{
					if ( b == '\n' ){
						staCreg[1] = (uint8_t)buffer[0];
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_CGATT_STAT:
				{
					if ( b == '\n' ){
						staCgatt[0] = (uint8_t)buffer[1];
						parseStateSim = PS_DETECT_MSG_TYPE;
						resetBuffer();
					}
				}
				break;
				
			case PS_CSQSpace:
				{
					if ( b == 32 ){
						parseStateSim = PS_CSQ;
						resetBuffer();
					}
				}
				break;
				
			case PS_CSQ:
				{
					if ( b == ',' ){
						for(int i=0; i<pos-1; i++)
							staCSQ[i] = buffer[i];
						Signal = atoi((char*)staCSQ);
						parseStateSim = PS_IGNORING_COMMAND_ECHO;
						resetBuffer();
					}
				}
				break;
				
			case PS_CMGR_READ:
			{
				if(b == ',') {
					parseStateSim = PS_CMGR_PHONE;
					resetBuffer();
				}
			}
			break;
			
			case PS_CMGR_PHONE:
			{
				if(b == ',') {
//					for(int i=0; i<20; i++)
//						sms_phone[i] = buffer[i];
					parseStateSim = PS_CMGR_NULL_DATETIME;
					resetBuffer();
					pos_phone = 0;
				}
				else {
					sms_phone[pos_phone] = buffer[pos_phone];
					pos_phone++;
				}
				
			}
			break;
			
			case PS_CMGR_NULL_DATETIME:
			{
				if(b == '\n') {
					parseStateSim = PS_CMGR_MESSAGE;
					resetBuffer();
				}
			}
			break;
			
			case PS_CMGR_MESSAGE:
			{
				if(b == '\n') {
					len_sdt = sms_col;
					
					parseStateSim = PS_DETECT_MSG_TYPE;
					resetBuffer();
					pos_mess = 0;
					sms_row = 0;
					sms_col = 0;
				}
				else if(b == 13) {}
				else {
					sms_mess[pos_mess] = buffer[pos_mess];
					pos_mess++;
					
					if(b == '*') {
						sms_row++;
						sms_col = 0;
					}
					else {
						sms_mess_split[sms_row][sms_col] = b;
						sms_col++;
					}
				}
			}
			break;
			
				
			default:
				break;
		}
	}
}
