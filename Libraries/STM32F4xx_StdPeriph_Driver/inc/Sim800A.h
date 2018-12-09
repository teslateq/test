#ifndef _Sim800A_H_
#define _Sim800A_H_
#include "stm32f4xx.h"

// err ~ simEna
// Sim_err ~ simGPRS
// stepConn - statusM95 - staM95

extern uint8_t stepConn;
extern uint8_t statusM95;
extern uint8_t simGPRS;
extern int simDr;
extern uint8_t simEna;
extern volatile uint8_t staM95;
extern volatile int16_t pos;
extern volatile char buffer[300];
extern volatile char save_data[500];
extern volatile int pos_save;
extern volatile uint8_t httpAction[20];

extern volatile uint8_t staCreg[5];
extern volatile uint8_t staCgatt[5];

extern volatile uint8_t staCSQ[5];
extern volatile uint8_t Signal;
extern volatile uint8_t cntLostSignal;
extern volatile uint8_t cntLS;

extern volatile char sms_phone[50];
extern volatile char sms_mess[200];
extern volatile char sms_mess_split[50][50];
extern volatile uint8_t pos_phone, pos_mess;
extern volatile uint16_t sms_row, sms_col, num_sdt, len_sdt;

extern uint8_t oldRead;
extern int cntEqual;
extern int cntNEqual;
extern int cntSum;
extern int bias;
extern int bias1st;
extern int loadBias;

extern volatile uint8_t result_web[50];
extern volatile uint8_t responeOk;

// Take from server
#define rowW 100
#define colW 100
extern volatile uint8_t web[rowW][colW];

extern void Delay(__IO uint32_t nCount);
extern void Uart_puts(USART_TypeDef *USARTx,volatile char *str);
extern void Uart_putc(USART_TypeDef *USARTx,uint8_t ch);
extern uint8_t hex2asc(uint8_t ch);
extern uint8_t AsciiToInt(uint8_t ch);
void USART6_Configuration(unsigned int BaudRate);
void GSM(volatile char* msg);
void GSMD(volatile char* msg, int waitMs);
void GSMB(uint8_t* msg, uint8_t len);
void GSM_hex(uint8_t data);
void GSMH(uint8_t* data, uint32_t len);
void resetBuffer(void);
void USART6_IRQHandler(void);
int ascii2baud(uint8_t ch);
void PWK_On(void);
void PWK_Off(void);
void check_statusM95(void);
void check_statusM95_main(void);
void check_signal(void);
void check_gprs(void);

// http
void waitForRequestDone(void);
void request_get(char* url, char* data);
void request_post(char* url, char* data);
#endif   
