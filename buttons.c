#include "buttons.h"

bool debounce(bool pin_logic_level) {
	
	static DEBOUNCE_STATES state = DEBOUNCE_ONE;
	
  switch (state)
  {
    case DEBOUNCE_ONE:
    {
      if(pin_logic_level)
        state = DEBOUNCE_ONE;
      else
        state = DEBOUNCE_1ST_ZERO;
      break;
    }
    case DEBOUNCE_1ST_ZERO:
    {
      if(pin_logic_level)
        state = DEBOUNCE_ONE;
      else
        state = DEBOUNCE_2ND_ZERO;
      break;
    }
    case DEBOUNCE_2ND_ZERO:
    {
      if(pin_logic_level)
        state = DEBOUNCE_ONE;
      else
        state = DEBOUNCE_PRESSED;
      break;
    }
    case DEBOUNCE_PRESSED:
    {
      if(pin_logic_level)
        state = DEBOUNCE_ONE;
      else
        state = DEBOUNCE_PRESSED;
      break;
    }
    default:
    {
      while(1){};
    }
  }

  if(state == DEBOUNCE_2ND_ZERO )
    return true;
  else
    return false;
}

bool up_pressed(void){
	uint8_t pin_logic_level;
	io_expander_read_reg(&pin_logic_level);
  pin_logic_level &= 1<<UP;
  return debounce(pin_logic_level);
}

bool down_pressed(void){
	uint8_t pin_logic_level;
	io_expander_read_reg(&pin_logic_level);
  pin_logic_level &= 1<<DOWN;
  return debounce(pin_logic_level);
}

bool left_pressed(void){
	uint8_t pin_logic_level;
	io_expander_read_reg(&pin_logic_level);
  pin_logic_level &= 1<<LEFT;
  return debounce(pin_logic_level);
}

bool right_pressed(void){
	uint8_t pin_logic_level;
	io_expander_read_reg(&pin_logic_level);
  pin_logic_level &= 1<<RIGHT;
  return debounce(pin_logic_level);
}