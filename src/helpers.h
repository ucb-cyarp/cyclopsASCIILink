//
// Created by Christopher Yarp on 10/17/19.
//

#ifndef CYCLOPSASCIILINK_HELPERS_H
#define CYCLOPSASCIILINK_HELPERS_H

#include <stdio.h>
#include <stdbool.h>
#include "cyclopsFraming.h"
#include "vitisStructure.h"

bool isReadyForReading(FILE* file);

//Returns the number of elements (not blocks) written
#ifdef MULTI_CH
int sendData(FILE* pipe, 
             const TX_SYMBOL_DATATYPE* txPacket_ch0, 
             const TX_MODTYPE_DATATYPE* txModMode_ch0, 
             const TX_SYMBOL_DATATYPE* txPacket_ch1, 
             const TX_MODTYPE_DATATYPE* txModMode_ch1, 
             const TX_SYMBOL_DATATYPE* txPacket_ch2, 
             const TX_MODTYPE_DATATYPE* txModMode_ch2, 
             const TX_SYMBOL_DATATYPE* txPacket_ch3, 
             const TX_MODTYPE_DATATYPE* txModMode_ch3, 
             int maxLenAvail, 
             TX_GAIN_DATATYPE gain, 
             int maxTokens, 
             int *tokens);
#else
int sendData(FILE* pipe, 
             const TX_SYMBOL_DATATYPE* txPacket_ch0, 
             const TX_MODTYPE_DATATYPE* txModMode_ch0, 
             int maxLenAvail, 
             int maxTokens, 
             int *tokens);
#endif

//Returns the number of elements (not blocks) read
#ifdef MULTI_CH
int recvData(FILE* pipe, 
             //Ch0
             RX_PACKED_DATATYPE* rxPackedData_ch0, 
             RX_STROBE_DATATYPE* rxPackedStrobe_ch0, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch0, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch0, 
             //Ch1
             RX_PACKED_DATATYPE* rxPackedData_ch1, 
             RX_STROBE_DATATYPE* rxPackedStrobe_ch1, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch1, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch1, 
             //Ch2
             RX_PACKED_DATATYPE* rxPackedData_ch2, 
             RX_STROBE_DATATYPE* rxPackedStrobe_ch2, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch2, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch2, 
             //Ch3
             RX_PACKED_DATATYPE* rxPackedData_ch3, 
             RX_STROBE_DATATYPE* rxPackedStrobe_ch3, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch3, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch3, 
             int maxBlocks, bool* doneReading);
#else
int recvData(FILE* pipe, 
             //Ch0
             RX_PACKED_DATATYPE* rxPackedData_ch0, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch0, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch0, 
             int maxBlocks, bool* doneReading);
#endif

#endif //CYCLOPSASCIILINK_HELPERS_H
