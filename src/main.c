#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printHelp(){
    printf("cyclopsASCIILink <-rx rx.pipe> <-tx tx.pipe -txfb tx_feedback.pipe>\n");
    printf("\n");
    printf("Optional Arguments:\n");
    printf("-rx: Path to the Rx Pipe\n");
    printf("-tx: Path to the Tx Pipe\n");
    printf("-txfb: Path to the Tx Feedback Pipe (required if -tx is present)\n");
}

int main(int argc, char **argv) {
    //--- Parse the arguments ---
    char *txPipeName = NULL;
    char *txFeedbackPipeName = NULL;
    char *rxPipeName = NULL;

    if(argc < 2){
        printHelp();
    }

    for(int i = 1; i<argc; i++){
        if(strcmp("-rx", argv[i]) == 0){
            i++; //Get the actual argument
            if(i<argc){
                rxPipeName = argv[i];
            }else{
                printf("Missing argument for -rx\n");
                exit(1);
            }
        }else if(strcmp("-tx", argv[i]) == 0){
            i++; //Get the actual argument
            if(i<argc){
                txPipeName = argv[i];
            }else{
                printf("Missing argument for -tx\n");
                exit(1);
            }
        }else if(strcmp("-txfb", argv[i]) == 0){
            i++; //Get the actual argument
            if(i<argc){
                txFeedbackPipeName = argv[i];
            }else{
                printf("Missing argument for -txfb\n");
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

    return 0;
}