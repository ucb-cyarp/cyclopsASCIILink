//
// Created by Christopher Yarp on 10/17/19.
//

#ifndef CYCLOPSASCIILINK_HELPERS_H
#define CYCLOPSASCIILINK_HELPERS_H

#include <stdio.h>
#include <stdbool.h>
#include "cyclopsFraming.h"

bool isReadyForReading(FILE* file);

//Returns the number of elements written
int sendData(FILE* pipe, const TX_SYMBOL_DATATYPE* txPacket, const TX_MODTYPE_DATATYPE* txModMode, int maxLenAvail, TX_GAIN_DATATYPE gain, int maxTokens, int *tokens);

//Returns the number of elements (not blocks) read
int recvData(FILE* pipe, RX_PACKED_DATATYPE* rxPackedData, RX_STROBE_DATATYPE* rxPackedStrobe, RX_PACKED_VALID_DATATYPE* rxPackedValid, RX_PACKED_LAST_DATATYPE* rxPackedLast, int maxBlocks, bool* doneReading);

#endif //CYCLOPSASCIILINK_HELPERS_H
