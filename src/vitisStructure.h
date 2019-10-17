//
// Created by Christopher Yarp on 10/17/19.
//

#ifndef CYCLOPSASCIILINK_VITISSTRUCTURE_H
#define CYCLOPSASCIILINK_VITISSTRUCTURE_H

#include <stdint.h>

// ** This file should be modified to include the header definition for the vitis I/O Pipes **
#include "vitisIncludes/vitisTypes.h"
#include "vitisIncludes/rx_combined_man_partition_io_linux_pipe.h"
#include "vitisIncludes/transmitter_man_partition_io_linux_pipe.h"

#define BOOL_TYPE vitisBool_t

// ** The following defines should be modified to match the included structures **
// ++ Transmitter ++
#define TX_AVAILABLE true

#define TRANSMITTER_STRUCTURE_TYPE_NAME transmitter_man_partition_inputs_bundle_1_t

#define TX_SYMBOL_MEMBER_NAME symbol_BUNDLE_1_inPort0_re
#define TX_SYMBOL_DATATYPE uint8_t

#define TX_MODTYPE_MEMBER_NAME ModMode_BUNDLE_1_inPort3_re
#define TX_MODTYPE_DATATYPE uint8_t

#define TX_GAIN_MEMBER_NAME gain_BUNDLE_1_inPort1_re
#define TX_GAIN_DATATYPE float

#define TX_ZERO_MEMBER_NAME zero_BUNDLE_1_inPort2_re
#define TX_ZERO_DATATYPE BOOL_TYPE

// ++ Receiver ++
#define RX_AVAILABLE true

#define RECEIVER_STRUCTURE_TYPE_NAME rx_combined_man_partition_outputs_bundle_2_t

#define RX_PACKED_MEMBER_NAME packed_symbol_BUNDLE_2_outPort25_re
#define RX_PACKED_DATATYPE uint8_t

#define RX_STROBE_MEMBER_NAME rx_strobe
#define RX_STROBE_DATATYPE BOOL_TYPE

#define RX_VALID_MEMBER_NAME packed_valid_BUNDLE_2_outPort24_re
#define RX_PACKED_VALID_DATATYPE BOOL_TYPE

#define RX_LAST_MEMBER_NAME packed_last_BUNDLE_2_outPort28_re
#define RX_PACKED_LAST_DATATYPE BOOL_TYPE

#endif //CYCLOPSASCIILINK_VITISSTRUCTURE_H
