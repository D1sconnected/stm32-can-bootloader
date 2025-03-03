#ifndef __CAN_ID_H__
#define __CAN_ID_H__

#ifdef __cplusplus
extern "C" {
#endif

// 0x0000-0x000F General System Control Commands ***************
#define CAN_ID_BUS_CONTROL                              0x0000
#define CAN_ID_BUS_CONTROL_OFF                          0x00
#define CAN_ID_BUS_CONTROL_ON                           0x01
// ------------------------------------------------------------
#define CAN_ID_APP_SWITCH                               0x0001
#define CAN_ID_APP_F407_BOARD_CODE                      0x01
#define CAN_ID_APP_F103_BOARD_CODE                      0x02
#define CAN_ID_APP_SWITCH_BOOTLOADER                    0x00
#define CAN_ID_APP_SWITCH_MAIN                          0x01
// ------------------------------------------------------------
#define CAN_ID_UPDATE_PROGRESS                          0x0004
#define CAN_ID_UPDATE_PROGRESS_DLC                      5
// ****************** UPDATE COMMANDS *************************
// 0x0010-0x002F ---- F407 Board Update Commands --------------
#define CAN_ID_UPDATE_F407_BOARD_ERASE                  0x0010
#define CAN_ID_UPDATE_F407_BOARD_ERASE_ACK              0x0011
#define CAN_ID_UPDATE_F407_BOARD_CHUNK                  0x0012
#define CAN_ID_UPDATE_F407_BOARD_CRC                    0x0013
#define CAN_ID_UPDATE_F407_BOARD_CRC_ACK                0x0014
#define CAN_ID_UPDATE_F407_BOARD_TIMESTAMP              0x0015
#define CAN_ID_UPDATE_F407_BOARD_TIMESTAMP_ACK          0x0016
// 0x0030-0x004F ---- F103 Board Update Commands --------------
#define CAN_ID_UPDATE_F103_BOARD_ERASE                  0x0030
#define CAN_ID_UPDATE_F103_BOARD_ERASE_ACK              0x0031
#define CAN_ID_UPDATE_F103_BOARD_CHUNK                  0x0032
#define CAN_ID_UPDATE_F103_BOARD_CRC                    0x0033
#define CAN_ID_UPDATE_F103_BOARD_CRC_ACK                0x0034
#define CAN_ID_UPDATE_F103_BOARD_TIMESTAMP              0x0035
#define CAN_ID_UPDATE_F103_BOARD_TIMESTAMP_ACK          0x0036
// ****************** MAIN COMMANDS ***************************
// 0x0200-0x02FF F407 Board CMD -------------------------------
#define CAN_ID_F407_BOARD_PING                          0x0200
// ------------------------------------------------------------
// 0x0300-0x03FF F103 Board CMD -------------------------------
#define CAN_ID_F103_BOARD_PING                          0x0300
// ------------------------------------------------------------
// 0x7FF - Last ID

#ifdef __cplusplus
}
#endif

#endif

