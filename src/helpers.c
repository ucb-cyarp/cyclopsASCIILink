//
// Created by Christopher Yarp on 10/17/19.
//

#include "helpers.h"
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

int sendData(sharedMemoryFIFO_t* fifo, const TX_SYMBOL_DATATYPE* txPacket, const TX_MODTYPE_DATATYPE* txModMode, int maxLenAvail, TX_GAIN_DATATYPE gain, int maxTokens, int *tokens){
    TX_STRUCTURE_TYPE_NAME txStruct[maxTokens];

    int txCursor = 0;
    int blockInd = 0;

    while(blockInd<maxTokens) {
        //Create packets to send
        #if TX_BLOCK_SIZE == 1
            if(txCursor<maxLenAvail) {
                //NOTE, for BLOCK_SIZE of 1, the structure does not contain arrays
                txStruct[blockInd].TX_SYMBOL_MEMBER_NAME = txPacket[txCursor];
                txStruct[blockInd].TX_MODTYPE_MEMBER_NAME = txModMode[txCursor];
                txStruct[blockInd].TX_GAIN_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_MEMBER_NAME = TX_ZERO_VALID_DATA;
                txCursor++;
            }else{
                txStruct[blockInd].TX_SYMBOL_MEMBER_NAME = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_MEMBER_NAME = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_MEMBER_NAME = TX_ZERO_BLANK_DATA;
            }
            blockInd++;
        #else
            //May have a partially filled block.  Check for this case
            int blockEnd = txCursor+TX_BLOCK_SIZE;
            int end = maxLenAvail < blockEnd ? maxLenAvail : blockEnd;
            int len = end-txCursor;

            //Fill up valid data
            for(int i = 0; i<len; i++){
                txStruct[blockInd].TX_SYMBOL_MEMBER_NAME[i] = txPacket[txCursor+i];
                txStruct[blockInd].TX_MODTYPE_MEMBER_NAME[i] = txModMode[txCursor+i];
                txStruct[blockInd].TX_GAIN_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_MEMBER_NAME [i]= TX_ZERO_VALID_DATA;
            }

            //Fill up the tail (if necessary)
            for(int i = len; i<TX_BLOCK_SIZE; i++){
                txStruct[blockInd].TX_SYMBOL_MEMBER_NAME[i] = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_MEMBER_NAME[i] = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_MEMBER_NAME[i] = TX_ZERO_BLANK_DATA;
            }

            blockInd++;
            txCursor+=len;
        #endif
    }

    //Write to pipe
    int elementsWritten = writeFifo(txStruct, sizeof(TX_STRUCTURE_TYPE_NAME), blockInd, fifo);
    if (elementsWritten != blockInd){
        printf("An error was encountered while writing\n");
        perror(NULL);
        exit(1);
    }

    //Subtract the tokens
    *tokens -= blockInd;
    return txCursor;
}

int recvData(sharedMemoryFIFO_t* fifo, RX_PACKED_DATATYPE* rxPackedData, RX_STROBE_DATATYPE* rxPackedStrobe, RX_PACKED_VALID_DATATYPE* rxPackedValid, RX_PACKED_LAST_DATATYPE* rxPackedLast, int maxBlocks, bool* doneReading){
    int ind = 0;
    for(int i = 0; i<maxBlocks; i++) {
        //Check for input (use select)
        bool inputReady = isReadyForReading(fifo);
        if(inputReady){
            RX_STRUCTURE_TYPE_NAME rx;
            int bytesRead = readFifo(&rx, sizeof(RX_STRUCTURE_TYPE_NAME), 1, fifo);
            if(bytesRead != 1){
                //Done with input (input pipe closed)
                *doneReading = true;
                break;
            }

            #if RX_BLOCK_SIZE == 1
                rxPackedData[ind] = rx.RX_PACKED_MEMBER_NAME;
                rxPackedStrobe[ind] = rx.RX_STROBE_MEMBER_NAME;
                rxPackedValid[ind] = rx.RX_VALID_MEMBER_NAME;
                rxPackedLast[ind] = rx.RX_LAST_MEMBER_NAME;
            #else
                memcpy(rxPackedData+ind, rx.RX_PACKED_MEMBER_NAME, sizeof(RX_PACKED_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedStrobe+ind, rx.RX_STROBE_MEMBER_NAME, sizeof(RX_STROBE_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedValid+ind, rx.RX_VALID_MEMBER_NAME, sizeof(RX_PACKED_VALID_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedLast+ind, rx.RX_LAST_MEMBER_NAME, sizeof(RX_PACKED_LAST_DATATYPE)*RX_BLOCK_SIZE);
            #endif
            ind+=RX_BLOCK_SIZE;
        }else{
            break;
        }
    }

    return ind;
}