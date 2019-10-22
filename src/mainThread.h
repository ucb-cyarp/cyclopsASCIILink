//
// Created by Christopher Yarp on 10/21/19.
//

#ifndef CYCLOPSASCIILINK_MAINTHREAD_H
#define CYCLOPSASCIILINK_MAINTHREAD_H

#include <stdint.h>
#include "cyclopsFraming.h"

typedef struct{
char *txPipeName;
char *txFeedbackPipeName;
char *rxPipeName;

double txPeriod;
int32_t txTokens;
int32_t maxBlocksToProcess;
TX_GAIN_DATATYPE gain;
} threadArgs_t;

void* mainThread(void* args);

#endif //CYCLOPSASCIILINK_MAINTHREAD_H
