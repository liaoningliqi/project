

#include "peripheral_i2c.h"

// I2c_CTRL0
void I2C_CTRL0_SET(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL0.SET = data;
}

void I2C_CTRL0_CLR(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL0.CLR = data;
}

void I2C_CTRL0_TOG(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL0.TOG = data;
}


// I2c_CTRL1
void I2C_CTRL1_SET(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL1.SET = data;
}

void I2C_CTRL1_CLR(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL1.CLR = data;
}

void I2C_CTRL1_TOG(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_CTRL1.TOG = data;
}

// GET I2C_CTRL1_DATA_ENGINE_CMPLT_IRQ
uint8_t GET_I2C_CTRL1_DATA_ENGINE_CMPLT_IRQ(I2C_TypeDef *I2C_BASE)
{
	return((I2C_BASE->I2C_CTRL1.NRM >> bsI2C_CTRL1_DATA_ENGINE_CMPLT_IRQ) & BW2M(bwI2C_CTRL1_DATA_ENGINE_CMPLT_IRQ));
}

// I2C_QUEUECTRL
void I2C_QUEUECTRL_SET(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECTRL.SET = data;
}

void I2C_QUEUECTRL_CLR(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECTRL.CLR = data;
}

void I2C_QUEUECTRL_TOG(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECTRL.TOG = data;
}

// I2C_QUEUECMD
void I2C_QUEUECMD_SET(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECMD.SET = data;
}

void I2C_QUEUECMD_CLR(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECMD.CLR = data;
}

void I2C_QUEUECMD_TOG(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_QUEUECMD.TOG = data;
}

// I2C_DEBUG0
void I2C_DEBUG0_SET(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_DEBUG0.SET = data;
}

void I2C_DEBUG0_CLR(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_DEBUG0.CLR = data;
}

void I2C_DEBUG0_TOG(I2C_TypeDef *I2C_BASE, uint32_t data)
{
	I2C_BASE->I2C_DEBUG0.TOG = data;
}

// GET I2C_DEBUG0_DMAREQ
uint8_t GET_I2C_DEBUG0_DMAREQ(I2C_TypeDef *I2C_BASE)
{
	return((I2C_BASE->I2C_DEBUG0.NRM >> bsI2C_DEBUG0_DMAREQ) & BW2M(bwI2C_DEBUG0_DMAREQ));
}


