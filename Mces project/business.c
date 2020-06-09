#include<lpc214x.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "LCD.H"
#define IR1 (IO1PIN & 1<<24)
#define IR2 (IO1PIN & 1<<27)
#define PLOCK 0x00000400
#define LED_ON (IO0CLR=1U<<31)
#define LED_OFF (IO0SET=1U<<31)
#define COL0 (IO1PIN & 1<<19)
#define COL1 (IO1PIN & 1<<18)
#define COL2 (IO1PIN & 1<<17)
#define COL3 (IO1PIN & 1<<16)
#define uint16_t unsigned int
	
typedef struct
{
	unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char weekDay;
  unsigned char date;
  unsigned char month;
  unsigned int year;  
}rtc_t;
rtc_t rtc;
unsigned char rowsel=0,colsel=0;
unsigned char lookup_table[4][4]={{'0','1','2','3'},{'4','5','6','7'},{'8','9','a','b'},{'c','d','e','f'}};
unsigned char items[][50]={{"Okra"},{"Tinda"},{"Oreo"},{"Soap"}};
unsigned  int x=0;
// ISR Routine to blink LED D7 to indicate project working
__irq   void  Timer1_ISR(void)
 {
	x = ~x;//x ^ 1;
  if (x)   
    IO0SET  =  1u << 31;   //P0.31  =  1
  else   
    IO0CLR =   1u <<31;   // P0.31  = 0	 
	T1IR  =  0x01; // clear match0 interrupt, and get ready for the next interrupt
  VICVectAddr = 0x00000000 ; //End of interrupt 
 }
void SystemInit(void){
   PLL0CON = 0x01; 
   PLL0CFG = 0x24; 
   PLL0FEED = 0xAA; 
   PLL0FEED = 0x55; 
   while( !( PLL0STAT & 0x00000400 ))
   { ; }
   PLL0CON = 0x03;
   PLL0FEED = 0xAA;  // lock the PLL registers after setting the required PLL
   PLL0FEED = 0x55;
   VPBDIV = 0x01;      
} 
unsigned int adc(int no,int ch)
{
  // adc(1,4) for temp sensor LM35, digital value will increase as temp increases
	// adc(1,3) for LDR - digival value will reduce as the light increases
	// adc(1,2) for trimpot - digital value changes as the pot rotation
	unsigned int val;
	PINSEL0 |=  0x0F300000;   
	/* Select the P0_13 AD1.4 for ADC function */
  /* Select the P0_12 AD1.3 for ADC function */
	/* Select the P0_10 AD1.2 for ADC function */
  switch (no)        //select adc
    {
        case 0: AD0CR   = 0x00200600 | (1<<ch); //select channel
                AD0CR  |= (1<<24) ;            //start conversion
                while ( ( AD0GDR &  ( 1U << 31 ) ) == 0);
                val = AD0GDR;
                break;
 
        case 1: AD1CR = 0x00200600  | ( 1 << ch );       //select channel
                AD1CR |=  ( 1 << 24 ) ;                              //start conversion
                while ( ( AD1GDR & (1U << 31) ) == 0);
                val = AD1GDR;
                break;
    }
    val = (val  >>  6) & 0x03FF;         // bit 6:15 is 10 bit AD value
    return  val;
}

void RTC_Init(void)
{
   //enable clock and select external 32.768KHz
	   CCR = ((1<< 0 ) | (1<<4));//D0 - 1 enable, 0 disable
} 														// D4 - 1 external clock,0 from PCLK

// SEC,MIN,HOUR,DOW,DOM,MONTH,YEAR are defined in LPC214x.h
void RTC_SetDateTime(rtc_t *rtc)//to set date & time
{
     SEC   =  rtc->sec;       // Update sec value
     MIN   =  rtc->min;       // Update min value
     HOUR  =  rtc->hour;      // Update hour value 
     DOW   =  rtc->weekDay;   // Update day value 
     DOM   =  rtc->date;      // Update date value 
     MONTH =  rtc->month;     // Update month value
     YEAR  =  rtc->year;      // Update year value
}

void RTC_GetDateTime(rtc_t *rtc)
{
     rtc->sec     = SEC ;       // Read sec value
     rtc->min     = MIN ;       // Read min value
     rtc->hour    = HOUR;       // Read hour value 
     rtc->weekDay = DOW;      // Read day value 
     rtc->date    = DOM;       // Read date value 
     rtc->month   = MONTH;       // Read month value
     rtc->year    = YEAR;       // Read year value

}
void timer1_Init()
{
	T1TCR = 0X00;
	T1MCR = 0X03;  //011 
	T1MR0 = 150000;
	T1TC  = 0X00;
	VICIntEnable = 0x0000020;  //00100000 Interrupt Souce No:5, D5=1
	VICVectAddr5 = (unsigned long)Timer1_ISR;  // set the timer ISR vector address
	VICVectCntl5 = 0x0000025;  // set the channel,D5=1,D4-D0->channelNo-5
	T1TCR = 0X01;
}
void uart_init(void)
{
 //configurations to use serial port
 PINSEL0 |= 0x00000005;  // P0.0 & P0.1 ARE CONFIGURED AS TXD0 & RXD0
 U0LCR = 0x83;   /* 8 bits, no Parity, 1 Stop bit    */
 U0DLM = 0; U0DLL = 8; // 115200 baud rate,PCLK = 15MHz
 U0LCR = 0x03;  /* DLAB = 0                         */
}
void serialPrint(unsigned val)
{
	int i=0;
	unsigned char buf[50],ch;
	sprintf((char *)buf,"%d\x0d\xa",val);
	while((ch = buf[i++])!= '\0')
	  {
		while((U0LSR & (0x01<<5))== 0x00){}; 
    U0THR= ch;                         
	  }
}
void serialPrintStr(char * buf)
{
	int i=0;
	char ch;
	while((ch = buf[i++])!= '\0')	
	  {
		  while((U0LSR & (1u<<5))== 0x00){}; 
      U0THR= ch;   
	  }
	//send new line
	//while(U0LSR & (0x01<<5)){};U0THR = 13;
	//while(U0LSR & (0x01<<5)){};U0THR = 10;	
	
}
void delay(int cnt)
{
	T0MR0 = 1000;//14926; // some arbitrary count for delay
	T0MCR = 0x0004; // set Tiimer 0 Stop after Match
	while(cnt--)
	{
		T0TC = 0X00;
	  T0TCR = 1; // start the timer (enbale)
		while(!(T0TC == T0MR0)){};// wait for the match
	  T0TCR = 2;// stop the timer		
	}
}

char* board(void){
	char *a;
	int i=0;
	IO0DIR|= 1U<<31|0X00FF0000;
	SystemInit();
	uart_init();
	LED_ON;delay_ms(500);
	do{
	while(1){
	rowsel=0;IO0SET=0X000F0000;IO0CLR=1<<16;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=1;IO0SET=0x000F0000;IO0CLR=1<<17;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=2;IO0SET=0x000F0000;IO0CLR=1<<18;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	rowsel=3;IO0SET=0x000F0000;IO0CLR=1<<19;
		if(COL0==0){colsel=0;break;};if(COL1==0){colsel=1;break;};
		if(COL2==0){colsel=2;break;};if(COL3==0){colsel=3;break;};
	};
	delay_ms(50);
	while(COL0==0||COL1==0||COL2==0||COL3==0);
	delay_ms(50);
	IO0SET=0x000F0000;
	U0THR=lookup_table[rowsel][colsel];
	a[i++]=lookup_table[rowsel][colsel];
	}
	while(i<2);
	return a;
}
unsigned char getAlphaCode(unsigned char alphachar){
	switch(alphachar){
		case '1':return 0xf9;
		case '2':return 0xa4;
		case '3':return 0xb0;
		case '4':return 0x9b;
		case '0':return 0xc0;
		case '5':return 0x92;
		case '6':return 0x82;
		case '7':return 0xf8;
		case '8':return 0x80;
		case '9':return 0x90;
		case ' ':return 0xff;
		default:break;		
	}
	return 0xff;
}
void seven_seg(char *buf){
	unsigned char i,j;
	unsigned char seg7data,temp=0;
IO0DIR|=1U<<31|1U<<14|1U<<11|1U<<15;
IO0CLR|=1U<<31;
SystemInit();

	for(i=0;i<5;i++){
	seg7data=getAlphaCode(*(buf+i));
		for(j=0;j<8;j++){
		temp=seg7data&0x80000000;
			if(temp==0x80)
				IO0SET|=1<<14;
			else
				IO0CLR=1<<14;
			IO0SET|=1<<11;
			delay_ms(1);
			IO0CLR|=1<<11;
			seg7data=seg7data<<1;
		}
	}
	IO0SET|=1<<15;
	delay_ms(1);
	IO0CLR|=1<<15;
	return;
}
int load_cell()
{
int load;
	IO0SET|=1<<13;
	load=adc(1,4);
	delay_ms(1);
	IO0CLR|=1<<13;
	return load;
}
int main(){
    unsigned char msg[100];
	  unsigned int cnt;
	char *a,*b="abcd",*ms;
	unsigned char *item;
	int num,i1,i2,val;
  
 // initialize the peripherals & the board GPIO
	
    //Board_Init();
    SystemInit();
    uart_init();
	  RTC_Init();
	  timer1_Init(); 
	  LCD_Reset();
		LCD_Init();

	// set date & time to 7thApril 2020,10:00:00am 
	cnt=0;
		rtc.hour = 17;rtc.min =  00;rtc.sec =  00;//10:00:00am
    rtc.date = 14;rtc.month = 04;rtc.year = 2020;//07th April 2020
    RTC_SetDateTime(&rtc);  // comment this line after first use
	while(1){
		       RTC_GetDateTime(&rtc);//get current date & time stamp
			sprintf((char *)msg,"time:%2d:%2d:%2d  Date:%2d/%2d/%2d \x0d\xa",(uint16_t)rtc.hour,(uint16_t)rtc.min,(uint16_t)rtc.sec,(uint16_t)rtc.date,(uint16_t)rtc.month,(uint16_t)rtc.year);
			// use the time stored in the variable rtc for date & time stamping
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0x80); LCD_DisplayString((char*)msg);
			delay(2000);
			a=board();
			num=atoi(a);
			i2=num%10;
			i1=num/10;
			item=items[i1];
			val=load_cell();
			sprintf((char*)ms,"%d",val);
			seven_seg(ms);
			serialPrintStr((char*)ms);
			sprintf((char*)msg,"Welcome");
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0xc0);LCD_DisplayString((char*)msg);
			LCD_CmdWrite(0x94);LCD_DisplayString((char*)ms);
			sprintf((char*)msg,"%s",item);
			serialPrintStr((char*)msg);
			LCD_CmdWrite(0xD4);LCD_DisplayString((char*)msg);
	}
}