//
// Created by Christopher Yarp on 10/17/19.
//

#include "helpers.h"
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

bool isReadyForReading(FILE* file){
    //See http://man7.org/linux/man-pages/man2/select.2.html for info on using select
    //See https://stackoverflow.com/questions/3167298/how-can-i-convert-a-file-pointer-file-fp-to-a-file-descriptor-int-fd for getting a fd from a FILE*

    int fileFD = fileno(file);
    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(fileFD, &fdSet);
    int maxFD = fileFD;

    //Timeout quickly
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;

    int selectStatus = pselect(maxFD+1, &fdSet, NULL, NULL, &timeout, NULL);
    if(selectStatus == -1){
        fprintf(stderr, "Error while checking if a file is ready for reading\n");
        perror(NULL);
        exit(1);
    }
    return FD_ISSET(fileFD, &fdSet);
}

int sendData(FILE* pipe, const TX_SYMBOL_DATATYPE* txPacket, const TX_MODTYPE_DATATYPE* txModMode, int maxLenAvail, TX_GAIN_DATATYPE gain, int maxTokens, int *tokens){
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
    int elementsWritten = fwrite(txStruct, sizeof(TX_STRUCTURE_TYPE_NAME), blockInd, pipe);
    fflush(pipe);
    if (elementsWritten != blockInd && ferror(pipe)){
        printf("An error was encountered while writing\n");
        perror(NULL);
        exit(1);
    } else if (elementsWritten != blockInd){
        printf("An unknown error was encountered while writing\n");
        exit(1);
    }

    //Subtract the tokens
    *tokens -= blockInd;
    return blockInd;
}

int recvData(FILE* pipe, RX_PACKED_DATATYPE* rxPackedData, RX_STROBE_DATATYPE* rxPackedStrobe, RX_PACKED_VALID_DATATYPE* rxPackedValid, RX_PACKED_LAST_DATATYPE* rxPackedLast, int maxBlocks, bool* doneReading){
    int ind = 0;
    for(int i = 0; i<maxBlocks; i++) {
        //Check for input (use select)
        bool inputReady = isReadyForReading(pipe);
        if(inputReady){
            RX_STRUCTURE_TYPE_NAME rx;
            int bytesRead = fread(&rx, sizeof(RX_STRUCTURE_TYPE_NAME), 1, pipe);
            if(bytesRead != 1 && feof(pipe)){
                //Done with input (input pipe closed)
                *doneReading = true;
                break;
            } else if (bytesRead != 1 && ferror(pipe)){
                printf("An error was encountered while reading Rx pipe\n");
                perror(NULL);
                exit(1);
            } else if (bytesRead != 1){
                printf("An unknown error was encountered while reading Rx pipe\n");
                exit(1);
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