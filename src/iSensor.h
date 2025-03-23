#ifndef I_SENSOR_H
#define I_SENSOR_H

#include "msg.h"



/**
* Gets the input message.
* @param input the input number
* @param message_block the message pointer returned
*/
void getInput(const unsigned int input, volatile MSG_BLOCK* message_block);

#endif
