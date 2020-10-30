#define _GNU_SOURCE //Need extra functions from sched.h to set thread affinity
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

#include "cyclopsFraming.h"
#include "mainThread.h"

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
    printf("-cpu: CPU to run this application on (does not apply to MacOS)\n");
}

int main(int argc, char **argv) {
    //--- Parse the arguments ---
    char *txPipeName = NULL;
    char *txFeedbackPipeName = NULL;
    char *rxPipeName = NULL;

    double txPeriod = 1.0;
    int32_t txTokens = 500;
    int32_t maxBlocksToProcess = 10;
    int cpu = -1;
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
        }else if(strcmp("-cpu", argv[i]) == 0) {
            i++; //Get the actual argument

            if (i < argc) {
                cpu = strtol(argv[i], NULL, 10);
                if (cpu <= 0) {
                    printf("-cpu must be non-negative\n");
                }
            } else {
                printf("Missing argument for -cpu\n");
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

    //Create Thread Args
    threadArgs_t threadArgs;
    threadArgs.txPipeName=txPipeName;
    threadArgs.txFeedbackPipeName=txFeedbackPipeName;
    threadArgs.rxPipeName=rxPipeName;

    threadArgs.txPeriod=txPeriod;
    threadArgs.txTokens=txTokens;
    threadArgs.maxBlocksToProcess=maxBlocksToProcess;
    threadArgs.gain=gain;

    //Create Thread
    pthread_t thread_app;
    pthread_attr_t attr_app;

    int status = pthread_attr_init(&attr_app);
    if(status != 0)
    {
        printf("Could not create pthread attributes ... exiting");
        exit(1);
    }

    #ifdef __APPLE__
        printf("Warning: On MacOS, CPU parameter is ignored\n");
    #else
        //Can't set thread affinity on MacOS
        cpu_set_t cpuset_app;
        
        //Set Thread CPU
        if(cpu>=0) {
            CPU_ZERO(&cpuset_app); //Clear cpuset
            CPU_SET(cpu, &cpuset_app); //Add CPU to cpuset
            status = pthread_attr_setaffinity_np(&attr_app, sizeof(cpu_set_t), &cpuset_app);//Set thread CPU affinity
            if (status != 0) {
                printf("Could not set thread core affinity ... exiting");
                exit(1);
            }
        }
    #endif

    //Start Thread
    status = pthread_create(&thread_app, &attr_app, mainThread, &threadArgs);
    if(status != 0)
    {
        printf("Could not create a thread ... exiting");
        errno = status;
        perror(NULL);
        exit(1);
    }

    //Wait for thread to exit
    void *res;
    status = pthread_join(thread_app, &res);
    if(status != 0)
    {
        printf("Could not join a thread ... exiting");
        errno = status;
        perror(NULL);
        exit(1);
    }


    return 0;
}