#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "helpers.h"
#include "feedbackDefines.h"
#include "cyclopsFraming.h"
#include "demoText.h"

void printHelp(){
    printf("cyclopsASCIILink <-rx rx.pipe> <-tx tx.pipe -txfb tx_feedback.pipe> <-txperiod 1.0> <-txtokens 500> <-processlimit 10>\n");
    printf("\n");
    printf("Optional Arguments:\n");
    printf("-rx: Path to the Rx Pipe\n");
    printf("-tx: Path to the Tx Pipe\n");
    printf("-txfb: Path to the Tx Feedback Pipe (required if -tx is present)\n");
    printf("-txperiod: The period (in seconds) between packet transmission\n");
    printf("-txtokens: The number of initial Tx tokens (in blocks).  Tokens are replenished via the feedback pipe\n");
    printf("-processlimit: The maximum number of blocks to process at one time\n");
}

int main(int argc, char **argv) {
    //--- Parse the arguments ---
    char *txPipeName = NULL;
    char *txFeedbackPipeName = NULL;
    char *rxPipeName = NULL;

    double txPeriod = 1.0;
    int32_t txTokens = 500;
    int32_t maxBlocksToProcess = 10;

    TX_GAIN_DATATYPE gain = 1;  //TODO: Add to parameters list

    if(argc < 2){
        printHelp();
    }

    for(int i = 1; i<argc; i++){
        if(strcmp("-rx", argv[i]) == 0){
            i++; //Get the actual argument

            if(!RX_AVAILABLE){
                printf("Rx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                rxPipeName = argv[i];
            }else{
                printf("Missing argument for -rx\n");
                exit(1);
            }
        }else if(strcmp("-tx", argv[i]) == 0){
            i++; //Get the actual argument

            if(!TX_AVAILABLE){
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                txPipeName = argv[i];
            }else{
                printf("Missing argument for -tx\n");
                exit(1);
            }
        }else if(strcmp("-txfb", argv[i]) == 0){
            i++; //Get the actual argument

            if(!TX_AVAILABLE){
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                txFeedbackPipeName = argv[i];
            }else{
                printf("Missing argument for -txfb\n");
                exit(1);
            }
        }else if(strcmp("-txperiod", argv[i]) == 0){
            i++; //Get the actual argument

            if(!TX_AVAILABLE){
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                txPeriod = strtod(argv[i], NULL);
                if(txPeriod<=0){
                    printf("-txperiod must be positive\n");
                }
            }else{
                printf("Missing argument for -txperiod\n");
                exit(1);
            }
        }else if(strcmp("-txtokens", argv[i]) == 0){
            i++; //Get the actual argument

            if(!TX_AVAILABLE){
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                txTokens = strtol(argv[i], NULL, 10);
                if(txTokens<=0){
                    printf("-txtokens must be positive\n");
                }
            }else{
                printf("Missing argument for -txtokens\n");
                exit(1);
            }
        }else if(strcmp("-processlimit", argv[i]) == 0) {
            i++; //Get the actual argument

            if (!TX_AVAILABLE) {
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if (i < argc) {
                maxBlocksToProcess = strtol(argv[i], NULL, 10);
                if (txTokens <= 0) {
                    printf("-processlimit must be positive\n");
                }
            } else {
                printf("Missing argument for -processlimit\n");
                exit(1);
            }
        }else{
            printf("Unknown CLI option: %s\n", argv[i]);
        }
    }

    if((txPipeName == NULL && txFeedbackPipeName != NULL) || (txPipeName != NULL && txFeedbackPipeName == NULL)){
        printf("-tx and -txfb must come as a pair\n");
        exit(1);
    }

    if(txPipeName == NULL && rxPipeName == NULL){
        exit(0);
    }

    //Open Pipes (if applicable)
    FILE *rxPipe = NULL;
    FILE *txPipe = NULL;
    FILE *txFeedbackPipe = NULL;

    if(rxPipeName != NULL){
        rxPipe= fopen(rxPipeName, "rb");
        if(rxPipe == NULL) {
            printf("Unable to open Rx Pipe ... exiting\n");
            perror(NULL);
            exit(1);
        }
    }

    if(txPipeName != NULL){
        txPipe = fopen(txPipeName, "wb");
        if(txPipe == NULL){
            printf("Unable to open Tx Pipe ... exiting\n");
            perror(NULL);
            exit(1);
        }

        txFeedbackPipe = fopen(txFeedbackPipeName, "rb");
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
        txPacket = (TX_SYMBOL_DATATYPE*) malloc(sizeof(TX_SYMBOL_DATATYPE)*MAX_PAYLOAD_PLUS_CRC_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);
        txModMode = (TX_MODTYPE_DATATYPE*) malloc(sizeof(TX_MODTYPE_DATATYPE)*MAX_PAYLOAD_PLUS_CRC_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL);

        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 4, testText, &msgBytesRead); //Encode a 16QAM Packet
//        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 2, testText, &msgBytesRead); //Encode a QPSK Packet
//        txPacketLen =  createRawCyclopsFrame(txPacket, txModMode, txSrc, txDst, txNetID, 1, testText, &msgBytesRead); //Encode a BPSK Packet
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

    //Main Loop
    bool running = true;
    while(running){
        if(txPipe!=NULL){
            //If transmissions are OK
            if(txTokens > 0) {
                //Check if OK to send

                if(txIndex<txPacketLen){
                    //Check if we are currently sending a packet
                    //Send a packet
                    txIndex += sendData(txPipe, txPacket+txIndex, txModMode+txIndex, txPacketLen-txIndex, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                }else{
                    //Should we be sending
                    time_t currentTime = time(NULL);
                    double duration = difftime(currentTime, lastTxStartTime);
                    if(duration >= txPeriod){
                        lastTxStartTime = currentTime;
                        txIndex = 0;
                        txIndex += sendData(txPipe, txPacket, txModMode, txPacketLen, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
                    }
                }
            }else{
                //Write 0's
                //TODO: Change to a more optimized solution
                sendData(txPipe, txPacket, txModMode, 0, gain, maxBlocksToProcess < txTokens ? maxBlocksToProcess : txTokens , &txTokens);
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

            RX_PACKED_DATATYPE rxPackedDataFiltered[RX_BLOCK_SIZE*maxBlocksToProcess]; //Worst case allocation
            RX_PACKED_LAST_DATATYPE rxPackedLastFiltered[RX_BLOCK_SIZE*maxBlocksToProcess];
            int filteredElements  = filterRxData(rxPackedDataFiltered, rxPackedLastFiltered, rxPackedData, rxPackedLast, rxPackedStrobe, rxPackedValid, rawElementsRead);

            printPacket(rxPackedDataFiltered, rxPackedLastFiltered, filteredElements, &rxByteInPacket, &rxModMode, &rxType, &rxSrc, &rxDst, &rxNetID, &rxLength, true);
        }
    }

    if(txPipe != NULL){
        //Cleanup
        free(txPacket);
        free(txModMode);
    }

    return 0;
}