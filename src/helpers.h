//
// Created by Christopher Yarp on 10/17/19.
//

#ifndef CYCLOPSASCIILINK_HELPERS_H
#define CYCLOPSASCIILINK_HELPERS_H

#include <stdio.h>
#include <stdbool.h>
#include "cyclopsFraming.h"
#include "vitisStructure.h"

#ifdef CYCLOPS_ASCII_SHARED_MEM
#include "depends/BerkeleySharedMemoryFIFO.h"
#else
bool isReadyForReading(FILE* file);
#endif

//Returns the number of elements (not blocks) written
//Also returns the number of blank elements (not blocks) written
typedef struct sendRtn {
    int elementsSent;
    int blanksSent;
} sendRtn_t ;

#ifdef MULTI_CH
#ifdef CYCLOPS_ASCII_SHARED_MEM
sendRtn_t sendData(sharedMemoryFIFO_t* fifo,
#else
sendRtn_t sendData(FILE* pipe,
#endif
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
#ifdef CYCLOPS_ASCII_SHARED_MEM
sendRtn_t sendData(sharedMemoryFIFO_t* fifo,
#else
sendRtn_t sendData(FILE* pipe,
#endif
             const TX_SYMBOL_DATATYPE* txPacket_ch0, 
             const TX_MODTYPE_DATATYPE* txModMode_ch0, 
             int maxLenAvail, 
             int maxTokens, 
             int *tokens);
#endif

#ifdef CYCLOPS_ASCII_SHARED_MEM
int sendBlank(sharedMemoryFIFO_t* fifo,
#else
int sendBlank(FILE* pipe,
#endif
              int blanksRequested,
              int maxTokens,
              int *tokens);

//Returns the number of elements (not blocks) read
#ifdef MULTI_CH
#ifdef CYCLOPS_ASCII_SHARED_MEM
int recvData(sharedMemoryFIFO_t* fifo, 
#else
int recvData(FILE* pipe, 
#endif
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
#ifdef CYCLOPS_ASCII_SHARED_MEM
int recvData(sharedMemoryFIFO_t* fifo, 
#else
int recvData(FILE* pipe, 
#endif
             //Ch0
             RX_PACKED_DATATYPE* rxPackedData_ch0, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch0, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch0, 
             int maxBlocks, bool* doneReading);
#endif

#endif //CYCLOPSASCIILINK_HELPERS_H
