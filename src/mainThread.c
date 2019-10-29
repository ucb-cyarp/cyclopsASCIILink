//
// Created by Christopher Yarp on 10/21/19.
//

#include "mainThread.h"
#include <stdlib.h>
#include <time.h>

#include "helpers.h"
#include "feedbackDefines.h"
#include "demoText.h"
#include "SharedMemoryFIFO.h"

void* mainThread(void* argsUncast){
    threadArgs_t* args = (threadArgs_t*) argsUncast;
    char *txSharedName = args->txSharedName;
    char *txFeedbackSharedName = args->txFeedbackSharedName;
    char *rxSharedName = args->rxSharedName;
    double txPeriod = args->txPeriod;
    int32_t txTokens = args->txTokens;
    int32_t maxBlocksToProcess = args->maxBlocksToProcess;
    int32_t fifoSize = args->fifoSize;
    TX_GAIN_DATATYPE gain = args->gain;

    //Open Pipes (if applicable)
    sharedMemoryFIFO_t txFifo;
    sharedMemoryFIFO_t rxFifo;
    sharedMemoryFIFO_t txfbFifo;

    //Initialize producer FIFOs first
    initSharedMemoryFIFO(&txFifo);
    initSharedMemoryFIFO(&rxFifo);
    initSharedMemoryFIFO(&txfbFifo);

    size_t rxFifoBufferSizeBytes = sizeof(RX_STRUCTURE_TYPE_NAME)*fifoSize;
    size_t txFifoBufferSizeBytes = sizeof(TX_STRUCTURE_TYPE_NAME)*fifoSize;
    size_t txfbFifoBufferSizeBytes = sizeof(FEEDBACK_DATATYPE)*fifoSize;

    if(txSharedName != NULL){
        producerOpenInitFIFO(txSharedName, txFifoBufferSizeBytes, &txFifo);
        consumerOpenFIFOBlock(txFeedbackSharedName, txfbFifoBufferSizeBytes, &txfbFifo);
    }

    if(rxSharedName != NULL){
        consumerOpenFIFOBlock(rxSharedName, rxFifoBufferSizeBytes, &rxFifo);
    }

    TX_SYMBOL_DATATYPE* txPacket;
    TX_MODTYPE_DATATYPE* txModMode;
    int txPacketLen = 0; //In terms of TX_SYMBOL_DATATYPE units
    int msgBytesRead = 0;

    uint8_t txSrc = 0;
    uint8_t txDst = 1;
    uint16_t txNetID = 10;

    //If transmitting, allocate arrays and form a Tx packet
    if(txSharedName != NULL){
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

    //Main Loop
    bool running = true;
    while(running){
        if(txSharedName!=NULL){
            //If transmissions are OK
            if(txTokens > 0) {
                //Check if OK to send

                if(txIndex<txPacketLen){
                    //Check if we are currently sending a packet
                    //Send a packet
                    // printf("In process of sending packet\n");
                    txIndex += sendData(&txFifo, txPacket+txIndex, txModMode+txIndex, txPacketLen-txIndex, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                }else{
                    //Should we be sending
                    time_t currentTime = time(NULL);
                    double duration = difftime(currentTime, lastTxStartTime);
                    if(duration >= txPeriod){
                        lastTxStartTime = currentTime;
                        txIndex = 0;
                        printf("Starting to send packet\n");
                        txIndex += sendData(&txFifo, txPacket, txModMode, txPacketLen, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                    }else{
                        //Write 0's
                        //TODO: Change to a more optimized solution
                        sendData(&txFifo, txPacket, txModMode, 0, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                    }
                }
            }

            for(int i = 0; i<maxBlocksToProcess; i++) {
                //Check for feedback (use select)
                bool feedbackReady = isReadyForReading(&txfbFifo);
                if(feedbackReady){
                    //Get feedback
                    FEEDBACK_DATATYPE tokensReturned;
                    //Once data starts coming, a full transaction should be in process.  Can block on the transaction.
                    int elementsRead = readFifo(&tokensReturned, sizeof(tokensReturned), 1, &txfbFifo);
                    if(elementsRead != 1){
                        //Done!
                        running = false;
                        break;
                    }
                    txTokens += tokensReturned;
                }else{
                    break;
                }
            }
        }

        if(rxSharedName!=NULL){
            //Read and print
            RX_PACKED_DATATYPE rxPackedData[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_STROBE_DATATYPE rxPackedStrobe[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_VALID_DATATYPE rxPackedValid[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast[RX_BLOCK_SIZE*maxBlocksToProcess];

            bool isDoneReading = false;
            int rawElementsRead = recvData(&rxFifo, rxPackedData, rxPackedStrobe, rxPackedValid, rxPackedLast, maxBlocksToProcess, &isDoneReading);
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
            int filteredElements  = filterRepackRxData(rxPackedDataFiltered, rxPackedLastFiltered, rxPackedData, rxPackedLast, rxPackedStrobe, rxPackedValid, rawElementsRead, &remainingPacked, &remainingLast, &remainingBits);
            // if(filteredElements>0){
            //     printf("Number Filtered Elements: %d\n", filteredElements);
            // }

            printPacket(rxPackedDataFiltered, rxPackedLastFiltered, filteredElements, &rxByteInPacket, &rxModMode, &rxType, &rxSrc, &rxDst, &rxNetID, &rxLength, true, &rxCount);
        }
    }



    if(txSharedName != NULL){
        //Cleanup
        cleanupProducer(&txFifo);
        cleanupConsumer(&txfbFifo);

        free(txPacket);
        free(txModMode);
    }

    if(rxSharedName != NULL){
        cleanupConsumer(&rxFifo);
    }

    return NULL;
}