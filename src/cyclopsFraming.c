//
// Created by Christopher Yarp on 10/15/19.
//

#include "cyclopsFraming.h"
#include <assert.h>
#include <stdbool.h>

int createRawPreamble(TX_SYMBOL_DATATYPE *rawPreambleBuf, TX_MODTYPE_DATATYPE *preambleModulationBuf){
    static_assert(sizeof(TX_SYMBOL_DATATYPE)*8 >= MAX_BITS_PER_SYMBOL, "MAX_BITS_PER_SYMBOL must fit within TX_SYMBOL_DATATYPE");

    for(int i = 0; i<PREAMBLE_SYMBOL_LEN; i++){
        //Each symbol is placed in a TX_SYMBOL_DATATYPE unit TX_REPITIONS_PER_SYMBOL times
        for(int j = 0; j<TX_REPITIONS_PER_SYMBOL; j++){
            int ind = i*TX_REPITIONS_PER_SYMBOL+j;
            rawPreambleBuf[ind] = cyclopsPreambleSymbols[i];
            preambleModulationBuf[ind] = MOD_TYPE_BPSK; //The Preamble is BPSK
        }
    }

    return PREAMBLE_SYMBOL_LEN*TX_REPITIONS_PER_SYMBOL;
}

int createRawHeader(TX_SYMBOL_DATATYPE *rawHeaderBuf, TX_MODTYPE_DATATYPE *headerModulationBuf, uint8_t modType, uint8_t type, uint8_t src, uint8_t dst, uint16_t netID, uint16_t length){
    //Header is BPSK
    int symbolsPerByteArg = 8;
    uint8_t byteArgMask = 1;

    int headerInd = 0;

    //Mod Type
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, modType, sizeof(modType)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Type
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, type, sizeof(type)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Src
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, src, sizeof(src)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Dst
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, dst, sizeof(dst)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //NET_ID
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, netID, sizeof(netID)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    //Length
    headerInd += unpackToSymbols(rawHeaderBuf+headerInd, headerModulationBuf+headerInd, length, sizeof(length)*8, BITS_PER_SYMBOL_HEADER, TX_REPITIONS_PER_SYMBOL);

    return headerInd;
}

int unpackToSymbols(TX_SYMBOL_DATATYPE *symbolBuf, TX_MODTYPE_DATATYPE *modulationBuf, uint64_t val, uint8_t bitsPerVal, uint8_t bitsPerSymbol, uint8_t symbolRepetitions){
    int symbolsPerVal = bitsPerVal/bitsPerSymbol;
    uint64_t valMask = (1 << bitsPerSymbol)-1;

    int modType;
    switch(bitsPerSymbol){
        case 1:
            modType = MOD_TYPE_BPSK;
            break;
        case 2:
            modType = MOD_TYPE_QPSK;
            break;
        case 3:
            modType = MOD_TYPE_QAM16;
            break;
        default:
            modType = MOD_TYPE_BPSK;
            break;
    }

    for(int i = 0; i<symbolsPerVal; i++) {
        int64_t symbol = val & valMask;
        val = val >> bitsPerSymbol;
        for (int j = 0; j < symbolRepetitions; j++) {
            symbolBuf[i*symbolRepetitions+j] = (TX_SYMBOL_DATATYPE) symbol;
            modulationBuf[i*symbolRepetitions+j] = (TX_MODTYPE_DATATYPE) modType;
        }
    }

    return symbolsPerVal*symbolRepetitions;
}

int createRawASCIIPayload(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, char* message, uint16_t payloadLenBytes, uint16_t bitsPerSymbol, int* msgBytesRead){
    int msgIndex = 0;
    int msgBytesReadLocal = -1;
    int arrayIndex = 0;
    bool foundEndOfStr = false;

    while(msgIndex<payloadLenBytes){
        if(!foundEndOfStr){
            if(message[msgIndex] == '\0'){
                msgBytesReadLocal = msgIndex;
                foundEndOfStr = true;
            }
        }

        if(foundEndOfStr){
            //Use padding byte
            uint8_t paddingByte;
            switch(bitsPerSymbol){
                case 1:
                    paddingByte = PADDING_BYTE_BPSK;
                    break;
                case 2:
                    paddingByte = PADDING_BYTE_QPSK;
                    break;
                case 3:
                    paddingByte = PADDING_BYTE_QAM16;
                    break;
                default:
                    paddingByte = PADDING_BYTE_BPSK;
                    break;
            }
            arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, paddingByte, 8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
        }else{
            arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, message[msgIndex], 8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
        }

        msgIndex++;
    }

    //If we did not encounter the nullPtr
    if(msgBytesReadLocal<0){
        msgBytesReadLocal = msgIndex;
    }

    *msgBytesRead = msgBytesReadLocal;
    return arrayIndex;
}

//TODO: Implement CRC.  For now, just adds padding
int createCRC(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint16_t bitsPerSymbol){
    int arrayIndex = 0;

    for(int i = 0; i<CRC_BYTES_LEN; i++){
        //Use padding byte
        uint8_t paddingByte;
        switch(bitsPerSymbol){
            case 1:
                paddingByte = PADDING_BYTE_BPSK;
                break;
            case 2:
                paddingByte = PADDING_BYTE_QPSK;
                break;
            case 3:
                paddingByte = PADDING_BYTE_QAM16;
                break;
            default:
                paddingByte = PADDING_BYTE_BPSK;
                break;
        }
        arrayIndex += unpackToSymbols(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, paddingByte, 8, bitsPerSymbol, TX_REPITIONS_PER_SYMBOL);
    }

    return arrayIndex;
}

int createRawCyclopsFrame(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint8_t src, uint8_t dst, uint16_t netID, int bitsPerPayloadSymbol, char* message, int* msgBytesRead){
    int arrayIndex = 0;

    int modType;
    int msgLen;
    switch(bitsPerPayloadSymbol){
        case 1:
            modType = MOD_TYPE_BPSK;
            msgLen = BPSK_PAYLOAD_LEN_BYTES;
            break;
        case 2:
            modType = MOD_TYPE_QPSK;
            msgLen = QPSK_PAYLOAD_LEN_BYTES;
            break;
        case 3:
            modType = MOD_TYPE_QAM16;
            msgLen = QAM16_PAYLOAD_LEN_BYTES;
            break;
        default:
            modType = MOD_TYPE_BPSK;
            msgLen = BPSK_PAYLOAD_LEN_BYTES;
            break;
    }

    //Preamble
    arrayIndex += createRawPreamble(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex);

    //Header
    arrayIndex += createRawHeader(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, modType, 0, src, dst, netID, msgLen);

    //Payload
    arrayIndex += createRawASCIIPayload(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, message, msgLen, bitsPerPayloadSymbol, msgBytesRead);

    //CRC
    arrayIndex += createCRC(packetBuffer+arrayIndex, modeModeBuffer+arrayIndex, bitsPerPayloadSymbol);

    return arrayIndex;
}