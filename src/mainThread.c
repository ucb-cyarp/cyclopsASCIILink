//
// Created by Christopher Yarp on 10/21/19.
//

#include "mainThread.h"
#include <stdlib.h>
#include <time.h>

#include "helpers.h"
#include "feedbackDefines.h"
#include "demoText.h"

void* mainThread(void* argsUncast){
    threadArgs_t* args = (threadArgs_t*) argsUncast;
    char *txPipeName = args->txPipeName;
    char *txFeedbackPipeName = args->txFeedbackPipeName;
    char *rxPipeName = args->rxPipeName;
    double txPeriod = args->txPeriod;
    int32_t txTokens = args->txTokens;
    int32_t maxBlocksToProcess = args->maxBlocksToProcess;
    TX_GAIN_DATATYPE gain = args->gain;

    //Open Pipes (if applicable)
    FILE *rxPipe = NULL;
    FILE *txPipe = NULL;
    FILE *txFeedbackPipe = NULL;

    if(rxPipeName != NULL){
        rxPipe= fopen(rxPipeName, "rb");
        printf("Opening Rx Pipe: %s\n", rxPipeName);
        if(rxPipe == NULL) {
            printf("Unable to open Rx Pipe ... exiting\n");
            perror(NULL);
            exit(1);
        }
    }

    if(txPipeName != NULL){
        txPipe = fopen(txPipeName, "wb");
        printf("Opening Tx Pipe: %s\n", txPipeName);
        if(txPipe == NULL){
            printf("Unable to open Tx Pipe ... exiting\n");
            perror(NULL);
            exit(1);
        }

        txFeedbackPipe = fopen(txFeedbackPipeName, "rb");
        printf("Opening Tx Feedback Pipe: %s\n", txFeedbackPipeName);
        if(txFeedbackPipe == NULL) {
            printf("Unable to open Tx Feedback Pipe ... exiting\n");
            perror(NULL);
            exit(1);
        }
    }

    TX_SYMBOL_DATATYPE* txPacket;
    TX_MODTYPE_DATATYPE* txModMode;
    int txPacketLen = 0; //In terms of TX_SYMBOL_DATATYPE units
    int msgBytesRead = 0;

    uint8_t txSrc = 0;
    uint8_t txDst = 1;
    uint16_t txNetID = 10;

    //If transmitting, allocate arrays and form a Tx packet
    if(txPipe != NULL){
        //Craft a packet to send
        txPacket = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);

//        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 4, testText, &msgBytesRead); //Encode a 16QAM Packet
//        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 2, testText, &msgBytesRead); //Encode a QPSK Packet
        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 1, testText, &msgBytesRead); //Encode a BPSK Packet
    }

    //Tx State
    int txIndex = 0; //The current symbol to be transmitted in the packet
    time_t lastTxStartTime = time(NULL); //Will transmit after an initial gap

    //Rx State
    int rxByteInPacket = 0;
    uint8_t rxModMode = 0;
    uint8_t rxType = 0;
    uint8_t rxSrc = 0;
    uint8_t rxDst = 0;
    int16_t rxNetID = 0;
    int16_t rxLength = 0;
    RX_PACKED_DATATYPE remainingPacked = 0;
    RX_PACKED_LAST_DATATYPE remainingLast = false;
    int remainingBits = 0;
    int rxCount = 0;
    int phaseCounter = 0;

    //Main Loop
    bool running = true;
    while(running){
        //==== Tx ====
        if(txPipe!=NULL){
            //If transmissions are OK
            if(txTokens > 0) {
                //Check if OK to send

                if(txIndex<txPacketLen){
                    //Check if we are currently sending a packet
                    //Send a packet
                    // printf("In process of sending packet\n");
                    txIndex += sendData(txPipe, txPacket+txIndex, txModMode+txIndex, txPacketLen-txIndex, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                }else{
                    //Should we be sending
                    time_t currentTime = time(NULL);
                    double duration = difftime(currentTime, lastTxStartTime);
                    if(duration >= txPeriod){
                        lastTxStartTime = currentTime;
                        txIndex = 0;
                        printf("Starting to send packet\n");
                        txIndex += sendData(txPipe, txPacket, txModMode, txPacketLen, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                    }else{
                        //Write 0's
                        //TODO: Change to a more optimized solution
                        sendData(txPipe, txPacket, txModMode, 0, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                    }
                }
            }

            for(int i = 0; i<maxBlocksToProcess; i++) {
                //Check for feedback (use select)
                bool feedbackReady = isReadyForReading(txFeedbackPipe);
                if(feedbackReady){
                    //Get feedback
                    FEEDBACK_DATATYPE tokensReturned;
                    //Once data starts coming, a full transaction should be in process.  Can block on the transaction.
                    int elementsRead = fread(&tokensReturned, sizeof(tokensReturned), 1, txFeedbackPipe);
                    if(elementsRead != 1 && feof(txFeedbackPipe)){
                        //Done!
                        running = false;
                        break;
                    } else if (elementsRead != 1 && ferror(txFeedbackPipe)){
                        printf("An error was encountered while reading the feedback pipe\n");
                        perror(NULL);
                        exit(1);
                    } else if (elementsRead != 1){
                        printf("An unknown error was encountered while reading the feedback pipe\n");
                        exit(1);
                    }
                    txTokens += tokensReturned;
                }else{
                    break;
                }
            }
        }

        //==== Rx ====
        if(rxPipe!=NULL){
            //Read and print
            RX_PACKED_DATATYPE rxPackedData[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_STROBE_DATATYPE rxPackedStrobe[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_VALID_DATATYPE rxPackedValid[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast[RX_BLOCK_SIZE*maxBlocksToProcess];

            bool isDoneReading = false;
            int rawElementsRead = recvData(rxPipe, rxPackedData, rxPackedStrobe, rxPackedValid, rxPackedLast, maxBlocksToProcess, &isDoneReading);
            if(isDoneReading){
                //done reading
                running = false;
            }
            // if(rawElementsRead>0){
            //     //printf("Rx %d elements\n", rawElementsRead);
            //     bool foundValid = false;
            //     for(int i = 0; i<rawElementsRead; i++){
            //         foundValid |= rxPackedValid[i];
            //         // printf("Strobe: %d, Valid: %d, Last %d\n", rxPackedStrobe[i], rxPackedValid[i], rxPackedLast[i]);
            //     }
            //     if(foundValid){
            //         printf("Found Valid\n");
            //     }
            // }

            RX_PACKED_DATATYPE rxPackedDataFiltered[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered[RX_BLOCK_SIZE*maxBlocksToProcess];
            int filteredElements  = filterRepackRxData(rxPackedDataFiltered, rxPackedLastFiltered, rxPackedData, rxPackedLast, rxPackedStrobe, rxPackedValid, rawElementsRead, &remainingPacked, &remainingLast, &remainingBits, &phaseCounter);
            // if(filteredElements>0){
            //     printf("Number Filtered Elements: %d\n", filteredElements);
            // }

            printPacket(rxPackedDataFiltered, rxPackedLastFiltered, filteredElements, &rxByteInPacket, &rxModMode, &rxType, &rxSrc, &rxDst, &rxNetID, &rxLength, true, &rxCount);
        }
    }

    if(txPipe != NULL){
        //Cleanup
        free(txPacket);
        free(txModMode);
    }

    return NULL;
}