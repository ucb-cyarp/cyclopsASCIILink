//
// Created by Christopher Yarp on 10/21/19.
//

#include "mainThread.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "helpers.h"
#include "feedbackDefines.h"
#include "demoText.h"

#define TX_ID_MAX 64

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

    //Ch0
    TX_SYMBOL_DATATYPE* txPacket_ch0;
    TX_MODTYPE_DATATYPE* txModMode_ch0;
    int txPacketLen_ch0 = 1; //In terms of TX_SYMBOL_DATATYPE units
    //Ch1
    TX_SYMBOL_DATATYPE* txPacket_ch1;
    TX_MODTYPE_DATATYPE* txModMode_ch1;
    int txPacketLen_ch1 = 1; //In terms of TX_SYMBOL_DATATYPE units
    //Ch2
    TX_SYMBOL_DATATYPE* txPacket_ch2;
    TX_MODTYPE_DATATYPE* txModMode_ch2;
    int txPacketLen_ch2 = 1; //In terms of TX_SYMBOL_DATATYPE units
    //Ch3
    TX_SYMBOL_DATATYPE* txPacket_ch3;
    TX_MODTYPE_DATATYPE* txModMode_ch3;
    int txPacketLen_ch3 = 1; //In terms of TX_SYMBOL_DATATYPE units
    
    int msgBytesRead = 0;
    uint8_t txSrc = 0;
    uint8_t txDst = 1;
    uint16_t txNetID = 10;

    char* blankStr = "";

    //If transmitting, allocate arrays and form a Tx packet
    if(txPipe != NULL){
        txPacket_ch0 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch0 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txPacket_ch1 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch1 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txPacket_ch2 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch2 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txPacket_ch3 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch3 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);

//        txPacketLen_ch0 =  createRawCyclopsFrame(txPacket_ch0, txModMode_ch0, txSrc, txDst, txNetID, 4, testText, &msgBytesRead); //Encode a 16QAM Packet
//        txPacketLen_ch0 =  createRawCyclopsFrame(txPacket_ch0, txModMode_ch0, txSrc, txDst, txNetID, 2, testText, &msgBytesRead); //Encode a QPSK Packet
    }

    //Tx State
    int txIndex = txPacketLen_ch0; //The current symbol to be transmitted in the packet
    time_t lastTxStartTime = time(NULL); //Will transmit after an initial gap

    int txStrLoc = 0; //This is the location we are currently at when sending the txStr
    const char* txStr = testTextLong;
    int txStrLen = strlen(txStr);

    int txID = 0; //This is the id number used to order packets

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

                if(txIndex<txPacketLen_ch0){
                    //Check if we are currently sending a packet
                    //Send a packet
                    // printf("In process of sending packet\n");
                    txIndex += sendData(txPipe, 
                                            txPacket_ch0+txIndex, 
                                            txModMode_ch0+txIndex, 
                                            txPacket_ch1+txIndex, 
                                            txModMode_ch1+txIndex, 
                                            txPacket_ch2+txIndex, 
                                            txModMode_ch2+txIndex, 
                                            txPacket_ch3+txIndex, 
                                            txModMode_ch3+txIndex, 
                                            txPacketLen_ch0-txIndex, 
                                            gain, 
                                            maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                            &txTokens);
                }else{
                    //Should we be sending
                    time_t currentTime = time(NULL);
                    double duration = difftime(currentTime, lastTxStartTime);
                    if(duration >= txPeriod){
                        lastTxStartTime = currentTime;
                        txIndex = 0;
                        printf("Starting to send packet\n");
                        //Create a new packet
                        txPacketLen_ch0 = createRawCyclopsFrame(txPacket_ch0, txModMode_ch0, txID, txID, txID, 1, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        //MsgBytesRead indicates how many bytes of the textToBeEncoded were read.  Add this to the text ptr passed to the next one
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        txPacketLen_ch1 = createRawCyclopsFrame(txPacket_ch1, txModMode_ch1, txID, txID, txID, 1, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        txPacketLen_ch2 = createRawCyclopsFrame(txPacket_ch2, txModMode_ch2, txID, txID, txID, 1, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        txPacketLen_ch3 = createRawCyclopsFrame(txPacket_ch3, txModMode_ch3, txID, txID, txID, 1, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        if(txPacketLen_ch0 != txPacketLen_ch1 || txPacketLen_ch0 != txPacketLen_ch2 || txPacketLen_ch0 != txPacketLen_ch3){
                            printf("Encountered an error generating packets");
                            exit(1);
                        }

                        //TODO: Will assume packets all have the same length.  Change this later
                        //Check packet lengths match
                        txIndex += sendData(txPipe, 
                                            txPacket_ch0, 
                                            txModMode_ch0, 
                                            txPacket_ch1, 
                                            txModMode_ch1, 
                                            txPacket_ch2, 
                                            txModMode_ch2, 
                                            txPacket_ch3, 
                                            txModMode_ch3, 
                                            txPacketLen_ch0, 
                                            gain, 
                                            maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                            &txTokens);
                    }else{
                        //Write 0's
                        //TODO: Change to a more optimized solution
                        sendData(txPipe, 
                                txPacket_ch0, 
                                txModMode_ch0, 
                                txPacket_ch1, 
                                txModMode_ch1, 
                                txPacket_ch2, 
                                txModMode_ch2, 
                                txPacket_ch3, 
                                txModMode_ch3, 
                                0, 
                                gain, 
                                maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                &txTokens);
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
        free(txPacket_ch0);
        free(txModMode_ch0);
        free(txPacket_ch1);
        free(txModMode_ch1);
        free(txPacket_ch2);
        free(txModMode_ch2);
        free(txPacket_ch3);
        free(txModMode_ch3);
    }

    return NULL;
}