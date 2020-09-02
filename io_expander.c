#include "io_expander.h"

bool io_expander_init(void) {
	// configure i2c
	if (gpio_enable_port(IO_EXPANDER_GPIO_BASE) == false) {
		return false;
	}
	
	// Configure SCL 
  if(gpio_config_digital_enable(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_alternate_function(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SCL_PIN)== false)
  {
    return false;
  }
    
  if(gpio_config_port_control (IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SCL_PCTL_M, IO_EXPANDER_I2C_SCL_PIN_PCTL)== false)
  {
    return false;
  }
	
	// configure sda
	if (gpio_config_digital_enable(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SDA_PIN) == false) {
		return false;
	}
  if (gpio_config_open_drain(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SDA_PIN) == false) {
		return false;
	}
  if (gpio_config_alternate_function(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SDA_PIN ) == false) {
		return false;
	}
  if (gpio_config_port_control(IO_EXPANDER_GPIO_BASE, IO_EXPANDER_I2C_SDA_PCTL_M, IO_EXPANDER_I2C_SDA_PIN_PCTL) == false) {
		return false;
	}
	
	// initialize i2c
	initializeI2CMaster(IO_EXPANDER_I2C_BASE);
	
	// enable leds as outputs
	io_expander_write_reg(MCP23017_IODIRA_R, 0x00);
	
	// initialize buttons
	io_expander_write_reg(MCP23017_IODIRB_R, 0xFF);
	io_expander_write_reg(MCP23017_GPPUB_R, 0xF);
	return true;
}

void io_expander_write_reg(uint8_t reg, uint8_t data) {
	// check if i2c busy
	while (I2CMasterBusy(IO_EXPANDER_I2C_BASE)) {};
	// set i2c slave addr
	i2cSetSlaveAddr(IO_EXPANDER_I2C_BASE, MCP23017_DEV_ID, I2C_WRITE);
	// send address
	i2cSendByte(IO_EXPANDER_I2C_BASE, reg, I2C_MCS_START | I2C_MCS_RUN);
	// send data
	i2cSendByte(IO_EXPANDER_I2C_BASE, data, I2C_MCS_RUN | I2C_MCS_STOP);
}

void io_expander_read_reg(uint8_t *data){
	// check if i2c busy
	while (I2CMasterBusy(IO_EXPANDER_I2C_BASE)) {};
	// set slave address
	i2cSetSlaveAddr(IO_EXPANDER_I2C_BASE, MCP23017_DEV_ID, I2C_WRITE);
	// send address 
	i2cSendByte(IO_EXPANDER_I2C_BASE, MCP23017_GPIOB_R, I2C_MCS_START | I2C_MCS_RUN);
	// set to read mode 
	i2cSetSlaveAddr(IO_EXPANDER_I2C_BASE, MCP23017_DEV_ID, I2C_READ);
	// read data
	i2cGetByte(IO_EXPANDER_I2C_BASE, data, I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_STOP);
}
