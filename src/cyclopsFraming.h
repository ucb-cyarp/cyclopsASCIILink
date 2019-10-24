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

#define MOD_TYPE_BPSK (0)
#define MOD_TYPE_QPSK (1)
#define MOD_TYPE_QAM16 (2)

#define PADDING_BYTE_BPSK ('U')
#define PADDING_BYTE_QPSK ('3')
#define PADDING_BYTE_QAM16 ('(')

#define BPSK_PAYLOAD_LEN_BYTES (1532)
#define QPSK_PAYLOAD_LEN_BYTES (3064)
#define QAM16_PAYLOAD_LEN_BYTES (6128)

extern const uint8_t cyclopsPreambleSymbols[];

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

//Note: We can read directly from the packed data stream once filtering for valid and strobe
//Returns the number of filtered elements
int filterRxData(RX_PACKED_DATATYPE* resultPacked, RX_PACKED_LAST_DATATYPE* resultPackedLast, const RX_PACKED_DATATYPE* rawPacked, const RX_PACKED_LAST_DATATYPE* rawPackedLast, const RX_STROBE_DATATYPE* rawStrobe, const RX_PACKED_VALID_DATATYPE* rawValid, int rawLen);

int unpackToSymbols(TX_SYMBOL_DATATYPE *symbolBuf, TX_MODTYPE_DATATYPE *modulationBuf, uint64_t val, uint8_t bitsPerVal, uint8_t bitsPerSymbol, uint8_t symbolRepitions);

//The latter pointers are used to carry state between calls in case the header is in the middle of being parsed between 2 calls to the function.  Byte in packet is used to track if the
void printPacket(const RX_PACKED_DATATYPE* packedFiltered, const RX_PACKED_LAST_DATATYPE* packedFilteredLast, int packedFilteredLen, int* byteInPacket, uint8_t* modMode, uint8_t* type, uint8_t* src, uint8_t* dst, int16_t *netID, int16_t *length, bool printDetails);

#endif //CYCLOPSASCIILINK_CYCLOPSFRAMING_H
