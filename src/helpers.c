//
// Created by Christopher Yarp on 10/17/19.
//

#include "helpers.h"
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

//We assume each channel has packets of the same length
#ifdef MULTI_CH
int sendData(sharedMemoryFIFO_t* fifo, 
             //Ch0
             const TX_SYMBOL_DATATYPE* txPacket_ch0, 
             const TX_MODTYPE_DATATYPE* txModMode_ch0, 
             //Ch1
             const TX_SYMBOL_DATATYPE* txPacket_ch1, 
             const TX_MODTYPE_DATATYPE* txModMode_ch1, 
             //Ch2
             const TX_SYMBOL_DATATYPE* txPacket_ch2, 
             const TX_MODTYPE_DATATYPE* txModMode_ch2, 
             //Ch3
             const TX_SYMBOL_DATATYPE* txPacket_ch3, 
             const TX_MODTYPE_DATATYPE* txModMode_ch3, 
             int maxLenAvail, 
             TX_GAIN_DATATYPE gain, 
             int maxTokens, 
             int *tokens){
#else
int sendData(sharedMemoryFIFO_t* fifo, 
             //Ch0
             const TX_SYMBOL_DATATYPE* txPacket_ch0, 
             const TX_MODTYPE_DATATYPE* txModMode_ch0, 
             int maxLenAvail, 
             int maxTokens, 
             int *tokens){
#endif

    TX_STRUCTURE_TYPE_NAME txStruct[maxTokens];

    int txCursor = 0;
    int blockInd = 0;

    while(blockInd<maxTokens) {
        //Create packets to send
        #if TX_BLOCK_SIZE == 1
            if(txCursor<maxLenAvail) {
                //NOTE, for BLOCK_SIZE of 1, the structure does not contain arrays
                //Ch0
                txStruct[blockInd].TX_SYMBOL_CH0_MEMBER_NAME = txPacket_ch0[txCursor];
                txStruct[blockInd].TX_MODTYPE_CH0_MEMBER_NAME = txModMode_ch0[txCursor];
                txStruct[blockInd].TX_EN_CH0_MEMBER_NAME = TX_EN_VALID_DATA;
                //Ch1
                #ifdef MULTI_CH
                txStruct[blockInd].TX_SYMBOL_CH1_MEMBER_NAME = txPacket_ch1[txCursor];
                txStruct[blockInd].TX_MODTYPE_CH1_MEMBER_NAME = txModMode_ch1[txCursor];
                txStruct[blockInd].TX_GAIN_CH1_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH1_MEMBER_NAME = TX_ZERO_VALID_DATA;
                //Ch2
                txStruct[blockInd].TX_SYMBOL_CH2_MEMBER_NAME = txPacket_ch2[txCursor];
                txStruct[blockInd].TX_MODTYPE_CH2_MEMBER_NAME = txModMode_ch2[txCursor];
                txStruct[blockInd].TX_GAIN_CH2_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH2_MEMBER_NAME = TX_ZERO_VALID_DATA;
                //Ch3
                txStruct[blockInd].TX_SYMBOL_CH3_MEMBER_NAME = txPacket_ch3[txCursor];
                txStruct[blockInd].TX_MODTYPE_CH3_MEMBER_NAME = txModMode_ch3[txCursor];
                txStruct[blockInd].TX_GAIN_CH3_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH3_MEMBER_NAME = TX_ZERO_VALID_DATA;
                #endif
                txCursor++;
            }else{
                //Ch0
                txStruct[blockInd].TX_SYMBOL_CH0_MEMBER_NAME = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH0_MEMBER_NAME = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_ZERO_CH0_MEMBER_NAME = TX_EN_BLANK_DATA;
                #ifdef MULTI_CH
                //Ch1
                txStruct[blockInd].TX_SYMBOL_CH1_MEMBER_NAME = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH1_MEMBER_NAME = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH1_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH1_MEMBER_NAME = TX_ZERO_BLANK_DATA;
                //Ch2
                txStruct[blockInd].TX_SYMBOL_CH2_MEMBER_NAME = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH2_MEMBER_NAME = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH2_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH2_MEMBER_NAME = TX_ZERO_BLANK_DATA;
                //Ch3
                txStruct[blockInd].TX_SYMBOL_CH3_MEMBER_NAME = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH3_MEMBER_NAME = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH3_MEMBER_NAME = gain;
                txStruct[blockInd].TX_ZERO_CH3_MEMBER_NAME = TX_ZERO_BLANK_DATA;
                #endif
            }
            blockInd++;
        #else
            //May have a partially filled block.  Check for this case
            int blockEnd = txCursor+TX_BLOCK_SIZE;
            int end = maxLenAvail < blockEnd ? maxLenAvail : blockEnd;
            int len = end-txCursor;

            //Fill up valid data
            for(int i = 0; i<len; i++){
                //Ch0
                txStruct[blockInd].TX_SYMBOL_CH0_MEMBER_NAME[i] = txPacket_ch0[txCursor+i];
                txStruct[blockInd].TX_MODTYPE_CH0_MEMBER_NAME[i] = txModMode_ch0[txCursor+i];
                txStruct[blockInd].TX_EN_CH0_MEMBER_NAME [i]= TX_EN_VALID_DATA;
                #ifdef MULTI_CH
                //Ch1
                txStruct[blockInd].TX_SYMBOL_CH1_MEMBER_NAME[i] = txPacket_ch1[txCursor+i];
                txStruct[blockInd].TX_MODTYPE_CH1_MEMBER_NAME[i] = txModMode_ch1[txCursor+i];
                txStruct[blockInd].TX_GAIN_CH1_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH1_MEMBER_NAME[i] = TX_ZERO_VALID_DATA;
                //Ch2
                txStruct[blockInd].TX_SYMBOL_CH2_MEMBER_NAME[i] = txPacket_ch2[txCursor+i];
                txStruct[blockInd].TX_MODTYPE_CH2_MEMBER_NAME[i] = txModMode_ch2[txCursor+i];
                txStruct[blockInd].TX_GAIN_CH2_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH2_MEMBER_NAME[i] = TX_ZERO_VALID_DATA;
                //Ch3
                txStruct[blockInd].TX_SYMBOL_CH3_MEMBER_NAME[i] = txPacket_ch3[txCursor+i];
                txStruct[blockInd].TX_MODTYPE_CH3_MEMBER_NAME[i] = txModMode_ch3[txCursor+i];
                txStruct[blockInd].TX_GAIN_CH3_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH3_MEMBER_NAME[i] = TX_ZERO_VALID_DATA;
                #endif
            }

            //Fill up the tail (if necessary)
            for(int i = len; i<TX_BLOCK_SIZE; i++){
                //Ch0
                txStruct[blockInd].TX_SYMBOL_CH0_MEMBER_NAME[i] = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH0_MEMBER_NAME[i] = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_EN_CH0_MEMBER_NAME[i] = TX_EN_BLANK_DATA;
                #ifdef MULTI_CH
                //Ch1
                txStruct[blockInd].TX_SYMBOL_CH1_MEMBER_NAME[i] = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH1_MEMBER_NAME[i] = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH1_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH1_MEMBER_NAME[i] = TX_ZERO_BLANK_DATA;
                //Ch2
                txStruct[blockInd].TX_SYMBOL_CH2_MEMBER_NAME[i] = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH2_MEMBER_NAME[i] = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH2_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH2_MEMBER_NAME[i] = TX_ZERO_BLANK_DATA;
                //Ch3
                txStruct[blockInd].TX_SYMBOL_CH3_MEMBER_NAME[i] = TX_SYMBOL_BLANK_VAL;
                txStruct[blockInd].TX_MODTYPE_CH3_MEMBER_NAME[i] = TX_MODTYPE_BLANK_VAL;
                txStruct[blockInd].TX_GAIN_CH3_MEMBER_NAME[i] = gain;
                txStruct[blockInd].TX_ZERO_CH3_MEMBER_NAME[i] = TX_ZERO_BLANK_DATA;
                #endif
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

#ifdef MULTI_CH
int recvData(sharedMemoryFIFO_t* fifo, 
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
             int maxBlocks, bool* doneReading){
#else
int recvData(sharedMemoryFIFO_t* fifo, 
             //Ch0
             RX_PACKED_DATATYPE* rxPackedData_ch0, 
             RX_PACKED_VALID_DATATYPE* rxPackedValid_ch0, 
             RX_PACKED_LAST_DATATYPE* rxPackedLast_ch0, 
             int maxBlocks, bool* doneReading){
#endif

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
                //Ch0
                rxPackedData_ch0[ind] = rx.RX_PACKED_CH0_MEMBER_NAME;
                rxPackedValid_ch0[ind] = rx.RX_VALID_CH0_MEMBER_NAME;
                rxPackedLast_ch0[ind] = rx.RX_LAST_CH0_MEMBER_NAME;
                #ifdef MULTI_CH
                //Ch1
                rxPackedData_ch1[ind] = rx.RX_PACKED_CH1_MEMBER_NAME;
                rxPackedStrobe_ch1[ind] = rx.RX_STROBE_CH1_MEMBER_NAME;
                rxPackedValid_ch1[ind] = rx.RX_VALID_CH1_MEMBER_NAME;
                rxPackedLast_ch1[ind] = rx.RX_LAST_CH1_MEMBER_NAME;
                //Ch2
                rxPackedData_ch2[ind] = rx.RX_PACKED_CH2_MEMBER_NAME;
                rxPackedStrobe_ch2[ind] = rx.RX_STROBE_CH2_MEMBER_NAME;
                rxPackedValid_ch2[ind] = rx.RX_VALID_CH2_MEMBER_NAME;
                rxPackedLast_ch2[ind] = rx.RX_LAST_CH2_MEMBER_NAME;
                //Ch3
                rxPackedData_ch3[ind] = rx.RX_PACKED_CH3_MEMBER_NAME;
                rxPackedStrobe_ch3[ind] = rx.RX_STROBE_CH3_MEMBER_NAME;
                rxPackedValid_ch3[ind] = rx.RX_VALID_CH3_MEMBER_NAME;
                rxPackedLast_ch3[ind] = rx.RX_LAST_CH3_MEMBER_NAME;
                #endif
            #else
                //Ch0
                memcpy(rxPackedData_ch0+ind, rx.RX_PACKED_CH0_MEMBER_NAME, sizeof(RX_PACKED_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedValid_ch0+ind, rx.RX_VALID_CH0_MEMBER_NAME, sizeof(RX_PACKED_VALID_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedLast_ch0+ind, rx.RX_LAST_CH0_MEMBER_NAME, sizeof(RX_PACKED_LAST_DATATYPE)*RX_BLOCK_SIZE);
                #ifdef MULTI_CH
                //Ch1
                memcpy(rxPackedData_ch1+ind, rx.RX_PACKED_CH1_MEMBER_NAME, sizeof(RX_PACKED_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedStrobe_ch1+ind, rx.RX_STROBE_CH1_MEMBER_NAME, sizeof(RX_STROBE_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedValid_ch1+ind, rx.RX_VALID_CH1_MEMBER_NAME, sizeof(RX_PACKED_VALID_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedLast_ch1+ind, rx.RX_LAST_CH1_MEMBER_NAME, sizeof(RX_PACKED_LAST_DATATYPE)*RX_BLOCK_SIZE);
                //Ch2
                memcpy(rxPackedData_ch2+ind, rx.RX_PACKED_CH2_MEMBER_NAME, sizeof(RX_PACKED_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedStrobe_ch2+ind, rx.RX_STROBE_CH2_MEMBER_NAME, sizeof(RX_STROBE_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedValid_ch2+ind, rx.RX_VALID_CH2_MEMBER_NAME, sizeof(RX_PACKED_VALID_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedLast_ch2+ind, rx.RX_LAST_CH2_MEMBER_NAME, sizeof(RX_PACKED_LAST_DATATYPE)*RX_BLOCK_SIZE);
                //Ch3
                memcpy(rxPackedData_ch3+ind, rx.RX_PACKED_CH3_MEMBER_NAME, sizeof(RX_PACKED_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedStrobe_ch3+ind, rx.RX_STROBE_CH3_MEMBER_NAME, sizeof(RX_STROBE_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedValid_ch3+ind, rx.RX_VALID_CH3_MEMBER_NAME, sizeof(RX_PACKED_VALID_DATATYPE)*RX_BLOCK_SIZE);
                memcpy(rxPackedLast_ch3+ind, rx.RX_LAST_CH3_MEMBER_NAME, sizeof(RX_PACKED_LAST_DATATYPE)*RX_BLOCK_SIZE);
                #endif
            #endif
            ind+=RX_BLOCK_SIZE;
        }else{
            break;
        }
    }

    return ind;
}