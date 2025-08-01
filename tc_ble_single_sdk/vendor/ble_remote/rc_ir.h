/********************************************************************************************************
 * @file    rc_ir.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#ifndef RC_IR_H_
#define RC_IR_H_




#define IR_CARRIER_FREQ					38000  	// 1 frame -> 1/38k -> 1000/38 = 26 us
#define PWM_CARRIER_CYCLE_TICK			( CLOCK_SYS_CLOCK_HZ/IR_CARRIER_FREQ )  //16M: 421 tick, f = 16000000/421 = 38004,T = 421/16=26.3125 us
#define PWM_CARRIER_HIGH_TICK			( PWM_CARRIER_CYCLE_TICK/3 )   // 1/3 duty

#define PWM_CARRIER_HALF_CYCLE_TICK		(PWM_CARRIER_CYCLE_TICK>>1)


#define IR_HIGH_CARR_TIME			565			// in us
#define IR_HIGH_NO_CARR_TIME		1685
#define IR_LOW_CARR_TIME			560
#define IR_LOW_NO_CARR_TIME			565
#define IR_INTRO_CARR_TIME			9000
#define IR_INTRO_NO_CARR_TIME		4500

#define IR_SWITCH_CODE              0x0d
#define IR_ADDR_CODE                0x00
#define IR_CMD_CODE                 0xbf

#define IR_REPEAT_INTERVAL_TIME     40500
#define IR_REPEAT_NO_CARR_TIME      2250
#define IR_END_TRANS_TIME			563

//#define IR_CARRIER_FREQ				37917//38222
#define IR_CARRIER_DUTY				3
#define IR_LEARN_SERIES_CNT     	160




enum{
	IR_SEND_TYPE_TIME_SERIES,
	IR_SEND_TYPE_BYTE,
	IR_SEND_TYPE_HALF_TIME_SERIES,
};

/**
 * @brief	The structure for some control parameters or status related to IR functionality.
 */
typedef struct{
	u32 cycle;
	u16 hich;
	u16 cnt;
}ir_ctrl_t;
/**
 * @brief	The structure used to controlling and sending IR signals
 */
typedef struct{
	ir_ctrl_t *ptr_irCtl;
	u8 type;
	u8 start_high;
	u8 ir_number;
	u8 code;
}ir_send_ctrl_data_t;


#define IR_GROUP_MAX		8

#define IR_SENDING_NONE  		0
#define IR_SENDING_DATA			1
#define IR_SENDING_REPEAT		2
/**
 * @brief	The structure used to IR communication control
 */
typedef struct{
	u8 is_sending;
	u8 repeat_enable;
	u8 last_cmd;
	u8 rsvd;

	u32 sending_start_time;
}ir_send_ctrl_t;

ir_send_ctrl_t ir_send_ctrl;

/**
 * @brief      this function is used to init IR
 * @param[in]  none
 * @return     none
 */
void rc_ir_init(void);

/**
 * @brief      this function is used to stop IR sending
 * @param[in]  none
 * @return     none
 */
void ir_send_release(void);

/**
 * @brief      this function is used to check IR is sending
 * @param[in]  none
 * @return     1 - IR is sending
 * 			   0 - IR is not sending
 */
int ir_sending_check(void);

/**
 * @brief      this function is used to do IR sending
 * @param[in]  addr1 - the first part of the NEC address.
 * @param[in]  addr2 - the second part of the NEC address.
 * @param[in]  cmd - the command code to be transmitted.
 * @return     none
 */
void ir_nec_send(u8 addr1, u8 addr2, u8 cmd);




#endif /* RC_IR_H_ */
