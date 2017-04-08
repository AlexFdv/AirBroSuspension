/*
 * HetPinsController.cpp
 *
 *  Created on: 12 бер. 2017 р.
 *      Author: Alex Fadeev
 */

#include "gio.h"
#include "het.h"

#include "HetConstants.h"
#include "HetPinsController.h"

void initializeHetPins()
{
    uint32 openBits = 0x00;
    openBits |= (uint32)1U << FORWARD_LEFT_UP_PIN;
    openBits |= (uint32)1U << FORWARD_LEFT_DOWN_PIN;
    openBits |= (uint32)1U << FORWARD_RIGHT_UP_PIN;
    openBits |= (uint32)1U << FORWARD_RIGHT_DOWN_PIN;

    openBits |= (uint32)1U << BACK_LEFT_UP_PIN;
    openBits |= (uint32)1U << BACK_LEFT_DOWN_PIN;
    openBits |= (uint32)1U << BACK_RIGHT_UP_PIN;
    openBits |= (uint32)1U << BACK_RIGHT_DOWN_PIN;

    gioSetDirection(hetPORT1, openBits);
}

void openPin(uint32 pin)
{
    gioSetBit(hetPORT1, pin, 1);
}

void closePin(uint32 pin)
{
    gioSetBit(hetPORT1, pin, 0);
}
