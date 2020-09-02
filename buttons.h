#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#include <stdbool.h>
#include <stdint.h>
#include "io_expander.h"

// Four directional buttons correspond to these GPIOB pins on the IO expander
#define UP					0
#define DOWN				1
#define LEFT				2
#define RIGHT				3

// Debounce states
typedef enum 
{
  DEBOUNCE_ONE,
  DEBOUNCE_1ST_ZERO,
  DEBOUNCE_2ND_ZERO,
  DEBOUNCE_PRESSED
} DEBOUNCE_STATES;

extern bool debounce(bool pin_logic_level);

extern bool up_pressed(void);

extern bool down_pressed(void);

extern bool left_pressed(void);

extern bool right_pressed(void);

#endif