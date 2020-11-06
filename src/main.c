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
    printf("cyclopsASCIILink <-rx rx.pipe> <-tx tx.pipe <-txfb tx_feedback.pipe> <-txperiod 1.0> <-txdutycycle 0.5> <-rxsubsampleperiod 200000> <-txtokens 500> <-processlimit 10>\n");
    printf("\n");
    printf("The application will default to using the txperiod unless a duty cycle or subsampling period are specified\n");
    printf("\n");
    printf("Optional Arguments:\n");
    printf("-rx: Path to the Rx Pipe\n");
    printf("-tx: Path to the Tx Pipe\n");
    printf("-txfb: Path to the Tx Feedback Pipe (required if -tx is present)\n");
    printf("-txperiod: The period (in seconds) between packet transmission\n");
    printf("-txtokens: The number of initial Tx tokens (in blocks).  Tokens are replenished via the feedback pipe\n");
    printf("-processlimit: The maximum number of blocks to process at one time\n");
	#ifdef CYCLOPS_ASCII_SHARED_MEM
	printf("-fifosize: The size of the FIFO in blocks\n");
	#endif
    printf("-cpu: CPU to run this application on (negative number to not pin to a particular CPU, does not apply to MacOS)\n");
}

int main(int argc, char **argv) {
    //--- Parse the arguments ---
    char *txFifoName = NULL;
    char *txFeedbackFifoName = NULL;
    char *rxFifoName = NULL;

    double txPeriod = 1.0;
    double txDutyCycle = 0.5;
    int rxSubsamplePeriod = 200000;
    int32_t txTokens = 500;
    int32_t maxBlocksToProcess = 10;
	#ifdef CYCLOPS_ASCII_SHARED_MEM
    	int32_t fifoSize = 8;
	#endif
    int cpu = -1;
    TX_GAIN_DATATYPE gain = 1;  //TODO: Add to parameters list
    bool useDutyCycle = false;
    bool useTxPeriod = false;

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
                rxFifoName = argv[i];
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
                txFifoName = argv[i];
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
                txFeedbackFifoName = argv[i];
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
                    exit(1);
                }
            }else{
                printf("Missing argument for -txperiod\n");
                exit(1);
            }

            useTxPeriod = true;
        }else if(strcmp("-txdutycycle", argv[i]) == 0){
            i++; //Get the actual argument

            if(!TX_AVAILABLE){
                printf("Tx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                txDutyCycle = strtod(argv[i], NULL);
                if(txPeriod<=0){
                    printf("-txdutycycle must be positive\n");
                    exit(1);
                }
            }else{
                printf("Missing argument for -txdutycycle\n");
                exit(1);
            }

            useDutyCycle = true;
        }else if(strcmp("-rxsubsampleperiod", argv[i]) == 0){
            i++; //Get the actual argument

            if(!RX_AVAILABLE){
                printf("Rx is unavailable in current configuration\n");
                exit(1);
            }

            if(i<argc){
                rxSubsamplePeriod = atoi(argv[i]);
                if(rxSubsamplePeriod<0){
                    printf("-rxsubsampleperiod must be >= 0\n");
                    exit(1);
                }
            }else{
                printf("Missing argument for -rxsubsampleperiod\n");
                exit(1);
            }

            useDutyCycle = true;
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
                    exit(1);
                }
            }else{
                printf("Missing argument for -txtokens\n");
                exit(1);
            }
		#ifdef CYCLOPS_ASCII_SHARED_MEM
        }else if(strcmp("-fifosize", argv[i]) == 0){
            i++; //Get the actual argument

            if(i<argc){
                fifoSize = strtol(argv[i], NULL, 10);
                if(fifoSize <= 0){
                    printf("-fifosize must be positive\n");
                }
            }else{
                printf("Missing argument for -fifosize\n");
                exit(1);
            }
		#endif
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
            } else {
                printf("Missing argument for -cpu\n");
                exit(1);
            }
        }else{
            printf("Unknown CLI option: %s\n", argv[i]);
        }
    }

    if((txFifoName == NULL && txFeedbackFifoName != NULL) || (txFifoName != NULL && txFeedbackFifoName == NULL)){
        printf("-tx and -txfb must come as a pair\n");
        exit(1);
    }

    if(txFifoName == NULL && rxFifoName == NULL){
        exit(0);
    }

    if(useTxPeriod && useDutyCycle){
        printf("-txdutycycle and/or -rxsubsampleperiod cannot be used in combination with -txperiod\n");
        exit(1);
    }

    //Create Thread Args
    threadArgs_t threadArgs;
    threadArgs.txFifoName=txFifoName;
    threadArgs.txFeedbackFifoName=txFeedbackFifoName;
    threadArgs.rxFifoName=rxFifoName;

    threadArgs.txPeriod=txPeriod;
    threadArgs.txTokens=txTokens;
    threadArgs.maxBlocksToProcess=maxBlocksToProcess;
	#ifdef CYCLOPS_ASCII_SHARED_MEM
    	threadArgs.fifoSize=fifoSize;
	#endif
    threadArgs.gain=gain;
    threadArgs.txDutyCycle=txDutyCycle;
    threadArgs.rxSubsamplePeriod=rxSubsamplePeriod;

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

        if(cpu >= 0){
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
        }
    #endif

    //Start Thread
    if(useDutyCycle){
        status = pthread_create(&thread_app, &attr_app, mainThread_fastPPS, &threadArgs);
    }else{
        //Defaults to using Tx period
        status = pthread_create(&thread_app, &attr_app, mainThread_slowPPS, &threadArgs);
    }
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