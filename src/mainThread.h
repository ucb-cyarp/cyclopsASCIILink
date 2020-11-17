//
// Created by Christopher Yarp on 10/21/19.
//

#ifndef CYCLOPSASCIILINK_MAINTHREAD_H
#define CYCLOPSASCIILINK_MAINTHREAD_H

#include <stdint.h>
#include "cyclopsFraming.h"

#define BITS_PER_SYMBOL_PAYLOAD_TX 4 //This changes the transmitted modulation between BPSK=1, QPSK=2, and 16QAM=4

typedef struct{
char *txFifoName;
char *txFeedbackFifoName;
char *rxFifoName;

double txPeriod;
int32_t txTokens;
int32_t maxBlocksInFlight;
int32_t maxBlocksToProcess;
#ifdef CYCLOPS_ASCII_SHARED_MEM
int32_t fifoSize; //Size in blocks
#endif
double txDutyCycle;
int64_t rxSubsamplePeriod;
TX_GAIN_DATATYPE gain;
} threadArgs_t;

void* mainThread_slowPPS(void* args);
void* mainThread_fastPPS(void* args);

#endif //CYCLOPSASCIILINK_MAINTHREAD_H
