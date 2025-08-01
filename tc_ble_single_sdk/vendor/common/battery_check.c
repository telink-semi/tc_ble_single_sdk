/********************************************************************************************************
 * @file    battery_check.c
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
#include "stack/ble/ble.h"
#include "battery_check.h"


#if (APP_BATT_CHECK_ENABLE)

#define ADC_SAMPLE_NUM		8


_attribute_data_retention_	u8				adc_first_flg = 1;
_attribute_data_retention_	static u8		lowBattDet_enable = 1;
#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
_attribute_data_retention_  static signed int 		batt_vol_mv;
#else
_attribute_data_retention_  static s16 		batt_vol_mv;
#endif
u8      adc_hw_initialized = 0;

extern unsigned short 	adc_gpio_calib_vref;
extern signed char 		adc_gpio_calib_vref_offset;
extern unsigned char   	adc_pre_scale;

#if (MCU_CORE_TYPE == MCU_CORE_TC321X)

	signed int sd_adc_sample_buffer[SD_ADC_FIFO_DEPTH] __attribute__((aligned(4))) = {0};

	/**
	 * @brief       This function serves to sort and get average code.
	 * @param[in]   sd_adc_data_buf -Pointer to sd_adc_data_buf
	 * @return      average code
	 * @note        If sd_adc_sort_and_get_average_code() interface is called, SD_ADC_SAMPLE_CNT must be a multiple of 4.
	 */
	signed int sd_adc_sort_and_get_average_code(signed int *sample_buffer)
	{
	    int i, j;
	    signed int sd_adc_code_average = 0;
	    signed int temp;

	    /**** insert Sort and get average value ******/
	    for(i = 1 ;i < SD_ADC_SAMPLE_CNT; i++)
	    {
	        if(sample_buffer[i] < sample_buffer[i-1])
	        {
	            temp = sample_buffer[i];
	            sample_buffer[i] = sample_buffer[i-1];
	            for(j=i-1; j>=0 && sample_buffer[j] > temp;j--)
	            {
	                sample_buffer[j+1] = sample_buffer[j];
	            }
	            sample_buffer[j+1] = temp;
	        }
	    }
	    //get average value from raw data(abandon 1/4 small and 1/4 big data)
	    for (i = SD_ADC_SAMPLE_CNT>>2; i < (SD_ADC_SAMPLE_CNT - (SD_ADC_SAMPLE_CNT>>2)); i++)
	    {
	        sd_adc_code_average += (float)sample_buffer[i]/(float)(SD_ADC_SAMPLE_CNT>>1);
	    }
	    return sd_adc_code_average;
	}

	/**
	 * @brief       This function serves to get voltage or temperature value.
	 * @param[in]   result_type -sd_adc_result_type_e
	 * @return      voltage or temperature value
	 */
	signed int sd_adc_get_result(sd_adc_result_type_e result_type)
	{
	    signed int code_average;
	    signed int sd_adc_result;

	    code_average = sd_adc_sort_and_get_average_code(sd_adc_sample_buffer);

	    if(result_type == SD_ADC_VOLTAGE_10X_MV)
	    {
	        return sd_adc_result = sd_adc_calculate_voltage(code_average,result_type);
	    }
	    else if(result_type == SD_ADC_VOLTAGE_MV)
	    {
	        return sd_adc_result = sd_adc_calculate_voltage(code_average,result_type);
	    }
	    else if((result_type == TEMP_VALUE))
	    {
	        return sd_adc_result = sd_adc_calculate_temperature(code_average);
	    }
	    else
	    {
	        return 0;
	    }
	}
#endif

/**
 * @brief      This function is used to enable battery detect
 * @param[in]  en - 1: enable;  0: disable.
 * @return     none.
 */
void battery_set_detect_enable (int en)
{
	lowBattDet_enable = en;

	/* when disable battery detect, maybe user need ADC module to process other task, probably will configure ADC
	 * settings to different mode, so here we must clear battery detect ADC setting flag, to make sure ADC settings
	 * initialized to battery detect mode when battery detect function used again
	 * so here we must clear */
	if(!en){
		#if (MCU_CORE_TYPE == MCU_CORE_827x || MCU_CORE_TYPE == MCU_CORE_825x)
			battery_clear_adc_setting_flag();  //need initialized again
		#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
			sd_adc_power_off(SD_ADC_SAMPLE_MODE);
		#endif
	}
}

/**
 * @brief      This function is used to get enable state of battery detect
 * @param[in]  none.
 * @return     0: Battery detect is disable 	 1:Battery detect is enable.
 */
int battery_get_detect_enable (void)
{
	return lowBattDet_enable;
}

/**
 * @brief		vbat detect initialization
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void adc_vbat_detect_init(void)
{
#if (MCU_CORE_TYPE == MCU_CORE_827x)
	// DFIFO Mode

	/******power off sar adc********/
	adc_power_on_sar_adc(0);

#ifdef	GPIO_VBAT_DETECT
	gpio_set_output_en(GPIO_VBAT_DETECT, 1);
	gpio_write(GPIO_VBAT_DETECT, 1);
#endif

	/******set adc sample clk as 4MHz******/
	adc_set_sample_clk(5); //adc sample clk= 24M/(1+5)=4M

	//set misc channel en,  and adc state machine state cnt 2( "set" stage and "capture" state for misc channel)
	adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);  	//set total length for sampling state machine and channel

	//set "capture state" length for misc channel: 240
	//set "set state" length for misc channel: 10
	//adc state machine  period  = 24M/250 = 96K, T = 10.4 uS
	adc_set_state_length(240, 10);  	//set R_max_mc,R_max_c,R_max_s
#ifndef	GPIO_VBAT_DETECT
	adc_set_vref_vbat_divider(ADC_VBAT_DIVIDER_1F3);
#endif


#if 1  //optimize, for saving time
	//set misc channel use differential_mode,
	//set misc channel resolution 14 bit,  misc channel differential mode
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	analog_write (anareg_adc_res_m, RES14 | FLD_ADC_EN_DIFF_CHN_M);
	adc_set_ain_chn_misc(ADC_INPUT_PCHN, GND);
#else
////set misc channel use differential_mode,
	adc_set_ain_channel_differential_mode(ADC_INPUT_PCHN, GND);

	//set misc channel resolution 14 bit
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	adc_set_resolution(RES14);
#endif

#ifndef	GPIO_VBAT_DETECT
	adc_set_ain_channel_differential_mode(VBAT, GND);
#endif
	//set misc channel vref 1.2V
	adc_set_ref_voltage(ADC_VREF_1P2V);


	//set misc t_sample 6 cycle of adc clock:  6 * 1/4M
#if 1   //optimize, for saving time
	adc_set_tsample_cycle_chn_misc(SAMPLING_CYCLES_6);  	//Number of ADC clock cycles in sampling phase
#else
	adc_set_tsample_cycle(SAMPLING_CYCLES_6);   	//Number of ADC clock cycles in sampling phase
#endif

#ifndef	GPIO_VBAT_DETECT
		//set Analog input pre-scal.ing 1
		adc_set_ain_pre_scaler(ADC_PRESCALER_1);
#else
		//set Analog input pre-scal.ing 1/8
		adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);
#endif

	/******power on sar adc********/
	//note: this setting must be set after all other settings
	adc_power_on_sar_adc(1);


#elif (MCU_CORE_TYPE == MCU_CORE_825x)
	// DFIFO Mode

	/******power off sar adc********/
	adc_power_on_sar_adc(0);

	//telink advice: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	gpio_set_output_en(GPIO_VBAT_DETECT, 1);
	gpio_write(GPIO_VBAT_DETECT, 1);


	/******set adc sample clk as 4MHz******/
	adc_set_sample_clk(5); //adc sample clk= 24M/(1+5)=4M

	/******set adc L R channel Gain Stage bias current trimming******/
	adc_set_left_right_gain_bias(GAIN_STAGE_BIAS_PER100, GAIN_STAGE_BIAS_PER100);

	//set misc channel en,  and adc state machine state cnt 2( "set" stage and "capture" state for misc channel)
	adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);  	//set total length for sampling state machine and channel

	//set "capture state" length for misc channel: 240
	//set "set state" length for misc channel: 10
	//adc state machine  period  = 24M/250 = 96K, T = 10.4 uS
	adc_set_state_length(240, 0, 10);  	//set R_max_mc,R_max_c,R_max_s


#if 1  //optimize, for saving time
	//set misc channel use differential_mode,
	//set misc channel resolution 14 bit,  misc channel differential mode
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	analog_write (anareg_adc_res_m, RES14 | FLD_ADC_EN_DIFF_CHN_M);
	adc_set_ain_chn_misc(ADC_INPUT_PCHN, GND);
#else
////set misc channel use differential_mode,
	adc_set_ain_channel_differential_mode(ADC_MISC_CHN, ADC_INPUT_PCHN, GND);

	//set misc channel resolution 14 bit
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	adc_set_resolution(ADC_MISC_CHN, RES14);
#endif


	//set misc channel vref 1.2V
	adc_set_ref_voltage(ADC_MISC_CHN, ADC_VREF_1P2V);


	//set misc t_sample 6 cycle of adc clock:  6 * 1/4M
#if 1   //optimize, for saving time
	adc_set_tsample_cycle_chn_misc(SAMPLING_CYCLES_6);  	//Number of ADC clock cycles in sampling phase
#else
	adc_set_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_6);   	//Number of ADC clock cycles in sampling phase
#endif

	//set Analog input pre-scal.ing 1/8
	adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);


	/******power on sar adc********/
	//note: this setting must be set after all other settings
	adc_power_on_sar_adc(1);
#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
	sd_adc_init();
	#ifdef	GPIO_VBAT_DETECT
		sd_adc_gpio_cfg_t sd_adc_gpio_cfg =
		{
		     .input_p          = GPIO_VBAT_DETECT,
		     .input_n          = SD_ADC_GNDN,
		     .p_div            = SD_ADC_GPIO_P_CHN_DIV_1F4,
		     .n_div            = SD_ADC_GPIO_N_CHN_DIV_1F4,
			 .clk_div          = SD_ADC_SAMPLE_CLOCK_DIV,
		     .downsample_rate  = SD_ADC_DOWN_SAMPLE_RATE,
		};
		sd_adc_gpio_sample_init(&sd_adc_gpio_cfg);
		gpio_set_func((GPIO_VBAT_DETECT&0x0fff),AS_GPIO);
		gpio_set_output_en((GPIO_VBAT_DETECT&0x0fff), 1);
		gpio_set_input_en((GPIO_VBAT_DETECT&0x0fff),0);
		gpio_write((GPIO_VBAT_DETECT&0x0fff), 1);
	#else
		sd_adc_vbat_sample_init(SD_ADC_SAMPLE_CLOCK_DIV, SD_ADC_VBAT_DIV_1F4, SD_ADC_DOWN_SAMPLE_RATE);
	#endif

	sd_adc_set_rx_fifo((unsigned int *)sd_adc_sample_buffer, SD_ADC_FIFO_DEPTH);
	sd_adc_set_irq_trig_thres(SD_ADC_SAMPLE_CNT);
	/**
	 * @attention
	 * -# After sd_adc_power_on(SD_ADC_SAMPLE_MODE), must wait >160us(when the C10 capacitor on the development board is 1nF) for VMID power to stabilize, otherwise there will be hundreds of sample data will be abnormal.
	 * -# After each call to sd_adc_power_on(SD_ADC_SAMPLE_MODE) and wait >160us(when the C10 capacitor on the development board is 1nF), the first 4 codes sampled by sd_adc may be abnormal and need to be discarded.
	 */
	sd_adc_power_on(SD_ADC_SAMPLE_MODE);
	sleep_us(160);
	sd_adc_sample_start();
#endif
}



_attribute_data_retention_	volatile unsigned int adc_dat_buf[ADC_SAMPLE_NUM];  //size must 16 byte aligned(16/32/64...)

_attribute_data_retention_ u32 adc_result;

_attribute_ram_code_ int app_battery_power_check(u16 alram_vol_mv)
{
//	when MCU powered up or wakeup from deep/deep with retention, adc need be initialized
	if(!adc_hw_initialized){
		adc_vbat_detect_init();
		adc_hw_initialized = 1;
	}
	#if (MCU_CORE_TYPE == MCU_CORE_827x || MCU_CORE_TYPE == MCU_CORE_825x)
		u16 temp;
		int i,j;
		adc_reset_adc_module();

		u32 t0 = clock_time();
		u16 adc_sample[ADC_SAMPLE_NUM] = {0};


		for(i=0;i<ADC_SAMPLE_NUM;i++){   	//dfifo data clear
			adc_dat_buf[i] = 0;
		}
		while(!clock_time_exceed(t0, 25));  //wait at least 2 sample cycle(f = 96K, T = 10.4us)

		//dfifo setting will lose in suspend/deep, so we need to configure it every time
		adc_config_misc_channel_buf((u16 *)adc_dat_buf, ADC_SAMPLE_NUM<<2);  //size: ADC_SAMPLE_NUM*4
		dfifo_enable_dfifo2();


		//////////////// get adc sample data and sort these data ////////////////
		for(i=0;i<ADC_SAMPLE_NUM;i++){
			while(!adc_dat_buf[i]);


			if(adc_dat_buf[i] & BIT(13)){  //14 bit resolution, BIT(13) is sign bit, 1 means negative voltage in differential_mode
				adc_sample[i] = 0;
			}
			else{
				adc_sample[i] = ((u16)adc_dat_buf[i] & 0x1FFF);  //BIT(12..0) is valid adc result
			}


			//insert sort
			if(i){
				if(adc_sample[i] < adc_sample[i-1]){
					temp = adc_sample[i];
					adc_sample[i] = adc_sample[i-1];
					for(j=i-1;j>=0 && adc_sample[j] > temp;j--){
						adc_sample[j+1] = adc_sample[j];
					}
					adc_sample[j+1] = temp;
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////////


		dfifo_disable_dfifo2();   //misc channel data dfifo disable


		///// get average value from raw data(abandon some small and big data ), then filter with history data //////
		#if (ADC_SAMPLE_NUM == 4)  	//use middle 2 data (index: 1,2)
			u32 adc_average = (adc_sample[1] + adc_sample[2])/2;
		#elif(ADC_SAMPLE_NUM == 8) 	//use middle 4 data (index: 2,3,4,5)
			u32 adc_average = (adc_sample[2] + adc_sample[3] + adc_sample[4] + adc_sample[5])/4;
		#endif


		#if 1
			adc_result = adc_average;
		#else  	//history data filter, more accurate.
			if(adc_first_flg){
				adc_result = adc_average;
				adc_first_flg = 0;
			}
			else{
				adc_result = ( (adc_result*3) + adc_average + 2 )>>2;  //filter
			}
		#endif



		//////////////// adc sample data convert to voltage(mv) ////////////////
		//                          (Vref, adc_pre_scale)   (BIT<12~0> valid data)
		//			 =  adc_result * Vref * adc_pre_scale / 0x2000 + offset
		//           =  adc_result * Vref*adc_pre_scale >>13 + offset
		if (ADC_INPUT_PCHN == VBAT){
			extern unsigned short adc_vbat_calib_vref;
			batt_vol_mv  = ((adc_result*adc_pre_scale*adc_vbat_calib_vref)>>13)*3;
		}
		else
			batt_vol_mv  = ((adc_result*adc_pre_scale*adc_gpio_calib_vref)>>13) + adc_gpio_calib_vref_offset;
	#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
		else{
			sd_adc_power_on(SD_ADC_SAMPLE_MODE);
			sleep_us(160);
			sd_adc_sample_start();
		}
		while(!sd_adc_get_irq_status());
		sd_adc_sample_stop();
		batt_vol_mv=sd_adc_get_result(SD_ADC_VOLTAGE_MV);
		sd_adc_power_off(SD_ADC_SAMPLE_MODE);

	#endif
	tlkapi_printf(APP_BATT_CHECK_LOG_EN, "[APP][BAT] The battery power is %dmV!\n", batt_vol_mv);

	if(batt_vol_mv < alram_vol_mv){
		return 0;
	}
	return 1;

}


#endif
