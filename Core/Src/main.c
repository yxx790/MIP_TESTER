/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "stdio.h"
#include "fonts.h"
#include "ssd1306.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define X0 0	//ssd1306
#define X0 2	//sh1106
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
_Bool short_state = 0; _Bool long_state = 0; uint32_t time_key1 = 0; //для кнопки
uint8_t page = 0; uint8_t selected = 0; uint8_t digitSelected = 0 ;uint8_t calibRestriction = 3;
uint32_t lastPing = 0;
uint8_t volPercent[]="0244";

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
void lcdPrint(uint8_t x, uint8_t y, char message[], uint8_t fontsize){
	SSD1306_GotoXY(X0 + x, y);
	if 		(fontsize == 1) {SSD1306_Puts(message , &Font_7x10, 1);}
	else if (fontsize == 2) {SSD1306_Puts(message , &Font_11x18, 1);}
	else if (fontsize == 3) {SSD1306_Puts(message , &Font_16x26, 1);}
	else if	(fontsize == 10) {SSD1306_Puts(message , &Font_7x10, 0);}
	else if (fontsize == 20) {SSD1306_Puts(message , &Font_11x18, 0);}
	else if (fontsize == 30) {SSD1306_Puts(message , &Font_16x26, 0);}
}
void lcdPrintSymbol(uint8_t x, uint8_t y, char Symbol, uint8_t fontsize){
	SSD1306_GotoXY(X0 + x, y);
	if 		(fontsize == 1) {SSD1306_Putc(Symbol , &Font_7x10, 1);}
	else if (fontsize == 2) {SSD1306_Putc(Symbol , &Font_11x18, 1);}
	else if (fontsize == 3) {SSD1306_Putc(Symbol , &Font_16x26, 1);}
	else if	(fontsize == 10) {SSD1306_Putc(Symbol , &Font_7x10, 0);}
	else if (fontsize == 20) {SSD1306_Putc(Symbol , &Font_11x18, 0);}
	else if (fontsize == 30) {SSD1306_Putc(Symbol , &Font_16x26, 0);}
}

void lcdPrintUpdate(uint8_t x, uint8_t y, char message[], uint8_t fontsize){
	lcdPrint(x, y, message, fontsize);
	SSD1306_UpdateScreen();
}

void page0draw(void){
	if (HAL_GetTick()-lastPing > 1000){//таймаут опроса опроса
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		uint8_t msg[]={0x46,0x0D};  uint8_t rx_buffer[73]={0};  MX_USART2_UART_Init();

		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buffer, 73, 100);
		if (rx_buffer[72] != 0x0D) {memset(rx_buffer, 0x20, sizeof(rx_buffer)/sizeof(rx_buffer[0]));}
		lastPing = HAL_GetTick();

		char C[6]={0};  char C1[6]={0};
		strncpy(C , &rx_buffer[43] , 5);
		strncpy(C1 , &rx_buffer[49] , 5);
		lcdPrint(0, 0, "C:        C1:", 1);
		lcdPrint(21, 0, C, 1); lcdPrint(91, 0, C1, 1);
	//		  	  lcdPrint(0, 0, rx_buffer, 1);//показать буфер

		char sn1[5]={0};	  char sn2[5]={0};
		strncpy(sn1 , &rx_buffer[61] , 4);
		strncpy(sn2 , &rx_buffer[65] , 4);
		lcdPrint(0, 13, "02", 2); lcdPrint(28, 13, sn1, 2); lcdPrint(78, 13, sn2, 2);

		uint8_t status[6]={0};
		strncpy(status , &rx_buffer[55] , 5);
		lcdPrint(0, 33, "status:", 1); lcdPrint(49, 33, status, 1);

	//	  char message[20]="";
		char message[20]={0};
		memset(message,' ',sizeof(message)/sizeof(message[0]));
		lcdPrint(0, 43, message, 1);
		if		(status[3] == '0' && status[4] == '0') {strcpy(message , "OK");}
		else if 	(status[3] == '1' && status[4] == '0') {strcpy(message , "warming up");}
		else if	(status[3] == '1' && status[4] == '1') {strcpy(message , "too many pings");}
		else if 	(status[3] == '2' && status[4] == '1') {strcpy(message , "dt/sec > 0.6 deg/s");}
		else if 	(status[3] == '2' && status[4] == '2') {strcpy(message , "dt/sec > 2 deg/s");}
		else if 	(status[3] == '2' && status[4] == '4') {strcpy(message , "dt; Zero < 0");}
		else if 	(status[3] == '3' && status[4] == '0') {strcpy(message , "Ur or Us < min");}
		else if 	(status[3] == '3' && status[4] == '1') {strcpy(message , "Zero < 0");}
		else if 	(status[3] == '4' && status[4] == '0') {strcpy(message , "temp not in range");}
		else if 	(status[3] == '5' && status[4] == '0') {strcpy(message , "rapid val changes");}
		else if 	(status[3] == '5' && status[4] == '1') {strcpy(message , "hardware error");}
		else if 	(status[3] == '9' && status[4] == '0') {strcpy(message , "software error");}
		else 											   {strcpy(message , "unknown");}
		lcdPrint(0, 43, message, 1);

	//	  char message2[10]="";
		char message2[10]={0};
		if (status[2] == '1') {strcpy(message2 , "LowPWR");}
		else if (status[2] == '0') {strcpy(message2 , "      ");}
		else {strcpy(message2 , "unknown");}
		lcdPrint(84, 33, message2, 1);

		char Um[6]={0};  char Ur[6]={0};
		strncpy(Um , &rx_buffer[19] , 5);
		strncpy(Ur , &rx_buffer[13] , 5);
		lcdPrint(0, 53, "Um:       Ur:", 1);
		lcdPrint(21, 53, Um, 1); lcdPrint(91, 53, Ur, 1);

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		SSD1306_UpdateScreen();
	}
}
void pageMenuDraw(void){//menu
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	lcdPrint(0, 0,  " calibration", 1);
	lcdPrint(0, 10, " settings", 1);
	lcdPrint(0, 20, " parameters", 1);
//	lcdPrint(0, 30, " rezerv1", 1);
//	lcdPrint(0, 40, " rezerv2", 1);
	lcdPrint(0, 50, " Exit", 1);

	if (selected == 1){lcdPrint(0, 0,  "-", 1);};
	if (selected == 2){lcdPrint(0, 10, "-", 1);};
	if (selected == 3){lcdPrint(0, 20, "-", 1);};
//	if (selected == 3){lcdPrint(0, 30, "-", 1);};
//	if (selected == 4){lcdPrint(0, 40, "-", 1);};
	if (selected == 0){lcdPrint(0, 50, "-", 1);};

	SSD1306_UpdateScreen();
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
}

void pageCalibrationDraw(void){//calibration
//	lcdPrint(0, 0,  "    CALIBRATION", 1);
	lcdPrint(57, 0, " Help",   1);	if (selected == 1) {lcdPrint(57, 0, "-", 1);};
	lcdPrint(0, 0, " Zero  ",  1);	if (selected == 2) {lcdPrint(0, 0, "-", 1);};
	lcdPrint(0, 10, " Calib", 1);	if (selected == 3) {lcdPrint(0, 10, "-", 1);};
	lcdPrint(0, 22, " Zero0 ",  1);	if (selected == 4) {lcdPrint(0, 22, "-", 1);};
	lcdPrint(0, 32, " Init  ",  1);	if (selected == 5) {lcdPrint(0, 32, "-", 1);};
	lcdPrint(0, 42, " Zero2 ",  1);	if (selected == 6) {lcdPrint(0, 42, "-", 1);};
	lcdPrint(0, 53, " Exit",   1);	if (selected == 0) {lcdPrint(0, 53, "-", 1);};

	if (HAL_GetTick()-lastPing > 1100){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		MX_USART2_UART_Init(); uint8_t msg[] = "CCS\r"; uint8_t rx_bufferA[19] = {0};
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_bufferA, 18, 50);
		lastPing = HAL_GetTick();
		if (rx_bufferA[17] != 0x0D) {memset(rx_bufferA, 0x20, sizeof(rx_bufferA)/sizeof(rx_bufferA[0]));}
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
//		lcdPrint(0, 00, rx_bufferA, 1);
		char C1[6]; 	strncpy(C1, &rx_bufferA[0] , 5); 	lcdPrint(71, 33, "C1:", 1);		 lcdPrint(92, 33, C1, 1);
		char Temp[6]; 	strncpy(Temp, &rx_bufferA[6] , 5);	lcdPrint(57, 43, "Temp:", 1);	 lcdPrint(92, 43, Temp, 1);
		char Stat[6]; 	strncpy(Stat, &rx_bufferA[12] , 5);	lcdPrint(57, 53, "Stat:", 1);	 lcdPrint(92, 53, Stat, 1);
//		uint16_t NKPR = atoi(C1); NKPR = round(NKPR/4.4); if(NKPR>999){NKPR=999;} if(C1[0]==0x2D){NKPR=0;}
		int16_t NKPR = atoi(C1); NKPR = round(NKPR/4.4f); if (C1[0]==0x2D){NKPR=-1;} else if (C1[0]==0x20){NKPR=-11;} else if (NKPR>999){NKPR=999;}
		char NKPRchar[3];	lcdPrint(92, 12, "   ", 2);	sprintf (NKPRchar, "%d", NKPR);
//		char NKPRchar[3];	lcdPrint(92, 12, "   ", 2);	sprintf (NKPRchar, "%u", NKPR);
		lcdPrint(57, 20, "NKPR:", 1);	lcdPrint(92, 12, NKPRchar, 2);
	}

	SSD1306_DrawLine(0, 20, 55, 20, 1); SSD1306_DrawLine(0, 52, 55, 52, 1); SSD1306_DrawLine(55, 10, 128, 10, 1);	SSD1306_DrawLine(55, 0, 55, 64, 1);	SSD1306_DrawLine(55, 30, 128, 30, 1);
	SSD1306_UpdateScreen();
}
void zero(void){
	while (HAL_GetTick()-lastPing < 1100) { }
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	uint8_t msg[]="ZERO\r";
	uint8_t rx_buff[10]={0};
//	memset(rx_buff, 0x20, sizeof(rx_buff)/sizeof(rx_buff[0]));
	MX_USART2_UART_Init();
	HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
	HAL_UART_Receive(&huart2, rx_buff, 10, 50);
	lastPing = HAL_GetTick();
	if (rx_buff[5]==0x4F && rx_buff[6]==0x4B){lcdPrintUpdate(7, 0, "OK    ",  1);}
	else {lcdPrintUpdate(7, 0, "FAULT ",  1);}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
	HAL_Delay(1000);
}
void zero0(void){
	while (HAL_GetTick()-lastPing < 1100) { }
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	uint8_t msg[]="ZERO0\r";
	uint8_t rx_buff[10]={0};
//	memset(rx_buff, 0x20, sizeof(rx_buff)/sizeof(rx_buff[0]));
	MX_USART2_UART_Init();
	HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
	HAL_UART_Receive(&huart2, rx_buff, 10, 50);
	lastPing = HAL_GetTick();
	if (rx_buff[6]==0x4F && rx_buff[7]==0x4B){lcdPrintUpdate(7, 22, "OK    ",  1);}
	else {lcdPrintUpdate(7, 22, "FAULT ",  1);}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
	HAL_Delay(1000);
}
void init(void){
	while (HAL_GetTick()-lastPing < 1100) { }
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	uint8_t msg[]="INIT\r";
	uint8_t rx_buff[10]={0};
//	memset(rx_buff, 0x20, sizeof(rx_buff)/sizeof(rx_buff[0]));
	MX_USART2_UART_Init();
	HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
	HAL_UART_Receive(&huart2, rx_buff, 10, 50);
	lastPing = HAL_GetTick();
	if (rx_buff[5]==0x4F && rx_buff[6]==0x4B){lcdPrintUpdate(7, 32, "OK    ",  1);}
	else {lcdPrintUpdate(7, 32, "FAULT ",  1);}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
	HAL_Delay(1000);
}
void zero2(void){
	while (HAL_GetTick()-lastPing < 1100) { }
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	uint8_t msg[]="ZERO2\r";
	uint8_t rx_buff[10]={0};
//	memset(rx_buff, 0x20, sizeof(rx_buff)/sizeof(rx_buff[0]));
	MX_USART2_UART_Init();
	HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
	HAL_UART_Receive(&huart2, rx_buff, 10, 50);
	lastPing = HAL_GetTick();
	if (rx_buff[6]==0x4F && rx_buff[7]==0x4B){lcdPrintUpdate(7, 42, "OK    ",  1);}
	else {lcdPrintUpdate(7, 42, "FAULT ",  1);}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
	HAL_Delay(1000);
}
void pageCalib50draw(void){
	uint8_t Y = 22;
	lcdPrint(0, 0, "VOL.PERCENT", 2);
	selected == 1 ?	lcdPrintSymbol(35, Y, volPercent[0], 20)	:	lcdPrintSymbol(35, Y, volPercent[0], 2);
	selected == 2 ? lcdPrintSymbol(35+11, Y, volPercent[1], 20)	:	lcdPrintSymbol(35+11, Y, volPercent[1], 2);
	lcdPrint	  (35+22, Y, ",", 2);
	selected == 3 ? lcdPrintSymbol(35+33, Y, volPercent[2], 20)	:	lcdPrintSymbol(35+33, Y, volPercent[2], 2);
	selected == 4 ? lcdPrintSymbol(35+44, Y, volPercent[3], 20)	:	lcdPrintSymbol(35+44, Y, volPercent[3], 2);
	selected == 0 ? lcdPrint(0,		 64-19, "Exit", 20)			:	lcdPrint(0,		 64-19, "Exit", 2);
	selected == 5 ? lcdPrint(128-23, 64-19, "OK", 20)			:	lcdPrint(128-23, 64-19, "OK", 2);
	SSD1306_UpdateScreen();
}
void pageDigitSelect(void){
	lcdPrint(0, 0, "              ", 2);
	for (uint8_t i = 0; i < 10; i++) {
		digitSelected == i ? lcdPrintSymbol(4+i*11, 0, i+48, 20)	:	lcdPrintSymbol(4+i*11, 0, i+48, 2);
	}
	SSD1306_UpdateScreen();
}
void setPGSfunk(void){
	SSD1306_Clear();
	while (HAL_GetTick()-lastPing < 1100) { }
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
	uint8_t msg[10]={0};
	sprintf (msg, "CALB %.4s", volPercent);
	lcdPrintUpdate(11, 22, msg,   2);
	sprintf (msg, "CALB %s\r", volPercent);
	HAL_Delay(800);
	uint8_t rx_buff[15]={0};
	MX_USART2_UART_Init();
	HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
	HAL_UART_Receive(&huart2, rx_buff, 15, 50);
	lastPing = HAL_GetTick();
	if (rx_buff[10]==0x4F && rx_buff[11]==0x4B){lcdPrintUpdate(10, 22, "    OK    ",  2); page = 2; selected = 3;}
	else {lcdPrintUpdate(10, 22, "  FAULT    ",  2);}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
	HAL_Delay(1000);
	SSD1306_Clear();
}
void pageSettingsDraw(void){
	lcdPrint(0, 50, " Exit", 	 1);		if (selected == 0) {lcdPrint(0, 50, "-", 1);};
	lcdPrint(0, 0,	" OEM/USR?",   1);		if (selected == 1) {lcdPrint(0, 0, "-", 1);};
	lcdPrint(0, 10, " set OEM",	 1);		if (selected == 2) {lcdPrint(0, 10, "-", 1);};
	lcdPrint(0, 20, " set USER",	 1);	if (selected == 3) {lcdPrint(0, 20, "-", 1);};
	lcdPrint(0, 30, " LOWPWR_0",	 1);	if (selected == 4) {lcdPrint(0, 30, "-", 1);};
	lcdPrint(0, 40, " LOWPWR_1",	 1);	if (selected == 5) {lcdPrint(0, 40, "-", 1);};
	SSD1306_UpdateScreen();
}
void oemUserQuestion(void){
	while (HAL_GetTick()-lastPing < 1100) { }
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		lcdPrint(7, 0,	"        ",   1);
		uint8_t msg[]="UART?\r";	uint8_t rx_buff[10]={0};	MX_USART2_UART_Init();
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buff, 10, 50);
		lastPing = HAL_GetTick();
		if (rx_buff[0] == 0x4F ) {lcdPrintUpdate(7, 0, "OEM",  1);}
		else if (rx_buff[0] == 0x55 ) {lcdPrintUpdate(7, 0, "USER",  1);}
		else {lcdPrintUpdate(7, 0, "FAULT",  1);}
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		HAL_Delay(1000);
}
void oem(void){
	while (HAL_GetTick()-lastPing < 1100) { }
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		lcdPrint(7, 10,	"        ",   1);
		uint8_t msg[]="OEM 0000\r";	uint8_t rx_buff[10]={0};	MX_USART2_UART_Init();
		//добавить определение пароля
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buff, 10, 50);
		lastPing = HAL_GetTick();
		if (rx_buff[0] == 0x4F ) {lcdPrintUpdate(7, 10, "OK",  1);}
		else {lcdPrintUpdate(7, 10, "FAULT",  1);}
//		lcdPrintUpdate(7, 10, rx_buff,  1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		HAL_Delay(1000);
}
void user(void){
	while (HAL_GetTick()-lastPing < 1100) { }
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		lcdPrint(7, 20,	"        ",   1);
		uint8_t msg[]="USER\r";	uint8_t rx_buff[10]={0};	MX_USART2_UART_Init();
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buff, 10, 50);
		lastPing = HAL_GetTick();
		if (rx_buff[0] == 0x55 ) {lcdPrintUpdate(7, 20, "OK",  1);}
		else {lcdPrintUpdate(7, 20, "FAULT",  1);}
//		lcdPrintUpdate(7, 20, rx_buff,  1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		HAL_Delay(1000);
}
void LOWPWR0(void){
	while (HAL_GetTick()-lastPing < 1100) { }
		lcdPrint(7, 30,	"        ",   1);
		uint8_t rx_buff[15]={0};
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		uint8_t msg[]="LOWPW0\r";
		MX_USART2_UART_Init();
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buff, 15, 50);
		lastPing = HAL_GetTick();
		if (rx_buff[7] == 0x4F ) {lcdPrintUpdate(7, 30, "OK",  1);}
		else {lcdPrintUpdate(7, 30, "FAULT",  1);}
//		lcdPrintUpdate(7, 30, rx_buff,  1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		HAL_Delay(1000);
}
void LOWPWR1(void){
	while (HAL_GetTick()-lastPing < 1100) { }
		lcdPrint(7, 40,	"        ",   1);
		uint8_t rx_buff[15]={0};
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		uint8_t msg[]="LOWPW1\r";
		MX_USART2_UART_Init();
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buff, 15, 50);
		lastPing = HAL_GetTick();
		if (rx_buff[1] == 0x4F ) {lcdPrintUpdate(7, 40, "OK",  1);}
		else {lcdPrintUpdate(7, 40, "FAULT",  1);}
//		lcdPrintUpdate(7, 40, rx_buff,  1);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		HAL_Delay(1000);
}

void pageParamsDraw(void){
	if (HAL_GetTick()-lastPing > 1100){//таймаут опроса опроса
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, RESET);
		uint8_t msg[]={0x46,0x0D};  uint8_t rx_buffer[73]={0};  MX_USART2_UART_Init();
		HAL_UART_Transmit(&huart2, msg, sizeof(msg), 50);
		HAL_UART_Receive(&huart2, rx_buffer, 73, 100);
		lastPing = HAL_GetTick();
		char C[6]={0}; char C1[6]={0}; char Um[6]={0}; char Ur[6]={0}; char Stz0[6]={0}; char Stz[6]={0}; char St[6]={0};  char Stzkt[6]={0};
		strncpy(C , &rx_buffer[43] , 5); strncpy(C1 , &rx_buffer[49] , 5);
		strncpy(Stz0 , &rx_buffer[25] , 5); strncpy(Stz , &rx_buffer[31] , 5);
		strncpy(St , &rx_buffer[7] , 5); strncpy(Stzkt , &rx_buffer[37] , 5);
		strncpy(Um , &rx_buffer[19] , 5);  strncpy(Ur , &rx_buffer[13] , 5);
		lcdPrint(0, 0, "C :     ", 1); lcdPrint(21, 0, C, 1); lcdPrint(63, 0, "C1 :     ", 1); lcdPrint(91, 0, C1, 1);
		lcdPrint(0, 15, "S0:     ", 1); lcdPrint(21, 15, Stz0, 1);	lcdPrint(63, 15, "Stz:     ", 1); lcdPrint(91, 15, Stz, 1);
		lcdPrint(0, 25, "St:     ", 1); lcdPrint(21, 25, St, 1);	lcdPrint(63, 25, "Skt:     ", 1); lcdPrint(91, 25, Stzkt, 1);
		lcdPrint(0, 40, "Um:     ", 1); lcdPrint(21, 40, Um, 1);  lcdPrint(63, 40, "Ur :     ", 1);  lcdPrint(91, 40, Ur, 1);
		lcdPrint(0, 53, " Exit",   1);	if (selected == 0) {lcdPrint(0, 53, "-", 1);};
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, SET);
		SSD1306_UpdateScreen();
	}
}
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  	HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  	SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  	MX_GPIO_Init();
  	MX_I2C1_Init();
  	MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
//  HAL_Delay(1000);
  	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
	SSD1306_Init();
	lcdPrint(35, 0, "BOBER", 2);
	lcdPrint(35, 19, "KURWA", 2);
	lcdPrint(9, 38, "TESTER v13", 2);
	SSD1306_UpdateScreen();
	HAL_Delay(500);
	SSD1306_Clear();
//  lastPing = HAL_GetTick()-1000;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
while (1){
	uint32_t ms = HAL_GetTick();
	_Bool key1_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);
	if(key1_state == 0 && !short_state && (ms - time_key1) > 50) {short_state = 1; long_state = 0; time_key1 = ms;}
	else if(key1_state == 0 && !long_state && (ms - time_key1) > 500){// действие на длинное нажатие
		long_state = 1;

		if 		(page == 0 && selected == 7)	 {calibRestriction = 6;}//снять ограничение на калибровку
		if 		(page == 0)						{page = 1; selected = 1; SSD1306_Clear();}//to menu

		else if 	(page == 1 && selected == 0)	{page = 0; selected = 0; SSD1306_Clear();}//exit
		else if 	(page == 1 && selected == 1)	{page = 2; selected = 2; SSD1306_Clear();}//to calibration
		else if 	(page == 1 && selected == 2)	{page = 3; selected = 1; SSD1306_Clear();}//settings
		else if 	(page == 1 && selected == 3)	{page = 4; selected = 0; SSD1306_Clear();}//params
	//	  else if 	(page == 1 && selected == 4)	{page = 5; selected = 0; SSD1306_Clear();}//rezerv1
	//	  else if 	(page == 1 && selected == 5)	{page = 6; selected = 0; SSD1306_Clear();}//rezerv2

		else if 	(page == 2 && selected == 0)	{page = 1; selected = 1; calibRestriction = 3; SSD1306_Clear();}//exit to menu
		else if 	(page == 2 && selected == 1)	{}													//help
		else if 	(page == 2 && selected == 2)	{lcdPrintUpdate(0, 0, "|", 1); zero();}				//zero
		else if 	(page == 2 && selected == 3)	{page = 20; selected = 1; SSD1306_Clear();}			//set 50 page
		else if 	(page == 2 && selected == 4)	{lcdPrintUpdate(0, 22, "|", 1); zero0();}			//zero0
		else if 	(page == 2 && selected == 5)	{lcdPrintUpdate(0, 32, "|", 1); init();}			//init
		else if 	(page == 2 && selected == 6)	{lcdPrintUpdate(0, 42, "|", 1); zero2();}			//zero2

		else if 	(page == 20 && selected == 5)	{setPGSfunk();}										//set PGS funk
		else if 	(page == 20 && selected == 0)	{page = 2; selected = 3; SSD1306_Clear();}			//exit to menu
		else if 	(page == 20 && selected < 5)	{page = 21; digitSelected = volPercent[selected-1]-48;}				//


		else if 	(page == 21)					{page = 20; volPercent[selected-1] = digitSelected+48;}				//change digit

		else if 	(page == 3 && selected == 0)	{page = 1; selected = 2; SSD1306_Clear();}			//exit to menu
		else if 	(page == 3 && selected == 1)	{lcdPrintUpdate(0, 0, "|", 1); oemUserQuestion();}	//oemUserQuestion
		else if 	(page == 3 && selected == 2)	{lcdPrintUpdate(0, 10, "|", 1); oem();}		 		//oem
		else if 	(page == 3 && selected == 3)	{lcdPrintUpdate(0, 20, "|", 1); user();}		 	//user
		else if 	(page == 3 && selected == 4)	{lcdPrintUpdate(0, 30, "|", 1); LOWPWR0();}			//LOWPWR0
		else if 	(page == 3 && selected == 5)	{lcdPrintUpdate(0, 40, "|", 1); LOWPWR1();}			//LOWPWR1

		else if 	(page == 4 && selected == 0)	{page = 1; selected = 3; SSD1306_Clear();}			//exit to menu
	}
	else if(key1_state == 1 && short_state && (ms - time_key1) > 50){
		short_state = 0;
		time_key1 = ms;
		if(!long_state){// действие на короткое нажатие
			if (page==0) {selected++; if (selected > 7) {selected=0;}} 					//0page
			if (page==1) {selected++; if (selected > 3) {selected=0;}} 					//menu
			if (page==2) {selected++; if (selected > calibRestriction) {selected=0;}} 	//calib
			if (page==3) {selected++; if (selected > 5) {selected=0;}} 					//settingss
			if (page==20) {selected++;if (selected > 5) {selected=0;}}  				//calib
			if (page==21) {digitSelected++;if (digitSelected > 9) {digitSelected=0;}}  	//digit
	  	}
	}

	if (page == 0) {page0draw();}
	if (page == 1) {pageMenuDraw();}
	if (page == 2) {pageCalibrationDraw();}
	if (page == 20) {pageCalib50draw();}
	if (page == 21) {pageDigitSelect();}
	if (page == 3) {pageSettingsDraw();}
	if (page == 4) {pageParamsDraw();}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x0010020A;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_sys_GPIO_Port, LED_sys_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OLED_RESET_GPIO_Port, OLED_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_sys_Pin */
  GPIO_InitStruct.Pin = LED_sys_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_sys_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : OLED_RESET_Pin */
  GPIO_InitStruct.Pin = OLED_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OLED_RESET_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
