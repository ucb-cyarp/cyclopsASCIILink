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
#include "depends/BerkeleySharedMemoryFIFO.h"

#define TX_ID_MAX (64)
#define RX_PACKETS_TO_STORE_PER_CH (5)
#define RX_MAX_FAILURES (10)

#define PRINT_RX_TITLE (true)
#define PRINT_RX_DETAILS (true)
#define PRINT_RX_CONTENT (true)

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
        // printf("Opened Tx FIFO: %s\n", txSharedName);
        consumerOpenFIFOBlock(txFeedbackSharedName, txfbFifoBufferSizeBytes, &txfbFifo);
        // printf("Opened TxFB FIFO: %s\n", txFeedbackSharedName);
    }

    if(rxSharedName != NULL){
        consumerOpenFIFOBlock(rxSharedName, rxFifoBufferSizeBytes, &rxFifo);
        // printf("Opened Rx FIFO: %s\n", rxSharedName);
    }

    //Ch0
    TX_SYMBOL_DATATYPE* txPacket_ch0;
    TX_MODTYPE_DATATYPE* txModMode_ch0;
    int txPacketLen_ch0 = 0; //In terms of TX_SYMBOL_DATATYPE units
    #ifdef MULTI_CH
    //Ch1
    TX_SYMBOL_DATATYPE* txPacket_ch1;
    TX_MODTYPE_DATATYPE* txModMode_ch1;
    int txPacketLen_ch1 = 0; //In terms of TX_SYMBOL_DATATYPE units
    //Ch2
    TX_SYMBOL_DATATYPE* txPacket_ch2;
    TX_MODTYPE_DATATYPE* txModMode_ch2;
    int txPacketLen_ch2 = 0; //In terms of TX_SYMBOL_DATATYPE units
    //Ch3
    TX_SYMBOL_DATATYPE* txPacket_ch3;
    TX_MODTYPE_DATATYPE* txModMode_ch3;
    int txPacketLen_ch3 = 0; //In terms of TX_SYMBOL_DATATYPE units
    #endif
    
    int msgBytesRead = 0;
    uint8_t txSrc = 0;
    uint8_t txDst = 1;
    uint16_t txNetID = 10;

    int txID = 0; //This is the id number used to order packets

    char* blankStr = "";

    //If transmitting, allocate arrays and form a Tx packet
    if(txSharedName != NULL){
        txPacket_ch0 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch0 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        #ifdef MULTI_CH
        txPacket_ch1 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch1 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txPacket_ch2 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch2 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txPacket_ch3 = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode_ch3 = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        #endif

        //Create a temporary packet to be used when sending a blank signal
        //TODO: Optimize this
        txPacketLen_ch0 = createRawCyclopsFrame(txPacket_ch0, txModMode_ch0, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, blankStr, &msgBytesRead);
        #ifdef MULTI_CH
        txPacketLen_ch1 = createRawCyclopsFrame(txPacket_ch1, txModMode_ch1, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, blankStr, &msgBytesRead);
        txPacketLen_ch2 = createRawCyclopsFrame(txPacket_ch2, txModMode_ch2, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, blankStr, &msgBytesRead);
        txPacketLen_ch3 = createRawCyclopsFrame(txPacket_ch3, txModMode_ch3, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, blankStr, &msgBytesRead);
        #endif
    }

    //Tx State
    int txIndex = txPacketLen_ch0; //The current symbol to be transmitted in the packet
    time_t lastTxStartTime = time(NULL); //Will transmit after an initial gap

    int txStrLoc = 0; //This is the location we are currently at when sending the txStr
    const char* txStr = testTextLong;
    int txStrLen = strlen(txStr);

    //Rx State
    rx_decoder_state_t rx_decoder_state_ch0 = {0, 0, 0, 0, 0, 0, 0, 0, NULL};
    #ifdef MULTI_CH
    rx_decoder_state_t rx_decoder_state_ch1 = {0, 0, 0, 0, 0, 0, 0, 0, NULL};
    rx_decoder_state_t rx_decoder_state_ch2 = {0, 0, 0, 0, 0, 0, 0, 0, NULL};
    rx_decoder_state_t rx_decoder_state_ch3 = {0, 0, 0, 0, 0, 0, 0, 0, NULL};
    #endif

    rx_packet_t rx_packet_buffer_ch0[RX_PACKETS_TO_STORE_PER_CH];
    #ifdef MULTI_CH
    rx_packet_t rx_packet_buffer_ch1[RX_PACKETS_TO_STORE_PER_CH];
    rx_packet_t rx_packet_buffer_ch2[RX_PACKETS_TO_STORE_PER_CH];
    rx_packet_t rx_packet_buffer_ch3[RX_PACKETS_TO_STORE_PER_CH];
    #endif

    packet_buffer_state_t rx_packet_buffer_state_ch0 = {rx_packet_buffer_ch0, RX_PACKETS_TO_STORE_PER_CH, 0, 0};
    #ifdef MULTI_CH
    packet_buffer_state_t rx_packet_buffer_state_ch1 = {rx_packet_buffer_ch1, RX_PACKETS_TO_STORE_PER_CH, 0, 0};
    packet_buffer_state_t rx_packet_buffer_state_ch2 = {rx_packet_buffer_ch2, RX_PACKETS_TO_STORE_PER_CH, 0, 0};
    packet_buffer_state_t rx_packet_buffer_state_ch3 = {rx_packet_buffer_ch3, RX_PACKETS_TO_STORE_PER_CH, 0, 0};
    #endif

    int currentID = 0;
    int currentBuffer = 0;
    int failureCount = 0;

    #ifdef MULTI_CH
    packet_buffer_state_t*  rx_packet_buffer_states[] = {&rx_packet_buffer_state_ch0,
                                                         &rx_packet_buffer_state_ch1,
                                                         &rx_packet_buffer_state_ch2,
                                                         &rx_packet_buffer_state_ch3};
    #else
    packet_buffer_state_t*  rx_packet_buffer_states[] = {&rx_packet_buffer_state_ch0};
    #endif


    //Ch0
    RX_PACKED_DATATYPE remainingPacked_ch0 = 0;
    RX_PACKED_LAST_DATATYPE remainingLast_ch0 = false;
    int remainingBits_ch0 = 0;
    int phaseCounter_ch0 = 0;
    #ifdef MULTI_CH
    //Ch1
    RX_PACKED_DATATYPE remainingPacked_ch1 = 0;
    RX_PACKED_LAST_DATATYPE remainingLast_ch1 = false;
    int remainingBits_ch1 = 0;
    int phaseCounter_ch1 = 0;
    //Ch2
    RX_PACKED_DATATYPE remainingPacked_ch2 = 0;
    RX_PACKED_LAST_DATATYPE remainingLast_ch2 = false;
    int remainingBits_ch2 = 0;
    int phaseCounter_ch2 = 0;
    //Ch3
    RX_PACKED_DATATYPE remainingPacked_ch3 = 0;
    RX_PACKED_LAST_DATATYPE remainingLast_ch3 = false;
    int remainingBits_ch3 = 0;
    int phaseCounter_ch3 = 0;
    #endif

    //Main Loop
    bool running = true;
    while(running){
        //==== Tx ====
        if(txSharedName!=NULL){
            //If transmissions are OK
            if(txTokens > 0) {
                //Check if OK to send

                if(txIndex<txPacketLen_ch0){
                    //Check if we are currently sending a packet
                    //Send a packet
                    // printf("In process of sending packet\n");
                    #ifdef MULTI_CH
                    txIndex += sendData(&txFifo, 
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
                    #else
                    txIndex += sendData(&txFifo, 
                                            txPacket_ch0+txIndex, 
                                            txModMode_ch0+txIndex, 
                                            txPacketLen_ch0-txIndex, 
                                            maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                            &txTokens);
                    #endif
                }else{
                    //Should we be sending
                    time_t currentTime = time(NULL);
                    double duration = difftime(currentTime, lastTxStartTime);
                    if(duration >= txPeriod){
                        lastTxStartTime = currentTime;
                        txIndex = 0;
                        printf("\nStarting to send packet\n");
                        //Create a new packet
                        txPacketLen_ch0 = createRawCyclopsFrame(txPacket_ch0, txModMode_ch0, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        //MsgBytesRead indicates how many bytes of the textToBeEncoded were read.  Add this to the text ptr passed to the next one
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        #ifdef MULTI_CH

                        txPacketLen_ch1 = createRawCyclopsFrame(txPacket_ch1, txModMode_ch1, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        txPacketLen_ch2 = createRawCyclopsFrame(txPacket_ch2, txModMode_ch2, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
                        txStrLoc+=msgBytesRead;
                        txID = txID<TX_ID_MAX-1 ? txID+1 : 0;
                        if(txStrLoc>=txStrLen){
                            //Got to the end of the string, wrap around
                            txStrLoc = 0;
                        }

                        txPacketLen_ch3 = createRawCyclopsFrame(txPacket_ch3, txModMode_ch3, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
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
                        #endif

                        //TODO: Will assume packets all have the same length.  Change this later
                        //Check packet lengths match
                        #ifdef MULTI_CH
                        txIndex += sendData(&txFifo, 
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
                        #else
                        txIndex += sendData(&txFifo, 
                                            txPacket_ch0, 
                                            txModMode_ch0, 
                                            txPacketLen_ch0, 
                                            maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                            &txTokens);
                        #endif
                    }else{
                        //Write 0's
                        //TODO: Change to a more optimized solution
                        #ifdef MULTI_CH
                        sendData(&txFifo, 
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
                        #else
                        sendData(&txFifo, 
                                txPacket_ch0, 
                                txModMode_ch0, 
                                0, 
                                maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                &txTokens);
                        #endif
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
                    // printf("Recieving Feedback Data\n");
                    int elementsRead = readFifo(&tokensReturned, sizeof(tokensReturned), 1, &txfbFifo);
                    // printf("Recieving Feedback Data\n");
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

        //==== Rx ====
        if(rxSharedName!=NULL){
            //Read and print
            //Ch0
            RX_PACKED_DATATYPE rxPackedData_ch0[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_VALID_DATATYPE rxPackedValid_ch0[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast_ch0[RX_BLOCK_SIZE*maxBlocksToProcess];
            #ifdef MULTI_CH
            //Ch1
            RX_PACKED_DATATYPE rxPackedData_ch1[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_STROBE_DATATYPE rxPackedStrobe_ch1[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_VALID_DATATYPE rxPackedValid_ch1[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast_ch1[RX_BLOCK_SIZE*maxBlocksToProcess];
            //Ch2
            RX_PACKED_DATATYPE rxPackedData_ch2[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_STROBE_DATATYPE rxPackedStrobe_ch2[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_VALID_DATATYPE rxPackedValid_ch2[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast_ch2[RX_BLOCK_SIZE*maxBlocksToProcess];
            //Ch3
            RX_PACKED_DATATYPE rxPackedData_ch3[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_STROBE_DATATYPE rxPackedStrobe_ch3[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_VALID_DATATYPE rxPackedValid_ch3[RX_BLOCK_SIZE*maxBlocksToProcess];
            RX_PACKED_LAST_DATATYPE rxPackedLast_ch3[RX_BLOCK_SIZE*maxBlocksToProcess];
            #endif

            bool isDoneReading = false;
            #ifdef MULTI_CH
            int rawElementsRead = recvData(&rxFifo,
                                           //Ch0 
                                           rxPackedData_ch0, 
                                           rxPackedStrobe_ch0, 
                                           rxPackedValid_ch0, 
                                           rxPackedLast_ch0, 
                                           //Ch1 
                                           rxPackedData_ch1, 
                                           rxPackedStrobe_ch1, 
                                           rxPackedValid_ch1, 
                                           rxPackedLast_ch1, 
                                           //Ch2 
                                           rxPackedData_ch2, 
                                           rxPackedStrobe_ch2, 
                                           rxPackedValid_ch2, 
                                           rxPackedLast_ch2, 
                                           //Ch3 
                                           rxPackedData_ch3, 
                                           rxPackedStrobe_ch3, 
                                           rxPackedValid_ch3, 
                                           rxPackedLast_ch3, 
                                           maxBlocksToProcess, &isDoneReading);
            #else
            int rawElementsRead = recvData(&rxFifo,
                                           //Ch0 
                                           rxPackedData_ch0, 
                                           rxPackedValid_ch0, 
                                           rxPackedLast_ch0, 
                                           maxBlocksToProcess, &isDoneReading);
            #endif

            if(isDoneReading){
                //done reading
                running = false;
            }
            // if(rawElementsRead>0){
            //     //printf("Rx %d elements\n", rawElementsRead);
            //     bool foundValid = false;
            //     for(int i = 0; i<rawElementsRead; i++){
            //         foundValid |= rxPackedValid_ch0[i];
            //         // printf("Strobe: %d, Valid: %d, Last %d\n", rxPackedStrobe_ch0[i], rxPackedValid_ch0[i], rxPackedLast_ch0[i]);
            //     }
            //     if(foundValid){
            //         printf("Found Valid\n");
            //     }
            // }

            //Ch0
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch0[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch0[RX_BLOCK_SIZE*maxBlocksToProcess];
            #ifdef MULTI_CH
            //Ch1
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch1[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch1[RX_BLOCK_SIZE*maxBlocksToProcess];
            //Ch2
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch2[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch2[RX_BLOCK_SIZE*maxBlocksToProcess];
            //Ch3
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch3[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch3[RX_BLOCK_SIZE*maxBlocksToProcess];
            #endif

            int filteredElements_ch0  = repackRxData(rxPackedDataFiltered_ch0, rxPackedLastFiltered_ch0, rxPackedData_ch0, rxPackedLast_ch0, rxPackedValid_ch0, rawElementsRead, &remainingPacked_ch0, &remainingLast_ch0, &remainingBits_ch0, &phaseCounter_ch0);
            #ifdef MULTI_CH
            int filteredElements_ch1  = repackRxData(rxPackedDataFiltered_ch1, rxPackedLastFiltered_ch1, rxPackedData_ch1, rxPackedLast_ch1, rxPackedValid_ch1, rawElementsRead, &remainingPacked_ch1, &remainingLast_ch1, &remainingBits_ch1, &phaseCounter_ch1);
            int filteredElements_ch2  = repackRxData(rxPackedDataFiltered_ch2, rxPackedLastFiltered_ch2, rxPackedData_ch2, rxPackedLast_ch2, rxPackedValid_ch2, rawElementsRead, &remainingPacked_ch2, &remainingLast_ch2, &remainingBits_ch2, &phaseCounter_ch2);
            int filteredElements_ch3  = repackRxData(rxPackedDataFiltered_ch3, rxPackedLastFiltered_ch3, rxPackedData_ch3, rxPackedLast_ch3, rxPackedValid_ch3, rawElementsRead, &remainingPacked_ch3, &remainingLast_ch3, &remainingBits_ch3, &phaseCounter_ch3);
            #endif

            // if(filteredElements_ch0>0){
            //     printf("Number Filtered Elements: %d\n", filteredElements_ch0);
            // }

            // printPacket(rxPackedDataFiltered_ch0, rxPackedLastFiltered_ch0, filteredElements_ch0, &rxByteInPacket, &rxModMode, &rxType, &rxSrc, &rxDst, &rxNetID, &rxLength, true, &rxCount_ch0);

            //Decode packets
            parsePacket(rxPackedDataFiltered_ch0, rxPackedLastFiltered_ch0, filteredElements_ch0, &rx_decoder_state_ch0, &rx_packet_buffer_state_ch0);
            #ifdef MULTI_CH
            parsePacket(rxPackedDataFiltered_ch1, rxPackedLastFiltered_ch1, filteredElements_ch1, &rx_decoder_state_ch1, &rx_packet_buffer_state_ch1);
            parsePacket(rxPackedDataFiltered_ch2, rxPackedLastFiltered_ch2, filteredElements_ch2, &rx_decoder_state_ch2, &rx_packet_buffer_state_ch2);
            parsePacket(rxPackedDataFiltered_ch3, rxPackedLastFiltered_ch3, filteredElements_ch3, &rx_decoder_state_ch3, &rx_packet_buffer_state_ch3);
            #endif

            //Check recieved packets and print
            #ifdef MULTI_CH
            processPackets(rx_packet_buffer_states, 4, &currentID, TX_ID_MAX, &currentBuffer, &failureCount, RX_MAX_FAILURES, PRINT_RX_TITLE, PRINT_RX_DETAILS, PRINT_RX_CONTENT);
            #else
            processPackets(rx_packet_buffer_states, 1, &currentID, TX_ID_MAX, &currentBuffer, &failureCount, RX_MAX_FAILURES, PRINT_RX_TITLE, PRINT_RX_DETAILS, PRINT_RX_CONTENT);
            #endif
        }
    }

    if(txSharedName != NULL){
        //Cleanup
		cleanupProducer(&txFifo);
        cleanupConsumer(&txfbFifo);
        free(txPacket_ch0);
        free(txModMode_ch0);
        #ifdef MULTI_CH
        free(txPacket_ch1);
        free(txModMode_ch1);
        free(txPacket_ch2);
        free(txModMode_ch2);
        free(txPacket_ch3);
        free(txModMode_ch3);
        #endif
    }

    if(rxSharedName != NULL){
        cleanupConsumer(&rxFifo);
    }

    return NULL;
}