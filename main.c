// Copyright (c) 2015-19, Joe Krachey
// All rights reserved.
//
// Redistribution and use in source or binary form, with or without modification, 
// are permitted provided that the following conditions are met:
//
// 1. Redistributions in source form must reproduce the above copyright 
//    notice, this list of conditions and the following disclaimer in 
//    the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "main.h"

int Spaceship_X; // X coordinate position of spaceship
int Spaceship_Y; // Y coordinate position of spaceship
int16_t accel_reading; // Holds the accelerometer reading
int delayCount = 0; // Delays movement of the rocket ship
extern volatile bool ADC_Alert; // Alerts when the ADC is interrupting
extern volatile bool touched; //Alerts when touch is held for more than 5ms
extern volatile uint8_t direction; // Tells main what direction to move the current location, values: 1 - Left, 2 - Right, 3 - Up, 4 - Down, 5 - Center
int enemyWaitCurr; // Keeps track of enemy wait
int enemyWaitTotal; // Keeps track of what enemy wait needs to reach to spawn a new enemy
int delayMod; // Changes how fast the enemy moves
uint16_t enemyPositions[10][2]; // Keeps coordinates of enemies
bool enemyPresent[10]; // keep track if an enemy exists here
uint32_t player_score; // keeps track of the players score, +1 for every enemy ship destroyed
uint8_t control_mode; // Keeps track of whether user is playing with accelerometer (1) or joystick (2)
uint16_t touch_y; // position of touch
uint16_t touch_x; // position of touch
uint16_t powerUpScore = 0; // Counts progress to power up
//const uint8_t rocketBitmaps[]; // Global for rocket map
//uint8_t rocketHeightPixels; // Global for rocket height
//uint8_t rocketWidthPixels; // Global for rocket width
uint16_t rocket_color = LCD_COLOR_RED; // Global for rocket color

//*****************************************************************************
//*****************************************************************************
void DisableInterrupts(void)
{
  __asm {
         CPSID  I
  }
}

//*****************************************************************************
//*****************************************************************************
void EnableInterrupts(void)
{
  __asm {
    CPSIE  I
  }
}


//*****************************************************************************
//*****************************************************************************
int 
main(void)
{
		// Initalize randomization
		srand(50);
	
		// Function that initializes the hardware and lcd screen
		project_init();
	
		// Function that initializes the home screen
		home_screen_init();
	
		// Get your mission statement
		instructions_init();
	
		// Select your control mode
		control_init();
	
		// Select your ships color
		color_init();
	
		// Initalize the game
		game_init();
	
		// Start running the game
		gameLogic();
}

//*****************
// This function drives the game logic
//*****************
void gameLogic(void) {
	while(1) {
		// Check how we are updating the game
			if(control_mode == 1) {
				// Check reading of accelerometer and use it to update position of spaceship
				accel_reading = accel_read_x();
				update_position_rocket();
			}
			// Check if joystick is supplying value
			else {
				if(ADC_Alert) {
					ADC_Alert = false;
					update_position_rocket();
				}
			}
			
			delayCount = (delayCount + 1) % delayMod;
			
			// Updates the position of the enemies moving down the screen
			update_position_enemy();
			
			/* Check if the left button is touched to pause
			if(left_pressed()) {
				while(1) {
					if(left_pressed()) {
						break;
					}
				}
			}
			*/
			
			// Check if the players score is a multiple of 20, increase difficulty if it is
			//if(player_score % 20 == 19) {
			//	increase_difficulty();
			//}
			
			// Check if we should shoot based on screen getting touched (shoots automatically on reset)
			if(touched) {
				touched = false;
				touch_y = ft6x06_read_y();
				touch_x = ft6x06_read_x();
				// Check for a power up use, activated when score is over 8 and you touch within bounds of rocket ship
				if(powerUpScore >= 8 && touch_x < Spaceship_X + rocketWidthPixels/2 + 5 && touch_x > Spaceship_X - rocketWidthPixels/2 - 5 && touch_y < Spaceship_Y + rocketHeightPixels/2 + 5 && touch_y > Spaceship_Y - rocketHeightPixels/2 - 5) {
					powerUpScore = 0;
					update_leds();
					powerUp();
				}
				// Else shoot if in LCD range
				else if(0 < touch_y && touch_y < Y && 0 < touch_x && touch_x < X) {
					shoot();
				}
			}
			
			// Spawns in a new enemy
			if(enemyWaitCurr == enemyWaitTotal - 1) {
				create_enemy();
			}
			
			// Update the enemy wait
			enemyWaitCurr = (enemyWaitCurr + 1) % enemyWaitTotal;
	}
}

//************
// This function kills all enemies when a power up is used (powerUpScore > 8)
//************
void powerUp(void) {
	int x;
	// Loop through the enemy arrays and remove all enemies
	for(x = 0; x < 10; x++) {
		if(enemyPresent[x] == true) {
				enemyPresent[x] = false;
				player_score++;
				lcd_draw_image(enemyPositions[x][0], enemyWidthPixels, enemyPositions[x][1], enemyHeightPixels, enemyBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK); //erase ufo
				// Do a quick explosion image in the position of the enemy
				lcd_draw_image(enemyPositions[x][0], explosionWidthPixels, enemyPositions[x][1], explosionHeightPixels, explosionBitmaps, LCD_COLOR_ORANGE, LCD_COLOR_BLACK); //draw explosion
				gp_timer_wait(TIMER1_BASE, 10000000);
				lcd_draw_image(enemyPositions[x][0], explosionWidthPixels, enemyPositions[x][1], explosionHeightPixels, explosionBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK); //erase explosion
			}
		}
}

//************
// This function increases the difficulty of your game every score multiple of 20
//************
/*
void increase_difficulty(void) {
	// Difficulty updates differ based on control_mode
	if(control_mode == 1) {
		enemyWaitCurr = enemyWaitCurr - 200; // Initializes as 9999
		enemyWaitTotal = enemyWaitTotal - 200; // Initializes as 10000
		delayMod = delayMod - 5; // Initializes as 200
	}
	else {
		enemyWaitCurr = enemyWaitCurr - 2000; // Initializes as 99999
		enemyWaitTotal = enemyWaitTotal - 2000; // Initializes as 100000
		delayMod = delayMod - 25; // Initializes as 1000
	}
}
*/

//*********
// This function updates the leds based on a kill
//*********
void update_leds(void) {
	// Light up the number of leds based on the current power up score
	switch(powerUpScore) {
		case 0:
			io_expander_write_reg(0x12, 0);
			break;
		case 1: 
			io_expander_write_reg(0x12, D607);
			break;
		case 2:
			io_expander_write_reg(0x12, D607 | D606);
			break;
		case 3:
			io_expander_write_reg(0x12, D607 | D606 | D605);
			break;
		case 4:
			io_expander_write_reg(0x12, D607 | D606 | D605 | D604);
			break;
		case 5:
			io_expander_write_reg(0x12, D607 | D606 | D605 | D604 | D603);
			break;
		case 6:
			io_expander_write_reg(0x12, D607 | D606 | D605 | D604 | D603 | D602);
			break;
		case 7:
			io_expander_write_reg(0x12, D607 | D606 | D605 | D604 | D603 | D602 | D601);
			break;
		case 8:
			io_expander_write_reg(0x12, D607 | D606 | D605 | D604 | D603 | D602 | D601 | D600);
			break;
		default:
			break;
	}
}

//*************
// This function shoots the laser across the screen, checking if an enemy is present
//*************
void shoot(void) {
	uint8_t x;
	uint16_t laserY = 290;
	uint8_t laserX = Spaceship_X;
	uint16_t laserStop;
	bool hit = false;
	
	// Loop through the enemy arrays to see if it was a hit
	for(x = 0; x < 10; x++) {
		if(enemyPresent[x] == true) {
			// if spaceship got a hit:
			if((enemyPositions[x][0] + enemyWidthPixels/2) >= laserX && (enemyPositions[x][0] - enemyWidthPixels/2) <= laserX) {
				hit = true;
				player_score++;
				powerUpScore++;
				// Upates the LEDs for the power up
				update_leds();
				enemyPresent[x] = false;
				laserStop = enemyPositions[x][1] + enemyHeightPixels; // makes the laser cut the UFO in half
				for(laserY = 290; laserY > laserStop; laserY= laserY - laserHeightPixels) //draws laser path
				{
					lcd_draw_image(laserX, laserWidthPixels, laserY, laserHeightPixels, laserBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK); //draw laser at base
					gp_timer_wait(TIMER1_BASE, 125000); //this does cause the game to pause
					lcd_draw_image(laserX, laserWidthPixels, laserY, laserHeightPixels, laserBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);	//erase laser
				}
				lcd_draw_image(enemyPositions[x][0], enemyWidthPixels, enemyPositions[x][1], enemyHeightPixels, enemyBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK); //erase ufo
				// Do a quick explosion image in the position of the enemy
				lcd_draw_image(enemyPositions[x][0], explosionWidthPixels, enemyPositions[x][1], explosionHeightPixels, explosionBitmaps, LCD_COLOR_ORANGE, LCD_COLOR_BLACK); //draw explosion
				gp_timer_wait(TIMER1_BASE, 1000000);
				lcd_draw_image(enemyPositions[x][0], explosionWidthPixels, enemyPositions[x][1], explosionHeightPixels, explosionBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK); //erase explosion
			
			}
		}
		 
	}
	
	// If no enemies hit draw a laser shooting across the screen
	if(!hit) {
		for(laserY = 290; laserY > 0; laserY= laserY - laserHeightPixels) {
				lcd_draw_image(laserX, laserWidthPixels, laserY, laserHeightPixels, laserBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK); //draw laser at base
				gp_timer_wait(TIMER1_BASE, 100000);
				lcd_draw_image(laserX, laserWidthPixels, laserY, laserHeightPixels, laserBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);	//erase laser
		}
	}
	
}

//*******
// Controls enemys moving down the screen
//*******
void update_position_enemy(void) {
	if((delayCount == delayMod - 1 && control_mode == 1) || (delayCount == delayMod - 1 && control_mode == 2)) {
		uint8_t i;
		// Loop through the enemy array looking for true positions and move the enemy down by 1
		for(i = 0; i < 10; ++i) {
			if(enemyPresent[i] == true) {
				// Check if at the bottom boundary to end the game
				if(enemyPositions[i][1] == Y - 10) {
					endGame();
				}
				// Otherwise update the enemies position
				else {
					lcd_draw_image(enemyPositions[i][0], enemyWidthPixels, enemyPositions[i][1], enemyHeightPixels, enemyBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
					enemyPositions[i][1] = enemyPositions[i][1] + 1;
					lcd_draw_image(enemyPositions[i][0], enemyWidthPixels, enemyPositions[i][1], enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
				}
			}
		}
	}
}

//*******
// This functions places a new enemy on the screen
//*******
void create_enemy(void) {
	uint8_t i;
	uint16_t random_position;
	// Loop through the enemy array for an open position
	for(i = 0; i < 10; ++i) {
		if(enemyPresent[i] == false) {
			enemyPresent[i] = true;
			// Pick a new random enemy X position
			random_position = (rand() % (X - 20)) + 10;
			
			// Set the enemies x and y positions and draw it on the screen
			enemyPositions[i][0] = random_position;
			enemyPositions[i][1] = ENEMY_Y_POSITION;
			lcd_draw_image(enemyPositions[i][0], enemyWidthPixels, enemyPositions[i][1], enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
			break;
		}
	}
}

//******
// This function initializes the lcd screen and the hardware
//******
void project_init(void) {
	// Call the function that initializes the hardware
	initialize_hardware();
	
	// Configure the LCD screen
	lcd_config_screen();
	lcd_clear_screen(LCD_COLOR_BLACK);
}

//**********
// This function initializes the game board
//**********
void game_init(void) {
	// Clear the screen
	lcd_clear_screen(LCD_COLOR_BLACK);
	
	// Set the coordinates of the space ship
	Spaceship_X = X/2;
	Spaceship_Y = Y - 20;
	
	// Draw the rocket at the bottom
	lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, rocket_color, LCD_COLOR_BLACK);
}

//*********
// This function updates the current position of the rocketship
//*********
void update_position_rocket(void) {
	// Accelerometer update
	if(control_mode == 1) {
		if((delayCount % 100 == 99)) {
			// Tilted left
			if(accel_reading > 1500) {
				// Make sure its within bounds of the screen
				if(Spaceship_X > 15) {
					// Erase old image and update X and draw new image
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
					Spaceship_X--;
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, rocket_color, LCD_COLOR_BLACK);
				}
			}
			// Tilted right
			else if(accel_reading < -1500) {
				// Make sure its within the bounds of the screen
				if(Spaceship_X < X - 15) {
					// Erase old image and update X and draw new image
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
					Spaceship_X++;
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, rocket_color, LCD_COLOR_BLACK);
				}
			}
		}
	}
	// Joystick update
	else {
		// Joystick left
			if(direction == 1) {
				// Make sure its within bounds of the screen
				if(Spaceship_X > 15) {
					// Erase old image and update X and draw new image
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
					Spaceship_X--;
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, rocket_color, LCD_COLOR_BLACK);
				}
			}
			// Joystick right
			else if(direction == 2) {
				// Make sure its within the bounds of the screen
				if(Spaceship_X < X - 15) {
					// Erase old image and update X and draw new image
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
					Spaceship_X++;
					lcd_draw_image(Spaceship_X, rocketWidthPixels, Spaceship_Y, rocketHeightPixels, rocketBitmaps, rocket_color, LCD_COLOR_BLACK);
				}
			}
	}
}

//*************
// This function sets up the home screen
//*************
void home_screen_init(void) {
	// Draw the homescreen on the LCD
	lcd_draw_image(X/2, starConquestWidthPixels, Y/4, starConquestHeightPixels, starConquestBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	lcd_draw_image(30, rocketWidthPixels, 50, rocketHeightPixels, rocketBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	lcd_draw_image(X - 30, enemyWidthPixels, 52, enemyHeightPixels, enemyBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	lcd_draw_image(X/2, touchToStartWidthPixels, Y - 40, touchToStartHeightPixels, touchToStartBitmaps, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	lcd_draw_image(X/2, planetWidthPixels, Y - 120, planetHeightPixels, planetBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK);
	
	while(1) {
		// Check if the user touched to start the game
		if(touched) {
			touched = false;
			touch_y = ft6x06_read_y();
			touch_x = ft6x06_read_x();
			if(touch_x < X/2 + touchToStartWidthPixels/2 + 30 && touch_x > X/2 - touchToStartWidthPixels/2 - 30 && touch_y < Y - 40 + touchToStartHeightPixels/2 + 30 && touch_y > Y - 40 - touchToStartHeightPixels/2 - 30) {
				break;
			}
		}
		
		// Create a flashing affect for the touch to start
		lcd_draw_image(X/2, touchToStartWidthPixels, Y - 40, touchToStartHeightPixels, touchToStartBitmaps, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
		gp_timer_wait(TIMER1_BASE, 20000000);
		lcd_draw_image(X/2, touchToStartWidthPixels, Y - 40, touchToStartHeightPixels, touchToStartBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
		gp_timer_wait(TIMER1_BASE, 20000000);
		
	}
}

//*********
// Initalize the control selection screen
//*********
void control_init(void) {
	// Initalize the control screen
	lcd_clear_screen(LCD_COLOR_BLACK);
	lcd_draw_image(X/2, controlMethodWidthPixels, 40, controlMethodHeightPixels, controlMethodBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK);
	lcd_draw_image(X/2, accelWidthPixels, 140, accelHeightPixels, accelBitmaps, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	lcd_draw_image(X/2, joystickWidthPixels, 240, joystickHeightPixels, joystickBitmaps, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
	
	// Wait for the user to pick a control method
	while(1){
		if(touched) {
			touched = false;
			touch_y = ft6x06_read_y();
			touch_x = ft6x06_read_x();
			// Check if accelerometer was touched, setting the control mode
			if(touch_x < X/2 + accelWidthPixels/2 + 5 && touch_x > X/2 - accelWidthPixels/2 - 5 && touch_y < 140 + accelHeightPixels/2 + 5 && touch_y > 140 - accelHeightPixels/2 - 5) {
				control_mode = 1;
				enemyWaitCurr = 9999;
				enemyWaitTotal = 10000; 
				delayMod = 200;
				break;
			}
			// Check if joystick was touched, setting the control mode
			else if(touch_x < X/2 + joystickWidthPixels/2 + 5 && touch_x > X/2 - joystickWidthPixels/2 - 5 && touch_y < 240 + joystickHeightPixels/2 + 5 && touch_y > 240 - joystickHeightPixels/2 - 5) {
				control_mode = 2;
				enemyWaitCurr = 99999;
				enemyWaitTotal = 100000; 
				delayMod = 1000;
				break;
			}
		}
	}
}

//**************
// This function initializes and waits for a color selection from the user
//**************
void color_init(void) {
	//coordinate for center of each circle:
	uint8_t c_1_x = 3*X/4;
	uint8_t c_1_y = Y/4 + 35;
	uint8_t c_2_x = X/4;
	uint8_t c_2_y = Y/4 + 35;
	uint8_t c_3_x = 3*X/4;
	uint8_t c_3_y = 3*Y/4;
	uint8_t c_4_x = X/4;
	uint8_t c_4_y = 3*Y/4;
	
	lcd_clear_screen(LCD_COLOR_BLACK);
	
	// Draw the circles with the different colors
	lcd_draw_image(X/2, choose_colorWidthPixels, Y/8, choose_colorHeightPixels, choose_colorBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	lcd_draw_image(c_1_x, circleWidthPixels, c_1_y , circleHeightPixels, circleBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK); 
	lcd_draw_image(c_2_x, circleWidthPixels, c_2_y , circleHeightPixels, circleBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK);
	lcd_draw_image(c_3_x, circleWidthPixels, c_3_y , circleHeightPixels, circleBitmaps, LCD_COLOR_YELLOW, LCD_COLOR_BLACK);
	lcd_draw_image(c_4_x, circleWidthPixels, c_4_y , circleHeightPixels, circleBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	
	// Wait for the user to pick a color
	while(1) {
		if(touched){
			touched = false;
			touch_y = ft6x06_read_y();
			touch_x = ft6x06_read_x();
			if( (touch_x < c_1_x +20) && (touch_x > c_1_x -20) && (touch_y < c_1_y+20 )&& (touch_y > c_1_y-20) )//top left
			{
				rocket_color = LCD_COLOR_RED;
				break;
			}
			if( (touch_x < c_2_x +20) && (touch_x > c_2_x -20) && (touch_y < c_2_y+20 )&& (touch_y > c_2_y-20) )//top left
			{
				rocket_color = LCD_COLOR_BLUE;
				break;
			}
			if( (touch_x < c_3_x +20) && (touch_x > c_3_x -20) && (touch_y < c_3_y+20 )&& (touch_y > c_3_y-20) )//bottom right
			{
				rocket_color = LCD_COLOR_YELLOW;
				break;
			}
			if( (touch_x < c_4_x +20) && (touch_x > c_4_x -20) && (touch_y < c_4_y+20 )&& (touch_y > c_4_y-20) )//bottom left
			{
				rocket_color = LCD_COLOR_GREEN;
				break;
			}
		}
	}
	
}

//****************
// This function initalizes the instruction screen
//****************
void instructions_init(void) {
	lcd_clear_screen(LCD_COLOR_BLACK);
	lcd_draw_image(X/2, yourMissionWidthPixels, Y/2, yourMissionHeightPixels, yourMissionBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	while(1) {
		// Check if the user touched to continue the game
		if(touched) {
			touched = false;
			touch_y = ft6x06_read_y();
			touch_x = ft6x06_read_x();
			if(touch_x < X/2 + continueWidthPixels/2 + 40 && touch_x > X/2 - continueWidthPixels/2 - 40 && touch_y < Y - 40 + continueHeightPixels/2 + 40 && touch_y > Y - 40 - continueHeightPixels/2 - 40) {
				break;
			}
		}
		
		// Create a flashing affect for the touch to continue
		lcd_draw_image(X/2, continueWidthPixels, Y - 40, continueHeightPixels, continueBitmaps, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
		gp_timer_wait(TIMER1_BASE, 20000000);
		lcd_draw_image(X/2, continueWidthPixels, Y - 40, continueHeightPixels, continueBitmaps, LCD_COLOR_BLACK, LCD_COLOR_BLACK);
		gp_timer_wait(TIMER1_BASE, 20000000);
	}
}

//**********
// This function launches when an enemy hits the bottom of the screen, ending the game
//**********
void endGame(void) {
	lcd_clear_screen(LCD_COLOR_BLACK);
	lcd_draw_image(X/2, endgameWidthPixels, 90, endgameHeightPixels, endgameBitmaps, LCD_COLOR_RED, LCD_COLOR_BLACK);
	lcd_draw_image(X/2, planetWidthPixels, 3*Y/4, planetHeightPixels, planetBitmaps, LCD_COLOR_BLUE, LCD_COLOR_BLACK);
	lcd_draw_image(X - 30, enemyWidthPixels, 3*Y/4 + 15 , enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(30, enemyWidthPixels, 3*Y/4 + 15, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(X - 30, enemyWidthPixels, 3*Y/4 - 15, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(30, enemyWidthPixels, 3*Y/4 - 15, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(X - 50, enemyWidthPixels, 3*Y/4 + 30, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(50, enemyWidthPixels, 3*Y/4 + 30, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(X - 50, enemyWidthPixels, 3*Y/4 - 30, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	lcd_draw_image(50, enemyWidthPixels, 3*Y/4 - 30, enemyHeightPixels, enemyBitmaps, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
	printf("Your Score: %i", player_score); 
	while(1) {
	}
}
