//
// Created by Christopher Yarp on 10/15/19.
//

#ifndef CYCLOPSASCIILINK_CYCLOPSFRAMING_H
#define CYCLOPSASCIILINK_CYCLOPSFRAMING_H

#include <stdint.h>
#include <stdbool.h>
#include "vitisStructure.h"

#define PREAMBLE_SYMBOL_LEN (2752)
#define HEADER_SYMBOL_LEN (64)
#define MAX_PAYLOAD_SYMBOL_LEN (12256)
#define CRC_BYTES_LEN (4)
#define MAX_PAYLOAD_PLUS_CRC_SYMBOL_LEN (MAX_PAYLOAD_SYMBOL_LEN+CRC_BYTES_LEN*8) //BPSK
#define MAX_PACKET_SYMBOL_LEN (PREAMBLE_SYMBOL_LEN+HEADER_SYMBOL_LEN+MAX_PAYLOAD_PLUS_CRC_SYMBOL_LEN)

#define MAX_BITS_PER_SYMBOL (4)
//#define BITS_PER_SYMBOL_PREAMBLE (1)
#define BITS_PER_SYMBOL_HEADER (1)

#define TX_REPITIONS_PER_SYMBOL (4)
#define RX_REPITITIONS_PER_OUTPUT (1)

#define MOD_TYPE_BPSK (0)
#define MOD_TYPE_QPSK (1)
#define MOD_TYPE_QAM16 (2)

#define PADDING_BYTE_BPSK ('U')
#define PADDING_BYTE_QPSK ('3')
#define PADDING_BYTE_QAM16 ('(')

#define BPSK_PAYLOAD_LEN_BYTES (1528)
#define QPSK_PAYLOAD_LEN_BYTES (3060)
#define QAM16_PAYLOAD_LEN_BYTES (6124)

#define MAX_PAYLOAD_LEN QAM16_PAYLOAD_LEN_BYTES
#define MAX_PAYLOAD_PLUS_CRC_LEN (QAM16_PAYLOAD_LEN_BYTES+CRC_BYTES_LEN)

//Colors (based on https://www.linuxjournal.com/article/8603, and https://en.wikipedia.org/wiki/Escape_sequences_in_C), and https://en.wikipedia.org/wiki/ANSI_escape_code
#define RESET (0)
#define BRIGHT (1)
#define DIM (2)
#define UNDERLINE (3)
#define BLINK (4)
#define REVERSE (5)
#define HIDDEN (6)

#define BLACK (0)
#define RED	(1)
#define GREEN (2)
#define YELLOW (3)
#define BLUE (4)
#define MAGENTA	(5)
#define CYAN (6)
#define	WHITE (7)
#define BRIGHT_BLACK (60)
#define BRIGHT_RED (61)
#define BRIGHT_GREEN (62)
#define BRIGHT_YELLOW (63)
#define BRIGHT_BLUE (64)
#define BRIGHT_MAGENTA (65)
#define BRIGHT_CYAN (66)
#define BRIGHT_WHITE (67)

#define BG_COLOR BRIGHT_WHITE
#define FG_COLOR_DEFAULT BLACK
#define FG_COLOR_START RED

void setColor(int fg_color, int bg_color, int mode);

extern const uint8_t cyclopsPreambleSymbols[];

typedef struct{
    //TODO: move to a more conservative data storage scheme
    int dataLen; //The number of bytes in the packet
    int id; //The packet sequence ID
    uint8_t modMode; //The modulation type of this packet
    uint8_t packetType; //The type of packet
    int rxCount; //The reception number of this packet
    char data[MAX_PAYLOAD_PLUS_CRC_LEN]; //A byte array to store the packet.  This is sized for the max size packet
} rx_packet_t; //This is used for Rx packets

typedef struct{
    int byteInPacket;
    uint8_t modMode;
    uint8_t type;
    uint8_t src;
    uint8_t dst;
    int16_t netID;
    int16_t length;
    int rxCount;
    rx_packet_t* currentPacket;
} rx_decoder_state_t;

typedef struct{
    rx_packet_t* packetBuffer;
    int packetBufferSize;
    int packetBufferOccupancy;
    int packetBufferReadInd;
} packet_buffer_state_t;

//This includes each symbol being repeated 4 times.  Each symbol is placed in a unit of MAX_BITS_PER_SYMBOL bits
int createRawPreamble(TX_SYMBOL_DATATYPE *rawPreambleBuf, TX_MODTYPE_DATATYPE *preambleModulationBuf);

//The packet has the following format
/*
 *   ************************
 *   *                      *
 *   *       Preamble       *
 *   *       2752 Symb      *
 *   *         BPSK         *
 *   *                      *
 *   ************************
 *   *            * ModType *
 *   *   Header   * 8 Symb  *
 *   *    BPSK    ***********
 *   *            * Type    *
 *   *            * 8 Symb  *
 *   *            ***********
 *   *            * Src     *
 *   *            * 8 Symb  *
 *   *            ***********
 *   *            * Dst     *
 *   *            * 8 Symb  *
 *   *            ***********
 *   *            * Net_ID  *
 *   *            * 16 Symb *
 *   *            ***********
 *   *            * Length  *
 *   *            * 16 Symb *
 *   ************************
 *   *                      *
 *   *       Payload        *
 *   *   BPSK = 1532 bytes  *
 *   *   QPSK = 3064 bytes  *
 *   *  16QAM = 6128 bytes  *
 *   *                      *
 *   ************************
 *   *                      *
 *   *         CRC          *
 *   *       4 bytes        *
 *   *    BPSK/QPSK/16QAM   *
 *   *                      *
 *   ************************
 *
 *   Note: Bit endieness is little (lsb is transmitted first)
 *   Note: Byte endianess is little (LSB is transmitted first)
 *
 *   Note: Length is The number of bytes in the payload
 *   Type = 0 is a standard frame
 *
 *   Warning: CRC will not be implemented in this version
 *   TODO: Implement CRC
 */

int createRawHeader(TX_SYMBOL_DATATYPE *rawHeaderBuf, TX_MODTYPE_DATATYPE *headerModulationBuf, uint8_t modType, uint8_t type, uint8_t src, uint8_t dst, uint16_t netID, uint16_t length);

int createRawASCIIPayload(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, const char* message, uint16_t payloadLenBytes, uint16_t bitsPerSymbol, int* msgBytesRead);

//TODO: Implement CRC.  For now, just adds padding
int createCRC(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint16_t bitsPerSymbol);

//This includes each symbol being repeated 4 times.  Each symbol is placed in a unit of MAX_BITS_PER_SYMBOL bits
//It also involves breaking each ASCII character into symbols
//For now, will restrict so that TX_SYMBOL_DATATYPE is a multiple of the symbol size (ie. 2 or more ASCII characters will never share a symbol)
//NOTE: The message should be null terminated
//The arrays must be pre-allocated
int createRawCyclopsFrame(TX_SYMBOL_DATATYPE *packetBuffer, TX_MODTYPE_DATATYPE *modeModeBuffer, uint8_t src, uint8_t dst, uint16_t netID, int bitsPerPayloadSymbol, const char* message, int* msgBytesRead);

//Note: We can read directly from the packed data stream once filtering for valid and strobe andf repacking
//Sanitized for printable characters
//Returns the number of filtered elements
//PhaseCounter should be set to 0 in the initial call
int filterRepackRxData(RX_PACKED_DATATYPE* resultPacked, RX_PACKED_LAST_DATATYPE* resultPackedLast, const RX_PACKED_DATATYPE* rawPacked, const RX_PACKED_LAST_DATATYPE* rawPackedLast, const RX_STROBE_DATATYPE* rawStrobe, const RX_PACKED_VALID_DATATYPE* rawValid, int rawLen, RX_PACKED_DATATYPE *remainingPacked, RX_PACKED_LAST_DATATYPE *remainingLast, int *remainingBits, int *phaseCounter);

int unpackToSymbols(TX_SYMBOL_DATATYPE *symbolBuf, TX_MODTYPE_DATATYPE *modulationBuf, uint64_t val, uint8_t bitsPerVal, uint8_t bitsPerSymbol, uint8_t symbolRepitions);

//The latter pointers are used to carry state between calls in case the header is in the middle of being parsed between 2 calls to the function.  Byte in packet is used to track if the
void printPacket(const RX_PACKED_DATATYPE* packedFiltered, const RX_PACKED_LAST_DATATYPE* packedFilteredLast, int packedFilteredLen, int* byteInPacket, uint8_t* modMode, uint8_t* type, uint8_t* src, uint8_t* dst, int16_t *netID, int16_t *length, bool printDetails, int* rxCount);

void parsePacket(const RX_PACKED_DATATYPE* packedFiltered, const RX_PACKED_LAST_DATATYPE* packedFilteredLast, int packedFilteredLen, rx_decoder_state_t *decoderState, packet_buffer_state_t* packetBufferState);

void printPacketStruct(rx_packet_t* packet, int ch, bool printTitle, bool printDetails, bool printContent);

void processPackets(packet_buffer_state_t** buffers, int numBuffers, int* currentID, int maxID, int *currentBuffer, int *failureCount, int maxFailures, bool printTitle, bool printDetails, bool printContent);

#endif //CYCLOPSASCIILINK_CYCLOPSFRAMING_H
