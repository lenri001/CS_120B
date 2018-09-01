#include <avr/io.h>
#include <avr/interrupt.h>
#include "bit.h"
#include "timer.h"
#include <stdio.h>
#include "io.c"
#include "keypad.h"
#include "scheduler.h"

#define FIRST_ROW 4
#define SECOND_ROW 10
#define ARRAY_LENGTH 16

enum SM_LOC_STATES { SM_LOC_START, SM_LOC_INIT, SM_LOC_WAIT, SM_LOC_LOSE,SM_LOC_WIN};
enum FIRST_ROW_STATES { FIRST_ROW_START, FIRST_ROW_INIT, FIRST_ROW_WAIT, FIRST_ROW_LOSE };
enum SECOND_ROW_STATES { SECOND_ROW_START, SECOND_ROW_INIT, SECOND_ROW_WAIT, SECOND_ROW_LOSE };
enum GAME_PLAY_STATES { GAMEPLAY_START, GAMEPLAY_INIT, GAMEPLAY_WAIT, GAMEPLAY_RESET };
enum MOTOR_STATES {MOTOR_START, MOTOR_WAIT_FOWARD, MOTOT_WAIT_BACKWARDS, MOTOR_ENDS};

const unsigned char FIRST_ROW_[] = "     #           ";
const unsigned char SECOND_ROW_[] = "           #    ";

// Global Varibles
unsigned Points = 0;
unsigned char ARRAY_LOC = 0;
unsigned char RESET = 0;
unsigned char WIN = 0;
unsigned char LOSE = 0;
unsigned char FIRST_FLAG = 0;
unsigned char BOTTOM_FLAG = 0;


int TickFct_SM_LOC(int state) {

	// Transitions
	switch(state) {
		case SM_LOC_START:
		state = SM_LOC_INIT;
		break;
		case SM_LOC_INIT:
		ARRAY_LOC = 0;
		state = SM_LOC_WAIT;
		break;
		case SM_LOC_WAIT:
		if(Points > 20)
		{
			state = SM_LOC_WIN;
			WIN =1;
			PORTA = 0x01; //TEMP FIX FOR MOTOR
		}

		if (LOSE) {
			state = SM_LOC_LOSE;
		}

		else if (ARRAY_LOC >= ARRAY_LENGTH){
			ARRAY_LOC = 0;
		}
		break;


		case SM_LOC_LOSE:
		if(!LOSE) state = SM_LOC_INIT;
		break;

		case SM_LOC_WIN:
		state = SM_LOC_START;
		break;

		default:
		state = SM_LOC_START; break;
	}

	// Actions
	switch(state) {
		case SM_LOC_WAIT:
		if (!RESET)
		{
			ARRAY_LOC++;
		}
		break;

		case SM_LOC_WIN:
		LCD_ClearScreen();
		LCD_DisplayString(1, "You WIn: ");


		break;
		default: break;
	}

	return state;
}

//iterates char in firsr row
int TickFct_FIRST_ROW_(int state) {
	static unsigned char i = 0;
	static unsigned chat j = 0;
	// Transitions
	switch(state) {
		case FIRST_ROW_START:
		state = FIRST_ROW_INIT;
		break;
		case FIRST_ROW_INIT:
		i = 1;
		j = ARRAY_LOC;
		state = FIRST_ROW_WAIT;
		break;
		case FIRST_ROW_WAIT:
		if(LOSE) {
			state = FIRST_ROW_LOSE;
			j = 0;
			FIRST_FLAG = 0;
			i = 1;
		}

		else if (i > ARRAY_LENGTH) {
			state = FIRST_ROW_INIT;
		}
		else {
			state = FIRST_ROW_WAIT;
		}


		break;
		case FIRST_ROW_LOSE:
		if(i <= ARRAY_LENGTH) {
			i++; j++;
		}
		else if(!LOSE){
			state = FIRST_ROW_INIT;
		}
		else {
			state = FIRST_ROW_INIT;
		}
		break;
		default:
		state = FIRST_ROW_START; break;
	}

	// Actions
	switch(state) {
		case FIRST_ROW_WAIT:
		LCD_Cursor(i);
		LCD_WriteData(FIRST_ROW_[j % ARRAY_LENGTH]);
		i++; j++;
		if (ARRAY_LOC == FIRST_ROW)
		{
			FIRST_FLAG = 1;
		}

		else
		{
			FIRST_FLAG = 0;
		}
		break;
		default: break;
	}

	return state;
}

// Iterates second row
int TickFct_SECOND_ROW_(int state) {
	static unsigned char i = 0;
	static unsigned char j = 0;

	// Transitions
	switch(state) {
		case SECOND_ROW_START:
		state = SECOND_ROW_INIT;
		break;
		case SECOND_ROW_INIT:
		i = 17;
		j = ARRAY_LOC;
		state = SECOND_ROW_WAIT;
		break;
		case SECOND_ROW_WAIT:
		if(LOSE) {
			state = SECOND_ROW_LOSE;
			j = BOTTOM_FLAG = 0;
			i = 17;
		}
		else if (i > ARRAY_LENGTH*2) {

			state = SECOND_ROW_INIT;
		}
		else {
			SECOND_ROW_WAIT;
		}
		break;

		case SECOND_ROW_LOSE:
		if(i <= ARRAY_LENGTH*2) {
			LCD_Cursor(i);
			LCD_DisplayString(1, "You loose: ");
			LCD_WriteData(Points/5+1 +'0' );

			i++; j++;
		}
		else if(!LOSE) state = SECOND_ROW_INIT;
		break;
		default:
		state = SECOND_ROW_START; break;
	}

	// Actions
	switch(state) {
		case SECOND_ROW_WAIT:
		LCD_Cursor(i);
		LCD_WriteData(SECOND_ROW_[j % ARRAY_LENGTH]);
		i++; j++;
		if (ARRAY_LOC == SECOND_ROW)
		{
			BOTTOM_FLAG = 1;
			Points++;
		}
		else BOTTOM_FLAG = 0;
		break;
		default: break;
	}

	return state;
}

int TickFct_GAME_PLAY(int state) {
	static unsigned char i = 0;
	static unsigned char BUTTON_PRESS = 0;
	BUTTON_PRESS = ~PINB;

	// Transitions
	switch(state) {
		case GAMEPLAY_START:
		state = GAMEPLAY_INIT;
		break;
		case GAMEPLAY_INIT:
		i = 2;
		RESET = 1;
		LOSE = 0;
		state = GAMEPLAY_WAIT;
		break;
		case GAMEPLAY_WAIT:

		// UP BUTTON
		if((BUTTON_PRESS & 0x01) && !RESET) {
			i = 2;

		}
		// DOWN BUTTON
		else if ((BUTTON_PRESS & 0x02) && !RESET) {
			i = 18;

		}
		// RESET
		else if (BUTTON_PRESS & 0x04) {
			RESET = (RESET) ? 0 : 1;
			state = GAMEPLAY_RESET;
			if (LOSE)
			{
				LOSE = 0;
			}
		}
		else {
			i = 2;
		}

		if ((i == 2 && FIRST_FLAG) || (i == 18 && BOTTOM_FLAG)) { // THE CURSOR LOCATION AND FLAG MEET
			RESET = LOSE = 1;
			state = GAMEPLAY_RESET;

		}
		break;
		case GAMEPLAY_RESET:
		if (BUTTON_PRESS ^ 0x04) {
			state = GAMEPLAY_WAIT;
		}
		break;
		default:
		state = GAMEPLAY_START; break;
	}

	// Actions
	switch(state) {
		case GAMEPLAY_WAIT:
		if (!LOSE) {
			LCD_Cursor(i);
		}
		else {
			// will display clock
		}
		break;

		default:
		break;
	}
	return state;
}

int TickFct_MOTOR(int state) {
	/////****************UNDER_CONSTRCUTION***********//////////////////
	// Transitions
	switch(state) {
		case MOTOR_START:
		if(WIN)
		{
			state = MOTOR_WAIT_FOWARD;
		}
		else
		{
			state = MOTOR_START;
		}
		break;
		case MOTOR_WAIT_FOWARD:
		state = MOTOT_WAIT_BACKWARDS;
		break;
		case MOTOT_WAIT_BACKWARDS:
		state =  MOTOR_ENDS;
		break;
		case MOTOR_ENDS:
		state = MOTOR_START;
		break;
	}

	// Actions
	switch(state) {
		case MOTOR_START:
		break;
		case MOTOR_WAIT_FOWARD:
		PORTA = 0x01;
		break;
		case MOTOT_WAIT_BACKWARDS:
		PORTA = 0x02;
		break;
		case MOTOR_ENDS:
		WIN = 0;
		break;
	}

	return state;
}



int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;	// MOTOR
	DDRB = 0x00; PORTB = 0xFF;  // INPUTS
	DDRC = 0xFF; PORTC = 0x00;  // LCD
	DDRD = 0xFF; PORTD = 0x00;  // LCD


	// SCHEDULER SET UP
	unsigned long int SM_LOC_Task_calc = 500;
	unsigned long int FIRST_ROW_Task_calc = 50;
	unsigned long int SECOND_ROW_Task_calc = 50;
	unsigned long int GAME_PLAY_Task_calc = 50;
	unsigned long int MOTOR_Task_calc = 2000;

	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SM_LOC_Task_calc, FIRST_ROW_Task_calc);
	tmpGCD = findGCD(tmpGCD, SECOND_ROW_Task_calc);
	tmpGCD = findGCD(tmpGCD, GAME_PLAY_Task_calc);
	tmpGCD = findGCD(tmpGCD, MOTOR_Task_calc);

	unsigned long int GCD = tmpGCD;


	unsigned long int SM_LOC_period = SM_LOC_Task_calc/GCD;
	unsigned long int FIRST_ROW_period = FIRST_ROW_Task_calc/GCD;
	unsigned long int SECOND_ROW_period = SECOND_ROW_Task_calc/GCD;
	unsigned long int GAME_PLAY_period = GAME_PLAY_Task_calc/GCD;
	unsigned long int MOTOR_period = MOTOR_Task_calc/GCD;

	static task SM_LOC_Task, FIRST_ROW_Task, SECOND_ROW_Task, GAME_PLAY_Task,MOTOR_Task ;
	task *tasks[] = { &SM_LOC_Task, &FIRST_ROW_Task, &SECOND_ROW_Task, &GAME_PLAY_Task, &MOTOR_Task };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	SM_LOC_Task.state = SM_LOC_START;
	SM_LOC_Task.period = SM_LOC_period;
	SM_LOC_Task.elapsedTime = SM_LOC_period;
	SM_LOC_Task.TickFct = &TickFct_SM_LOC;

	FIRST_ROW_Task.state = FIRST_ROW_START;
	FIRST_ROW_Task.period = FIRST_ROW_period;
	FIRST_ROW_Task.elapsedTime = FIRST_ROW_period;
	FIRST_ROW_Task.TickFct = &TickFct_FIRST_ROW_;

	SECOND_ROW_Task.state = SECOND_ROW_START;
	SECOND_ROW_Task.period = SECOND_ROW_period;
	SECOND_ROW_Task.elapsedTime = SECOND_ROW_period;
	SECOND_ROW_Task.TickFct = &TickFct_SECOND_ROW_;

	GAME_PLAY_Task.state = GAMEPLAY_START;
	GAME_PLAY_Task.period = GAME_PLAY_period;
	GAME_PLAY_Task.elapsedTime = GAME_PLAY_period;
	GAME_PLAY_Task.TickFct = &TickFct_GAME_PLAY;

	MOTOR_Task.state = MOTOR_START;
	MOTOR_Task.period = MOTOR_period;
	MOTOR_Task.elapsedTime = MOTOR_period;
	MOTOR_Task.TickFct = &TickFct_MOTOR;

	LCD_init();
	LCD_ClearScreen();

	TimerSet(GCD);
	TimerOn();

	unsigned short i;

	while(1)
	{

		// scheduler loop
		for(i = 0; i < numTasks; i++) {
			// task is ready to tick
			if(tasks[i]->elapsedTime == tasks[i]->period) {
				// setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// reset the elapsed time for the next tick
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		if(WIN == 1)
		{
			LCD_ClearScreen();
			LCD_DisplayString(1, "You WIN: "); // WILL FIX LATER SHOULD BE DONE IN TICK FUNCTION
			LCD_WriteData((Points/5-3) +'0' );
			PORTA = 0x01;
			while(!TimerFlag);      // wait for a period
			TimerFlag = 0;          // reset TimerFlag
			return(0);
		}
		while(!TimerFlag);      // wait for a period
		TimerFlag = 0;          // reset TimerFlag
	}
}
