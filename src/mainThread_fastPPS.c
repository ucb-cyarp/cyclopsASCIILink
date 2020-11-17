//
// Created by Christopher Yarp on 10/21/19.
//

#include "mainThread.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "helpers.h"
#include "feedbackDefines.h"
#include "demoText.h"

#ifdef CYCLOPS_ASCII_SHARED_MEM
#include "depends/BerkeleySharedMemoryFIFO.h"
#endif

#define TX_ID_MAX (64)
#define RX_PACKETS_TO_STORE_PER_CH (5)
#define RX_MAX_FAILURES (10)

#define PRINT_RX_TITLE (true)
#define PRINT_RX_DETAILS (true)
#define PRINT_RX_CONTENT (true)

void* mainThread_fastPPS(void* argsUncast){
    threadArgs_t* args = (threadArgs_t*) argsUncast;
    char *txFifoName = args->txFifoName;
    char *txFeedbackFifoName = args->txFeedbackFifoName;
    char *rxFifoName = args->rxFifoName;
    double txDutyCycle = args->txDutyCycle;
    int64_t rxSubsamplePeriod = args->rxSubsamplePeriod;
    int32_t txTokens = args->txTokens;
    int32_t maxBlocksToProcess = args->maxBlocksToProcess;
    int32_t maxBlocksInFlight = args->maxBlocksInFlight;
	#ifdef CYCLOPS_ASCII_SHARED_MEM
    int32_t fifoSize = args->fifoSize;
	#endif
    TX_GAIN_DATATYPE gain = args->gain;

    printf("Tx Duty Cycle: %f, Rx Subsample: %ld\n", txDutyCycle, rxSubsamplePeriod);

    //TODO: Make this an arg
    int numTxPktsToCreate = 128;
    int randSeed = 8675309;

    //Open Pipes (if applicable)
	#ifdef CYCLOPS_ASCII_SHARED_MEM
		sharedMemoryFIFO_t txFifo;
	    sharedMemoryFIFO_t rxFifo;
	    sharedMemoryFIFO_t txFeedbackFifo;

	    //Initialize producer FIFOs first
	    initSharedMemoryFIFO(&txFifo);
	    initSharedMemoryFIFO(&rxFifo);
	    initSharedMemoryFIFO(&txFeedbackFifo);

	    size_t rxFifoBufferSizeBytes = sizeof(RX_STRUCTURE_TYPE_NAME)*fifoSize;
	    size_t txFifoBufferSizeBytes = sizeof(TX_STRUCTURE_TYPE_NAME)*fifoSize;
	    size_t txFeedbackFifoBufferSizeBytes = sizeof(FEEDBACK_DATATYPE)*fifoSize;

	    if(txFifoName != NULL){
	        producerOpenInitFIFO(txFifoName, txFifoBufferSizeBytes, &txFifo);
	        // printf("Opened Tx FIFO: %s\n", txSharedName);
	        consumerOpenFIFOBlock(txFeedbackFifoName, txFeedbackFifoBufferSizeBytes, &txFeedbackFifo);
	        // printf("Opened TxFB FIFO: %s\n", txFeedbackSharedName);
	    }

	    if(rxFifoName != NULL){
	        consumerOpenFIFOBlock(rxFifoName, rxFifoBufferSizeBytes, &rxFifo);
	        // printf("Opened Rx FIFO: %s\n", rxSharedName);
	    }
	#else
	    FILE *rxFifo = NULL;
	    FILE *txFifo = NULL;
	    FILE *txFeedbackFifo = NULL;

	    if(rxFifoName != NULL){
	        rxFifo= fopen(rxFifoName, "rb");
	        printf("Opening Rx Pipe: %s\n", rxFifoName);
	        if(rxFifo == NULL) {
	            printf("Unable to open Rx Pipe ... exiting\n");
	            perror(NULL);
	            exit(1);
	        }
	    }

	    if(txFifoName != NULL){
	        txFifo = fopen(txFifoName, "wb");
	        printf("Opening Tx Pipe: %s\n", txFifoName);
	        if(txFifo == NULL){
	            printf("Unable to open Tx Pipe ... exiting\n");
	            perror(NULL);
	            exit(1);
	        }

	        txFeedbackFifo = fopen(txFeedbackFifoName, "rb");
	        printf("Opening Tx Feedback Pipe: %s\n", txFeedbackFifoName);
	        if(txFeedbackFifo == NULL) {
	            printf("Unable to open Tx Feedback Pipe ... exiting\n");
	            perror(NULL);
	            exit(1);
	        }
	    }

	#endif

    //Ch0
    TX_SYMBOL_DATATYPE* txPacket_ch0;
    TX_MODTYPE_DATATYPE* txModMode_ch0;
    #ifdef MULTI_CH
    //Ch1
    TX_SYMBOL_DATATYPE* txPacket_ch1;
    TX_MODTYPE_DATATYPE* txModMode_ch1;
    //Ch2
    TX_SYMBOL_DATATYPE* txPacket_ch2;
    TX_MODTYPE_DATATYPE* txModMode_ch2;
    //Ch3
    TX_SYMBOL_DATATYPE* txPacket_ch3;
    TX_MODTYPE_DATATYPE* txModMode_ch3;
    #endif

    uint8_t txSrc = 0;
    uint8_t txDst = 1;
    uint16_t txNetID = 10;

    int txID = 0; //This is the id number used to order packets
    char* blankStr = "";


    //If transmitting, allocate arrays for packets to be constructed before entering the main loop
    //    All packets will be pre-formed on the Tx side and will be selected for transmission later.

    size_t pktSrcArraySymbolsPerAllocPkt;
    size_t pktSrcArraySymbolBytesPerAllocPkt;
    size_t pktSrcArrayModTypeBytesPerAllocPkt;
    TX_SYMBOL_DATATYPE*  txPacket_srcArray;
    TX_SYMBOL_DATATYPE* txModMode_srcArray;
    int txPacketLen = 0;

    if(txFifoName != NULL){
        pktSrcArraySymbolsPerAllocPkt = MAX_PACKET_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL;
        if(TX_DEMO_BLOCK_SIZE>pktSrcArraySymbolsPerAllocPkt) {
            printf("Block size must be <= the packet length in symbols\n");
            exit(1);
        }
        pktSrcArraySymbolBytesPerAllocPkt = sizeof(TX_SYMBOL_DATATYPE)*pktSrcArraySymbolsPerAllocPkt;
        pktSrcArrayModTypeBytesPerAllocPkt = sizeof(TX_MODTYPE_DATATYPE)*pktSrcArraySymbolsPerAllocPkt;
        txPacket_srcArray  = (TX_SYMBOL_DATATYPE*)  malloc(pktSrcArraySymbolBytesPerAllocPkt*numTxPktsToCreate);
        txModMode_srcArray = (TX_MODTYPE_DATATYPE*) malloc(pktSrcArrayModTypeBytesPerAllocPkt*numTxPktsToCreate);

        //Create packets to select from
        int txStrLoc = 0; //This is the location we are currently at when sending the txStr
        const char* txStr = testTextExtraLong;
        int txStrLen = strlen(txStr);

        for(int i = 0; i<numTxPktsToCreate; i++) {
            TX_SYMBOL_DATATYPE* txPkt = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*i;
            TX_MODTYPE_DATATYPE* txModMode = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*i;
            int msgBytesRead = 0;
            int pktLen = createRawCyclopsFrame(txPkt, txModMode, txID, txID, txID, BITS_PER_SYMBOL_PAYLOAD_TX, txStr+txStrLoc, &msgBytesRead); //Encode a BPSK Packet
            if(i == 0) {
                txPacketLen = pktLen;
            }else if(pktLen != txPacketLen) {
                //TODO: allow different length packets
                //      Only really an issue with multiple chanels sending different packet lengths.
                //      Requires seperate duty cycle logic for each
                printf("Packets should have the same length");
                exit(1);
            }

            //MsgBytesRead indicates how many bytes of the textToBeEncoded were read.  Add this to the text ptr passed to the next one
            txStrLoc += msgBytesRead;
            txID = txID < TX_ID_MAX - 1 ? txID + 1 : 0;
            if (txStrLoc >= txStrLen) {
                //Got to the end of the string, wrap around
                txStrLoc = 0;
            }
        }

        //Select Initial packets for each channel
        srand(randSeed);
        //Ch0
        int pktInd_ch0 = rand()%numTxPktsToCreate;
        txPacket_ch0 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch0;
        txModMode_ch0 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch0;
        #ifdef MULTI_CH
            //Ch1
            int pktInd_ch1 = rand()%numTxPktsToCreate;
            txPacket_ch1 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch1;
            txModMode_ch1 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch1;
            //Ch2
            int pktInd_ch2 = rand()%numTxPktsToCreate;
            txPacket_ch2 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch2;
            txModMode_ch2 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch2;
            //Ch3
            int pktInd_ch3 = rand()%numTxPktsToCreate;
            txPacket_ch3 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch3;
            txModMode_ch3 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch3;
        #endif
    }

    //Compute gap symbols
    //Note: gap symbols will be quantized to blocks
    //dutyCycle=pktLen/(pktLen+blankLen)
    //blankLen=pktLen*(1-dutyCycle)/dutyCycle

    double blankLenDbl = txPacketLen*(1.0-txDutyCycle)/txDutyCycle;
    int blankLen = (int) round(blankLenDbl);

    //Because of block quantization, there may be some unavoidable blank samples at the end of a packet
    int pktBlankPadding = txPacketLen%TX_BLOCK_SIZE;

    int extraBlanksRequired = blankLen-pktBlankPadding;
    //Note, it is possible for this to be negative if the requested number of blanks is less than the padding required
    //We will threshold at 0 additional blanks
    bool warnDutyCycle = false;
    if(extraBlanksRequired<0) {
        extraBlanksRequired = 0;
        warnDutyCycle = true;
    }

    //The extra blanks need to be quantized to blocks
    if(extraBlanksRequired%TX_BLOCK_SIZE != 0) {
        warnDutyCycle = true;
    }
    int extraBlankBlocksRequired = (int) round(((double) extraBlanksRequired)/TX_BLOCK_SIZE);
    int extraBlanksRequiredRounded = extraBlankBlocksRequired*TX_BLOCK_SIZE;

    if(warnDutyCycle) {
        double actualDutyCycle = ((double) txPacketLen)/(txPacketLen + pktBlankPadding + extraBlanksRequiredRounded);
        fprintf(stderr, "Warning, requested duty cycle of %f could not be achieved.  Using a duty cycle of %f instead.\n", txDutyCycle, actualDutyCycle);
    }

    printf("Pkt Len (Symbols): %d, Blank Len (Symbols): %d\n", txPacketLen, pktBlankPadding+extraBlanksRequiredRounded);

    //Tx State
    int txIndex = 0; //The current symbol to be transmitted in the packet.  Setting to 0 since we already selected packets to send
    time_t lastTxStartTime = time(NULL); //Will transmit after an initial gap

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

    int64_t pktRxCounter_ch0 = 0;
    #ifdef MULTI_CH
        int64_t pktRxCounter_ch1 = 0;
        int64_t pktRxCounter_ch2 = 0;
        int64_t pktRxCounter_ch3 = 0;
    #endif

    #ifdef MULTI_CH
    int64_t pktRxCounterTotal[4] = {0, 0, 0, 0};
    #else
    int64_t pktRxCounterTotal[1] = {0};

    #endif


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

    int blankCount = 0;

    int outstandingBal = 0;

    //Main Loop
    bool running = true;
    while(running){
        //==== Tx ====
        if(txFifoName!=NULL){
            //If transmissions are OK (we have Tx tokens from flow control)
            bool blocksInFlightTxOk = true;
            if(maxBlocksInFlight>0){
                blocksInFlightTxOk = outstandingBal < maxBlocksInFlight;
            }

            if(txTokens > 0 && blocksInFlightTxOk) {
                //Check if we are already sending a packet, are sending blanks
                //Will select new packets to send after finishing sending blanks
                if(txIndex<txPacketLen){
                    //We are currently sending a packet
                    //Continue sending it
                    // printf("In process of sending packet\n");
                    #ifdef MULTI_CH
					#ifdef CYCLOPS_ASCII_SHARED_MEM
					sendRtn_t sendStatus = sendData(&txFifo,
					#else
                    sendRtn_t sendStatus = sendData(txFifo,
					#endif
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
                    #ifdef CYCLOPS_ASCII_SHARED_MEM
                    sendRtn_t sendStatus = sendData(&txFifo,
					#else
                    sendRtn_t sendStatus = sendData(txFifo,
					#endif
                                            txPacket_ch0+txIndex,
                                            txModMode_ch0+txIndex, 
                                            txPacketLen-txIndex,
                                            maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens, 
                                            &txTokens);
                    #endif
					txIndex += sendStatus.elementsSent;
					blankCount += sendStatus.blanksSent;
                    outstandingBal += sendStatus.elementsSent/TX_BLOCK_SIZE + sendStatus.blanksSent/TX_BLOCK_SIZE;
                }else{
                    //We are done sending the packet

                    //We may be sending blanks, or we may need to select a new packet to send
                    if(blankCount < extraBlanksRequiredRounded) {
                        //sending blanks
                        int blanksToRequest = extraBlanksRequiredRounded-blankCount;

                        #ifdef CYCLOPS_ASCII_SHARED_MEM
                        int blanksSent = sendBlank(&txFifo,
                        #else
                        int blanksSent = sendBlank(txFifo,
                        #endif
                                                blanksToRequest,
                                                maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens,
                                                &txTokens);

                        blankCount += blanksSent;
                        outstandingBal += blanksSent/TX_BLOCK_SIZE;
                    }

                    //Check if after sending blanks (if applicable) new packet(s) need to be chosen
                    if(blankCount>=extraBlanksRequiredRounded) {
                        //Reset Tx state for next transmission
                        txIndex = 0;
                        blankCount = 0;

                        //Select new packets
                        int pktInd_ch0 = rand()%numTxPktsToCreate;
                        txPacket_ch0 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch0;
                        txModMode_ch0 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch0;
                        #ifdef MULTI_CH
                            //Ch1
                            int pktInd_ch1 = rand()%numTxPktsToCreate;
                            txPacket_ch1 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch1;
                            txModMode_ch1 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch1;
                            //Ch2
                            int pktInd_ch2 = rand()%numTxPktsToCreate;
                            txPacket_ch2 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch2;
                            txModMode_ch2 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch2;
                            //Ch3
                            int pktInd_ch3 = rand()%numTxPktsToCreate;
                            txPacket_ch3 = txPacket_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch3;
                            txModMode_ch3 = txModMode_srcArray+pktSrcArraySymbolsPerAllocPkt*pktInd_ch3;
                        #endif
                    }
                }
            }

            //Check for feedback returning tokens
            for(int i = 0; i<maxBlocksToProcess; i++) {
                //Check for feedback (use select)
				#ifdef CYCLOPS_ASCII_SHARED_MEM
					bool feedbackReady = isReadyForReading(&txFeedbackFifo);
				#else
                	bool feedbackReady = isReadyForReading(txFeedbackFifo);
				#endif
                if(feedbackReady){
                    //Get feedback
                    FEEDBACK_DATATYPE tokensReturned;
                    //Once data starts coming, a full transaction should be in process.  Can block on the transaction.
					
					#ifdef CYCLOPS_ASCII_SHARED_MEM
	                    int elementsRead = readFifo(&tokensReturned, sizeof(tokensReturned), 1, &txFeedbackFifo);
	                    if(elementsRead != 1){
	                        //Done!
	                        running = false;
	                        break;
	                    }
					#else
	                    int elementsRead = fread(&tokensReturned, sizeof(tokensReturned), 1, txFeedbackFifo);
	                    if(elementsRead != 1 && feof(txFeedbackFifo)){
	                        //Done!
	                        running = false;
	                        break;
	                    } else if (elementsRead != 1 && ferror(txFeedbackFifo)){
	                        printf("An error was encountered while reading the feedback pipe\n");
	                        perror(NULL);
	                        exit(1);
	                    } else if (elementsRead != 1){
	                        printf("An unknown error was encountered while reading the feedback pipe\n");
	                        exit(1);
	                    }
					#endif
					
                    txTokens += tokensReturned;
                }else{
                    break;
                }
            }
        }

        //==== Rx ====
        if(rxFifoName!=NULL){
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
			#ifdef CYCLOPS_ASCII_SHARED_MEM
			int rawElementsRead = recvData(&rxFifo,
			#else
            int rawElementsRead = recvData(rxFifo,
			#endif
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
            #ifdef CYCLOPS_ASCII_SHARED_MEM
			int rawElementsRead = recvData(&rxFifo,
			#else
            int rawElementsRead = recvData(rxFifo,
			#endif
                                           //Ch0 
                                           rxPackedData_ch0, 
                                           rxPackedValid_ch0, 
                                           rxPackedLast_ch0, 
                                           maxBlocksToProcess, &isDoneReading);
            #endif

            outstandingBal -= rawElementsRead/RX_BLOCK_SIZE;

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

            if(txPacketLen<RX_BLOCK_SIZE*maxBlocksToProcess) {
                fprintf(stderr, "Multiple packets can be received in a single processing iteration which could lead to more discontinuities\n");
                exit(1);
                //TODO: To fix this, allocate more extra space below for packet discontinuity
            }
            //Ch0
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch0[RX_BLOCK_SIZE*(maxBlocksToProcess+1)]; //Worst case allocation (+1 for packet discontinuity)
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch0[RX_BLOCK_SIZE*(maxBlocksToProcess+1)];
            #ifdef MULTI_CH
            //Ch1
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch1[RX_BLOCK_SIZE*(maxBlocksToProcess+1)]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch1[RX_BLOCK_SIZE*(maxBlocksToProcess+1)];
            //Ch2
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch2[RX_BLOCK_SIZE*(maxBlocksToProcess+1)]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch2[RX_BLOCK_SIZE*(maxBlocksToProcess+1)];
            //Ch3
            RX_PACKED_DATATYPE rxPackedDataFiltered_ch3[RX_BLOCK_SIZE*(maxBlocksToProcess+1)]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered_ch3[RX_BLOCK_SIZE*(maxBlocksToProcess+1)];
            #endif

            int filteredElements_ch0  = repackRxDataEveryNth(rxPackedDataFiltered_ch0, rxPackedLastFiltered_ch0, rxPackedData_ch0, rxPackedLast_ch0, rxPackedValid_ch0, rawElementsRead, &remainingPacked_ch0, &remainingLast_ch0, &remainingBits_ch0, &phaseCounter_ch0, &pktRxCounter_ch0, pktRxCounterTotal+0, rxSubsamplePeriod);
            #ifdef MULTI_CH
            int filteredElements_ch1  = repackRxDataEveryNth(rxPackedDataFiltered_ch1, rxPackedLastFiltered_ch1, rxPackedData_ch1, rxPackedLast_ch1, rxPackedValid_ch1, rawElementsRead, &remainingPacked_ch1, &remainingLast_ch1, &remainingBits_ch1, &phaseCounter_ch1, &pktRxCounter_ch1, pktRxCounterTotal+0, rxSubsamplePeriod);
            int filteredElements_ch2  = repackRxDataEveryNth(rxPackedDataFiltered_ch2, rxPackedLastFiltered_ch2, rxPackedData_ch2, rxPackedLast_ch2, rxPackedValid_ch2, rawElementsRead, &remainingPacked_ch2, &remainingLast_ch2, &remainingBits_ch2, &phaseCounter_ch2, &pktRxCounter_ch2, pktRxCounterTotal+1, rxSubsamplePeriod);
            int filteredElements_ch3  = repackRxDataEveryNth(rxPackedDataFiltered_ch3, rxPackedLastFiltered_ch3, rxPackedData_ch3, rxPackedLast_ch3, rxPackedValid_ch3, rawElementsRead, &remainingPacked_ch3, &remainingLast_ch3, &remainingBits_ch3, &phaseCounter_ch3, &pktRxCounter_ch3, pktRxCounterTotal+2, rxSubsamplePeriod);
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

            //Print packets
            #ifdef MULTI_CH
            processPacketsNoIDCheck(rx_packet_buffer_states, 4, TX_ID_MAX, &currentBuffer, PRINT_RX_TITLE, PRINT_RX_DETAILS, PRINT_RX_CONTENT, pktRxCounterTotal);
            #else
            processPacketsNoIDCheck(rx_packet_buffer_states, 1, TX_ID_MAX, &currentBuffer, PRINT_RX_TITLE, PRINT_RX_DETAILS, PRINT_RX_CONTENT, pktRxCounterTotal);
            #endif
        }
    }

    if(txFifoName != NULL){
        //Cleanup
		#ifdef CYCLOPS_ASCII_SHARED_MEM
			cleanupProducer(&txFifo);
	        cleanupConsumer(&txFeedbackFifo);
        #endif
        free(txPacket_srcArray);
        free(txModMode_srcArray);
    }

	#ifdef CYCLOPS_ASCII_SHARED_MEM
	    if(rxFifoName != NULL){
	        cleanupConsumer(&rxFifo);
	    }
	#endif

    return NULL;
}