/********************************************************************************************************
 * @file    rc_ir_learn.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"

#include "rc_ir.h"
#include "rc_ir_learn.h"

#if (REMOTE_IR_LEARN_ENABLE)


ir_learn_ctrl_t	ir_learn_ctrl;
ir_learn_ctrl_t *g_ir_learn_ctrl = &ir_learn_ctrl;
ir_send_dma_data_t ir_send_dma_data;
extern ir_send_ctrl_t ir_send_ctrl;

/************************************************************************
* Functionir_learn_init(void)				  			 				*
* Description ir learn init algorithm ,set related GPIO function and	*
* 			    irq related. 											*
************************************************************************/
void ir_learn_init(void)
{
	#if(MCU_CORE_TYPE!=MCU_CORE_TC321X)
		memset((void*)g_ir_learn_ctrl, 0, sizeof(ir_learn_ctrl_t));  //ir_learn_ctrl_t is same as YF

		// To output ircontrol and irout low, then ir receive can work.
		gpio_set_func(GPIO_IR_OUT, AS_GPIO);
		gpio_set_input_en(GPIO_IR_OUT, 0);
		gpio_set_output_en(GPIO_IR_OUT, 1);
		gpio_set_data_strength(GPIO_IR_OUT, 1);
		gpio_write(GPIO_IR_OUT, 0);

		gpio_set_func(GPIO_IR_CONTROL, AS_GPIO);
		gpio_set_input_en(GPIO_IR_CONTROL, 0);
		gpio_set_output_en(GPIO_IR_CONTROL, 1);
		gpio_set_data_strength(GPIO_IR_CONTROL, 1);//ʹܽGPIOΪʱЧ
		gpio_write(GPIO_IR_CONTROL, 0);//IRcontrol low ,enable IR learn func.

		gpio_set_func(GPIO_IR_LEARN_IN, AS_GPIO);
		gpio_set_input_en(GPIO_IR_LEARN_IN, 1);
		gpio_set_output_en(GPIO_IR_LEARN_IN, 0);
		gpio_setup_up_down_resistor(GPIO_IR_LEARN_IN, PM_PIN_PULLUP_10K);  //open pull up resistor
		gpio_set_interrupt_pol(GPIO_IR_LEARN_IN, pol_falling);    //falling edge

		reg_irq_src = FLD_IRQ_GPIO_EN; //clean irq status
		reg_irq_mask |= FLD_IRQ_GPIO_EN;
	#else
		ir_learn_rx_t ir_learn_rx = {
			.cnt_mode      = RISING_EDGE_START_CNT,
			.rx_mode       = ANALOG_RX_MODE,
			.timeout_cnt   = TICK_VALUE_1048575,
		};
		if (g_chip_version == CHIP_VERSION_A0) {
			ir_learn_rx.cnt_mode = FALLING_EDGE_START_CNT;
		}
		ir_learn_rx_init(&ir_learn_rx);
		ir_learn_ana_rx_dis();
		memset(g_ir_learn_ctrl, 0, sizeof(g_ir_learn_ctrl));
	#endif
}


/***********************************************************************
* Functionir_record(u32 tm) 				  			 			   *
* DescriptionIR learn algorithm ,need to put in ram code when 	   *
*               learning high frequency carrier wave.  				   *
* Parametertm:clock time now ,to count interval between 2 interrupt  *
***********************************************************************/
_attribute_ram_code_
static inline void ir_record(u32 tm)
{
	//1.add irq cnt
	++ g_ir_learn_ctrl->ir_enter_irq_cnt;		  //add irq counter
	g_ir_learn_ctrl->curr_trigger_tm_point = tm;  //record latest time
	g_ir_learn_ctrl->time_interval = (u32)(g_ir_learn_ctrl->curr_trigger_tm_point - g_ir_learn_ctrl->last_trigger_tm_point);  // count interval

	//2.first irq , do following things :
	// a>record irq time
	// b>change state to IR_LEARN_BEGIN
	if(g_ir_learn_ctrl->ir_enter_irq_cnt == 1 )
	{
		g_ir_learn_ctrl -> ir_learn_tick = tm;
		g_ir_learn_ctrl -> ir_learn_state = IR_LEARN_BEGIN;
	}

	//3.second irq , do following things if time interval is valid:
	//	a>set carrier or not symbol,stand for is recording carrier.
	//  b>record first interval
	//	c>record switch start point for later calculate
	else if(g_ir_learn_ctrl->ir_enter_irq_cnt == 2 )
	{
		//second come in
		//if first interval too long ,it fails.
		if(g_ir_learn_ctrl->time_interval > IR_LEARN_INTERVAL_THRESHOLD)
		{
			g_ir_learn_ctrl->ir_learn_state = IR_LEARN_FAIL_FIRST_INTERVAL_TOO_LONG;
		    return;
		}
		//if first interval is valid, IR learn process begin.
		else
		{
			g_ir_learn_ctrl->carr_or_not = 1;
			g_ir_learn_ctrl->carr_first_interval = g_ir_learn_ctrl->time_interval;
			g_ir_learn_ctrl->carr_switch_start_tm_point = g_ir_learn_ctrl->last_trigger_tm_point;
		}
	}

	//4.other irq
	else
	{
		// time interval less than IR_LEARN_INTERVAL_THRESHOLD, means it's normal carrier
		if(g_ir_learn_ctrl->time_interval < IR_LEARN_INTERVAL_THRESHOLD)
		{
			//if previews irq is carrier, symbol is 1. else here is 0 ,need to record in new buff.
			if(g_ir_learn_ctrl->carr_or_not == 0)
			{
				g_ir_learn_ctrl->wave_series_cnt ++;
				g_ir_learn_ctrl->carr_or_not = 1;
			}
			g_ir_learn_ctrl->wave_series_buf[g_ir_learn_ctrl->wave_series_cnt] = \
					(u32)(g_ir_learn_ctrl->curr_trigger_tm_point - g_ir_learn_ctrl->carr_switch_start_tm_point);
		}
		// this part means it's change from carrier to non_carrier
		else if( IR_LEARN_INTERVAL_THRESHOLD < g_ir_learn_ctrl->time_interval && \
				g_ir_learn_ctrl->time_interval < IR_LEARN_END_THRESHOLD)
		{
			DBG_CHN2_HIGH;
			if(g_ir_learn_ctrl->carr_or_not == 1)
			{
				g_ir_learn_ctrl->carr_or_not = 0;
				g_ir_learn_ctrl->carr_switch_start_tm_point = tm;
				g_ir_learn_ctrl->wave_series_buf[g_ir_learn_ctrl->wave_series_cnt] += g_ir_learn_ctrl->carr_first_interval;
				g_ir_learn_ctrl->wave_series_cnt ++;
				g_ir_learn_ctrl->wave_series_buf[g_ir_learn_ctrl->wave_series_cnt] = (u32)(g_ir_learn_ctrl->time_interval - g_ir_learn_ctrl->carr_first_interval/3);
			}
			else
			{
				g_ir_learn_ctrl->ir_learn_state = IR_LEARN_FAIL_TWO_LONG_NO_CARRIER;
				return;
			}
			DBG_CHN2_LOW;
		}
		//if next wave comes too late, we consider the process is over,the latest wave is useless.
		else if(g_ir_learn_ctrl->time_interval>IR_LEARN_END_THRESHOLD)
		{
			g_ir_learn_ctrl->ir_learn_state = IR_LEARN_SAMPLE_END;
		}

		//as long as the process begins,record first (IR_CARR_CHECK_CNT) interval and choose shortest as carrier.
		if(g_ir_learn_ctrl->carr_check_cnt < IR_CARR_CHECK_CNT)
		{
			if(g_ir_learn_ctrl->carr_cycle_interval < g_ir_learn_ctrl->time_interval)
			{
				g_ir_learn_ctrl->carr_cycle_interval = g_ir_learn_ctrl->time_interval;
			}
			++g_ir_learn_ctrl->carr_check_cnt;
		}

		if(g_ir_learn_ctrl->wave_series_cnt >= MAX_SECTION_NUMBER)
		{
			g_ir_learn_ctrl->ir_learn_state = IR_LEARN_FAIL_FLASH_FULL;
		}
	}

	//always record last trigger time
    g_ir_learn_ctrl->last_trigger_tm_point = g_ir_learn_ctrl->curr_trigger_tm_point;
}

/***********************************************************************
* Functionir_learn_irq_handler(void)			  			 		   *
* DescriptionIR learn process in irq. 				  			   *
***********************************************************************/
_attribute_ram_code_
void ir_learn_irq_handler(void)
{
    if ((g_ir_learn_ctrl -> ir_learn_state != IR_LEARN_WAIT_KEY) && (g_ir_learn_ctrl -> ir_learn_state != IR_LEARN_BEGIN))
    {
        return;
    }
	#if(MCU_CORE_TYPE!=MCU_CORE_TC321X)
		u32 src = reg_irq_src;
		if ((src & IR_LEARN_INTERRUPT_MASK))
		{
			reg_irq_src = IR_LEARN_INTERRUPT_MASK;
			ir_record(clock_time());  // IR Learning
		}
	#else
		if( ir_learn_get_irq_status(IR_LEARN_CYCLE_IRQ))
		{
			ir_learn_clr_irq_status(IR_LEARN_CYCLE_IRQ);
			ir_learn_clr_irq_status(IR_LEARN_HIGH_IRQ);
			ir_record(clock_time());  // IR Learning
		}
		if( ir_learn_get_irq_status(IR_LEARN_TIMEOUT_IRQ))
		{
			/**
			 * When a timeout interrupt occurs, there is still the last piece of data in the high register.
			 * Therefore, in the timeout interrupt, we check the high interrupt to retrieve the last value from the high register.
			 */
			ir_learn_clr_irq_status(IR_LEARN_TIMEOUT_IRQ);
		}
	#endif
}

/***********************************************************************
* Functionir_learn_send_init(void)				  			 	   *
* Description IR learn send init,set pwn and irq related.			   *
***********************************************************************/
void ir_learn_send_init(void)
{
	//only pwm0 support fifo mode
	pwm_set_clk(CLOCK_SYS_CLOCK_HZ,16000000);
	#if(MCU_CORE_TYPE!=MCU_CORE_TC321X)
        pwm_n_revert(PWM0_ID);	//if use PWMx_N, user must set "pwm_n_revert" before gpio_set_func(pwmx_N).
        gpio_set_func(GPIO_IR_OUT, AS_PWM0_N);
	#endif
	pwm_set_mode(PWM0_ID, PWM_IR_DMA_FIFO_MODE);
	pwm_set_phase(PWM0_ID, 0);   //no phase at pwm beginning

	pwm_set_dma_address(&ir_send_dma_data);
	#if(MCU_CORE_TYPE==MCU_CORE_TC321X)
    ir_learn_tx_t ir_learn_tx = {
        .tx_mode = ANALOG_TX_MODE,
    };
    ir_learn_tx_init(&ir_learn_tx);
	#endif
	//add fifo stop irq, when all waveform send over, this irq will triggers
	//enable system irq PWM
	reg_irq_mask |= FLD_IRQ_SW_PWM_EN;

	//enable pwm0 fifo stop irq
	reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //clear irq status

	ir_send_ctrl.last_cmd = 0xff; //must
}

/***********************************************************************
* Functionir_learn_start(void)					  			 	   *
* Description Begin IR learn process.			   					   *
***********************************************************************/
void ir_learn_start(void)
{
	memset(&ir_learn_ctrl,0,sizeof(ir_learn_ctrl));
	#if(MCU_CORE_TYPE!=MCU_CORE_TC321X)
        reg_irq_src = IR_LEARN_INTERRUPT_MASK;
        gpio_en_interrupt(GPIO_IR_LEARN_IN, 1);
	#else
        irq_set_mask(FLD_IRQ_IR_LEARN_EN);
        ir_learn_set_irq_mask(IR_LEARN_CYCLE_IRQ|IR_LEARN_TIMEOUT_IRQ);
        ir_learn_ana_rx_en();
        ir_learn_en();
	#endif
	g_ir_learn_ctrl -> ir_learn_state = IR_LEARN_WAIT_KEY;
	g_ir_learn_ctrl -> ir_learn_tick = clock_time();
}

/***********************************************************************
* Functionir_learn_start(void)					  			 	   *
* Description Stop IR learn process.			   					   *
***********************************************************************/
void ir_learn_stop(void)
{
	#if(MCU_CORE_TYPE!=MCU_CORE_TC321X)
    reg_irq_src = IR_LEARN_INTERRUPT_MASK;
	gpio_en_interrupt(GPIO_IR_LEARN_IN, 0);
	#else
    irq_disable_type(FLD_IRQ_IR_LEARN_EN);
    ir_learn_clr_irq_mask(IR_LEARN_CYCLE_IRQ|IR_LEARN_TIMEOUT_IRQ);
    ir_learn_ana_rx_dis();
    ir_learn_dis();
	#endif
}

/***********************************************************************
* Functionget_ir_learn_state(void)				  			 	   *
* Description Get ir learn state in UI layer.						   *
*  				0 	 : ir learn success								   *
* 				1 	 : ir learn is doing or disable					   *
* 				else : ir learn fail ,return fail reason,			   *
* 					   match enum ir_learn_states 	         		   *
***********************************************************************/
unsigned char get_ir_learn_state(void)
{
    if(g_ir_learn_ctrl -> ir_learn_state == IR_LEARN_SUCCESS)
        return 0;
    else if(g_ir_learn_ctrl -> ir_learn_state == IR_LEARN_DISABLE)
        return 2;
    else if(g_ir_learn_ctrl -> ir_learn_state < IR_LEARN_SUCCESS)
        return 1;
    else
        return (g_ir_learn_ctrl -> ir_learn_state);
}

/***********************************************************************
* Functionir_learn_copy_result(ir_learn_send_t* send_buffer)				  			 	   *
* Description Copy necessary parameter to send_buffer to 			   					   *
***********************************************************************/
void ir_learn_copy_result(ir_learn_send_t* send_buffer)
{
	ir_learn_send_t* g_ir_learn_send = send_buffer;
	g_ir_learn_send -> ir_learn_carrier_cycle = g_ir_learn_ctrl -> carr_cycle_interval;
	g_ir_learn_send -> ir_learn_wave_num = g_ir_learn_ctrl -> wave_series_cnt;
	for(u32 i=0;i<(g_ir_learn_ctrl -> wave_series_cnt)+1;i++)
	{
		g_ir_learn_send -> ir_lenrn_send_buf[i] = g_ir_learn_ctrl -> wave_series_buf[i];
	}
}

/***********************************************************************
* Functionir_learn_detect(void)					  			 	   *
* DescriptionIR learn deal process,better to use it every loop	   *
***********************************************************************/
void ir_learn_detect(void)
{
	//ir learn overtime
	if( (g_ir_learn_ctrl -> ir_learn_state == IR_LEARN_WAIT_KEY) && clock_time_exceed(g_ir_learn_ctrl -> ir_learn_tick , IR_LEARN_OVERTIME_THRESHOLD) )	//10s overtime
	{
		g_ir_learn_ctrl -> ir_learn_state = IR_LEARN_FAIL_WAIT_OVER_TIME;
		ir_learn_stop();
	}
	//time beyond IR_LEARN_SAMPLE_END , check wave number to decide if success
	else if( ((g_ir_learn_ctrl -> ir_learn_state == IR_LEARN_BEGIN)&&(clock_time_exceed(g_ir_learn_ctrl -> curr_trigger_tm_point , IR_LEARN_END_THRESHOLD/16))) || \
		(g_ir_learn_ctrl -> ir_learn_state == IR_LEARN_SAMPLE_END) )
	{
		if(g_ir_learn_ctrl -> wave_series_cnt > CARR_AND_NO_CARR_MIN_NUMBER)
		{
			g_ir_learn_ctrl -> ir_learn_state = IR_LEARN_SUCCESS;
			g_ir_learn_ctrl -> wave_series_buf[g_ir_learn_ctrl -> wave_series_cnt-1] += g_ir_learn_ctrl -> carr_cycle_interval;	//add last carrier
			ir_learn_stop();
		}
		else
		{
			g_ir_learn_ctrl -> ir_learn_state = IR_LEARN_FAIL_WAVE_NUM_TOO_FEW;
			ir_learn_stop();
		}
	}
}

/***********************************************************************
* Functionir_learn_send(void)					  			 	  	   *
* DescriptionSend IR code that be learned. 		   			       *
* *********************************************************************/
_attribute_ram_code_
void ir_learn_send(ir_learn_send_t* send_buffer)
{
    if (ir_sending_check())
    {
        return;
    }
    ir_learn_send_t* g_ir_learn_send = send_buffer;
    u32 carrier_cycle = g_ir_learn_send->ir_learn_carrier_cycle;
    u32 carrier_high = g_ir_learn_send->ir_learn_carrier_cycle / 3;

    ir_send_dma_data.data_num = 0;

    pwm_set_cycle_and_duty(PWM0_ID, carrier_cycle,  carrier_high ); 	//config carrier: 38k, 1/3 duty

    u8 is_carrier = 1;
    foreach (i,g_ir_learn_send->ir_learn_wave_num+1)
    {
    	ir_send_dma_data.data[ir_send_dma_data.data_num ++] = pwm_config_dma_fifo_waveform(is_carrier, PWM0_PULSE_NORMAL,g_ir_learn_send->ir_lenrn_send_buf[i]/carrier_cycle);
    	is_carrier = is_carrier == 1 ? 0:1;
    }

	ir_send_dma_data.dma_len = ir_send_dma_data.data_num * 2;

	ir_send_ctrl.repeat_enable = 0;  //need repeat signal

	reg_pwm_irq_sta = FLD_IRQ_PWM0_IR_DMA_FIFO_DONE;   //clear  dma fifo mode done irq status
	reg_pwm_irq_mask |= FLD_IRQ_PWM0_IR_DMA_FIFO_DONE; //enable dma fifo mode done irq mask

	ir_send_ctrl.is_sending = IR_SENDING_DATA;

	ir_send_ctrl.sending_start_time = clock_time();

	pwm_start_dma_ir_sending();

}

#endif
