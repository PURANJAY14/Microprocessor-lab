#include <at89c5131.h>
#include "lcdproj.h"		//Header file with LCD interfacing functions
#include "serialproj.c"	//C file with UART interfacing functions
sbit LED=P1^7;        
sbit pinA=P1^0;
sbit pinB=P1^1;
sbit pinC=P1^2;
sbit pinD=P1^3;
int state_A=0;			//to detect a toggle on a pin(store the current state)
int state_B=0;
int state_C=0;
int state_D=0;
int token=-1;									//token issued
int token_processing=0;				//token which was last processed by any bank counter
int ready_A=0;
int ready_B=0;								//counter is free or not
int ready_C=0;
int ready_D=0;
int curr_token_A=-2;					//token which was last issued to counter,if no token left to assign then -2
int curr_token_B=-2;
int curr_token_C=-2;
int curr_token_D=-2;
int inter=0;									//to signal a interrupt
int MOD=9;										//circle back after 99

void two_digit(int val)
{
	int b=val/10;   				//retrieve unit's place digit
	int a=val-b*10; 				//retrieve ten's place digit
	lcd_write_char(48+b);		//Add 48 to get the ascii value
	lcd_write_char(48+a);
}
void display_counter()
{
	lcd_cmd(0x01);					//clear display
	lcd_cmd(0x80);					//Force cursor to beginning of first line
	
	lcd_write_string("A: ");
	if(curr_token_A!=-2)two_digit(curr_token_A%MOD+1);			// -2 means curr_token_A has processed all its tokens,else process the current token assigned
	else lcd_write_string("  ");
	
	lcd_write_string(" B: ");
	if(curr_token_B!=-2)two_digit(curr_token_B%MOD+1);
	else lcd_write_string("  ");
	
	lcd_cmd(0xC0);
	
	lcd_write_string("C: ");
	if(curr_token_C!=-2)two_digit(curr_token_C%MOD+1);
	else lcd_write_string("  ");

	lcd_write_string(" D: ");
	if(curr_token_D!=-2)two_digit(curr_token_D%MOD+1);
	else lcd_write_string("  ");		
	
}
void display_token()
{
	lcd_cmd(0x01);
	lcd_cmd(0x80);
	lcd_write_string("Token: ");
	two_digit(token%MOD+1);					//display the token
	msdelay(2000);
	display_counter();							//display the counter as current token can be assigned to already free counter  
	
	
}

void new_customer(void)
{
		//called when t key is pressed i.e interrupt occurred.
	 //if a counter has processed all its token and is free then ready_A=1 else 0,assign token_processing  and make ready_A=0
	//increment the token pointer.
	
	if(ready_A==1)
	{
		curr_token_A=token_processing;
		 token_processing=(token_processing+1);
		ready_A=0;
	}
	else if(ready_B==1)
	{
		curr_token_B=token_processing;
		token_processing=(token_processing+1);
		ready_B=0;
	}
	else if(ready_C==1)
	{
		curr_token_C=token_processing;
		token_processing=(token_processing+1);
		ready_C=0;
	}
	else if(ready_D==1)
	{
		curr_token_D=token_processing;
		token_processing=(token_processing+1);
		ready_D=0;
	}
	
	
	token=(token+1);
	
	return ;
	
}

void serial_ISR(void) interrupt 4
{
		if(TI==1)			//check whether TI is set
		{
			TI = 0;			//Clear TI flag
			tx_complete = 1;	//Set tx_complete flag indicating interrupt completion
		}
		else if(RI==1)			//check whether RI is set
		{	
			unsigned char ch;
			ch = SBUF;
			inter=1;
			/*if(ch=='t')
			{new_customer();//issue token and assign to counter if ready
			display_token();}*/
			RI = 0;			//Clear RI flag
			rx_complete = 1;	//Set rx_complete flag indicating interrupt completion
			
		}
}

//Main function
void main(void)
{
	unsigned char ch=0;
	
	//Initialize port P1 for output from P1.7-P1.4
	P1 = 0x0F;
	
	//Call initialization functions
	lcd_init();
	uart_init();
	display_counter();
	//These strings will be printed in terminal software
	transmit_string("************************\r\n");
	transmit_string("******Bank Queue********\r\n");
	transmit_string("************************\r\n");
	transmit_string("Press t for Token\r\n");
	

	
	while(1)
	{
			
		//whenever there is a toggle on pin which is detected using xor (0^1=1^0=1),make the current state_A=pinA ,
		//if token_processing pointer is less than token issued then we have some token left to assign else make the counter ready_A=1 
		//ready to receive as soon as the next token is issued.
		//curr_token_A=-2 because we will display a blank space as it has no token to process.
		inter=0;
		if(pinA^state_A==1)
		{
			state_A=pinA;
			
			if(token_processing<=token)
			{
				curr_token_A=token_processing;
				token_processing+=1;
				
			}
			else {curr_token_A=-2;ready_A=1;}
			display_counter();
			
		}
		if(pinB^state_B==1)
		{
			state_B=pinB;
			
			if(token_processing<=token)
			{
				curr_token_B=token_processing;
				token_processing+=1;
				
			}
			else {curr_token_B=-2;ready_B=1;}
			display_counter();
			
		}
		if(pinC^state_C==1)
		{
			state_C=pinC;
			
			if(token_processing<=token)
			{
				curr_token_C=token_processing;
				token_processing+=1;
				
			}
			else {curr_token_C=-2;ready_C=1;}
			display_counter();
			
		}
		if(pinD^state_D==1)
		{
			state_D=pinD;
			
			if(token_processing<=token)
			{
				curr_token_D=token_processing;
				token_processing+=1;
				
			}
			else {curr_token_D=-2;ready_D=1;}
			display_counter();
			
		}
		
		if(inter==1)
		{	inter=0;
			new_customer();//issue token and assign to counter if ready
			display_token();
		}
		
		
			
	}
}
