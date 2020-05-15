//
// Created by Christopher Yarp on 10/17/19.
//

#ifndef CYCLOPSASCIILINK_VITISSTRUCTURE_H
#define CYCLOPSASCIILINK_VITISSTRUCTURE_H

#include <stdint.h>

// ** This file should be modified to include the header definition for the vitis I/O Pipes **
#include "vitisIncludes/vitisTypes.h"
#include "vitisIncludes/rx_demo_io_linux_pipe.h"
#include "vitisIncludes/tx_demo_io_linux_pipe.h"
#include "vitisIncludes/rx_demo_parameters.h"
#include "vitisIncludes/tx_demo_parameters.h"

#define BOOL_TYPE vitisBool_t

// ** The following defines should be modified to match the included structures **
// ++ Transmitter ++
#define TX_AVAILABLE true

//Tx Structure
#define TX_STRUCTURE_TYPE_NAME tx_demo_inputs_bundle_1_t

//Tx BlockSize
#define TX_BLOCK_SIZE TX_DEMO_BLOCK_SIZE
//Tx Symbol
#define TX_SYMBOL_DATATYPE uint8_t
#define TX_SYMBOL_BLANK_VAL 0
//Tx ModType
#define TX_MODTYPE_DATATYPE uint8_t
#define TX_MODTYPE_BLANK_VAL 0
//Tx Gain
#define TX_GAIN_DATATYPE float
//Tx Zero
#define TX_ZERO_DATATYPE BOOL_TYPE
#define TX_ZERO_VALID_DATA false
#define TX_ZERO_BLANK_DATA true

//Ch0
#define TX_SYMBOL_CH0_MEMBER_NAME symbol_ch0_BUNDLE_1_inPort0_re
#define TX_MODTYPE_CH0_MEMBER_NAME ModMode_ch0_BUNDLE_1_inPort3_re
#define TX_GAIN_CH0_MEMBER_NAME gain_ch0_BUNDLE_1_inPort1_re
#define TX_ZERO_CH0_MEMBER_NAME zero_ch0_BUNDLE_1_inPort2_re

//Ch1
#define TX_SYMBOL_CH1_MEMBER_NAME symbol_ch1_BUNDLE_1_inPort4_re
#define TX_MODTYPE_CH1_MEMBER_NAME ModMode_ch1_BUNDLE_1_inPort7_re
#define TX_GAIN_CH1_MEMBER_NAME gain_ch1_BUNDLE_1_inPort5_re
#define TX_ZERO_CH1_MEMBER_NAME zero_ch1_BUNDLE_1_inPort6_re

//Ch2
#define TX_SYMBOL_CH2_MEMBER_NAME symbol_ch2_BUNDLE_1_inPort8_re
#define TX_MODTYPE_CH2_MEMBER_NAME ModMode_ch2_BUNDLE_1_inPort11_re
#define TX_GAIN_CH2_MEMBER_NAME gain_ch2_BUNDLE_1_inPort9_re
#define TX_ZERO_CH2_MEMBER_NAME zero_ch2_BUNDLE_1_inPort10_re

//Ch3
#define TX_SYMBOL_CH3_MEMBER_NAME symbol_ch3_BUNDLE_1_inPort12_re
#define TX_MODTYPE_CH3_MEMBER_NAME ModMode_ch3_BUNDLE_1_inPort15_re
#define TX_GAIN_CH3_MEMBER_NAME gain_ch3_BUNDLE_1_inPort13_re
#define TX_ZERO_CH3_MEMBER_NAME zero_ch3_BUNDLE_1_inPort14_re

// ++ Receiver ++
#define RX_AVAILABLE true
#define RX_PACKED_BITS 4 //There are 4 bits per packed

#define RX_BLOCK_SIZE RX_DEMO_BLOCK_SIZE
#define RX_STRUCTURE_TYPE_NAME rx_demo_outputs_bundle_2_t

#define RX_PACKED_DATATYPE uint8_t
#define RX_STROBE_DATATYPE BOOL_TYPE
#define RX_PACKED_VALID_DATATYPE BOOL_TYPE
#define RX_PACKED_LAST_DATATYPE BOOL_TYPE

//Ch0
#define RX_PACKED_CH0_MEMBER_NAME packed_symbol_ch0_BUNDLE_2_outPort2_re
#define RX_STROBE_CH0_MEMBER_NAME strobe_ch0_BUNDLE_2_outPort0_re
#define RX_VALID_CH0_MEMBER_NAME packed_valid_ch0_BUNDLE_2_outPort1_re
#define RX_LAST_CH0_MEMBER_NAME packed_last_ch0_BUNDLE_2_outPort3_re

//Ch1
#define RX_PACKED_CH1_MEMBER_NAME packed_symbol_ch1_BUNDLE_2_outPort7_re
#define RX_STROBE_CH1_MEMBER_NAME strobe_ch1_BUNDLE_2_outPort5_re
#define RX_VALID_CH1_MEMBER_NAME packed_valid_ch1_BUNDLE_2_outPort6_re
#define RX_LAST_CH1_MEMBER_NAME packed_last_ch1_BUNDLE_2_outPort8_re

//Ch2
#define RX_PACKED_CH2_MEMBER_NAME packed_symbol_ch2_BUNDLE_2_outPort12_re
#define RX_STROBE_CH2_MEMBER_NAME strobe_ch2_BUNDLE_2_outPort10_re
#define RX_VALID_CH2_MEMBER_NAME packed_valid_ch2_BUNDLE_2_outPort11_re
#define RX_LAST_CH2_MEMBER_NAME packed_last_ch2_BUNDLE_2_outPort13_re

//Ch3
#define RX_PACKED_CH3_MEMBER_NAME packed_symbol_ch3_BUNDLE_2_outPort17_re
#define RX_STROBE_CH3_MEMBER_NAME strobe_ch3_BUNDLE_2_outPort15_re
#define RX_VALID_CH3_MEMBER_NAME packed_valid_ch3_BUNDLE_2_outPort16_re
#define RX_LAST_CH3_MEMBER_NAME packed_last_ch3_BUNDLE_2_outPort18_re

#endif //CYCLOPSASCIILINK_VITISSTRUCTURE_H
