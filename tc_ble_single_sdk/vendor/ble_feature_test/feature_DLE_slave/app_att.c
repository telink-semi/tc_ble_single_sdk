/********************************************************************************************************
 * @file    app_att.c
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

#include "app_att.h"

#include "app_config.h"


#if (FEATURE_TEST_MODE == TEST_LL_DATA_LENGTH_EXTENSION_SLAVE)

/**
 *  @brief  connect parameters structure for ATT
 */
typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  u16 intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  u16 latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  u16 timeout;
} gap_periConnectParams_t;

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

static const u16 userdesc_UUID	= GATT_UUID_CHAR_USER_DESC;

static const u16 serviceChangeUUID = GATT_UUID_SERVICE_CHANGE;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;

static const u16 my_appearanceUUID = GATT_UUID_APPEARANCE;

static const u16 my_periConnParamUUID = GATT_UUID_PERI_CONN_PARAM;

static const u16 my_appearance = GAP_APPEARE_UNKNOWN;

static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;

static const gap_periConnectParams_t my_periConnParameters = {8, 11, 0, 1000};

_attribute_data_retention_	static u16 serviceChangeVal[2] = {0};

_attribute_data_retention_	static u8 serviceChangeCCC[2] = {0,0};

static const u8 my_devName[] = {'f', 'e', 'a', 't', 'u', 'r', 'e'};
static const u8 my_PnPtrs [] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};



////////////////////// SPP ////////////////////////////////////
static const u8 TelinkSppServiceUUID[16]	      	    =WRAPPING_BRACES( TELINK_SPP_UUID_SERVICE);
static const u8 TelinkSppDataServer2ClientUUID[16]      = WRAPPING_BRACES(TELINK_SPP_DATA_SERVER2CLIENT);
static const u8 TelinkSppDataClient2ServerUUID[16]      = WRAPPING_BRACES(TELINK_SPP_DATA_CLIENT2SERVER);

// Spp data from Server to Client characteristic variables
static u8 SppDataServer2ClientDataCCC[2]  				= {0};
//this array will not used for sending data(directly calling HandleValueNotify API), so cut array length from 20 to 1, saving some SRAM
static u8 SppDataServer2ClientData[1] 					= {0};  //SppDataServer2ClientData[20]
// Spp data from Client to Server characteristic variables
//this array will not used for receiving data(data processed by Attribute Write CallBack function), so cut array length from 20 to 1, saving some SRAM
static u8 SppDataClient2ServerData[1] 					= {0};  //SppDataClient2ServerData[20]

//SPP data descriptor
static const u8 TelinkSPPS2CDescriptor[] 		 		= "Telink SPP: Module->Phone";
static const u8 TelinkSPPC2SDescriptor[]        		= "Telink SPP: Phone->Module";


//// GAP attribute values
static const u8 my_devNameCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
	U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
};
static const u8 my_appearanceCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
	U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
};
static const u8 my_periConnParamCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
	U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
};


//// GATT attribute values
static const u8 my_serviceChangeCharVal[5] = {
	CHAR_PROP_INDICATE,
	U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
	U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
};


//// device Information  attribute values
static const u8 my_PnCharVal[5] = {
	CHAR_PROP_READ,
	U16_LO(DeviceInformation_pnpID_DP_H), U16_HI(DeviceInformation_pnpID_DP_H),
	U16_LO(CHARACTERISTIC_UUID_PNP_ID), U16_HI(CHARACTERISTIC_UUID_PNP_ID)
};




//// Telink spp  attribute values
static const u8 TelinkSppDataServer2ClientCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(SPP_SERVER_TO_CLIENT_DP_H), U16_HI(SPP_SERVER_TO_CLIENT_DP_H),
	TELINK_SPP_DATA_SERVER2CLIENT
};
static const u8 TelinkSppDataClient2ServerCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(SPP_CLIENT_TO_SERVER_DP_H), U16_HI(SPP_CLIENT_TO_SERVER_DP_H),
	TELINK_SPP_DATA_CLIENT2SERVER
};

extern int module_onReceiveData(void *p);

// TM : to modify
static const attribute_t my_Attributes[] = {

	// 0x0000
	{ATT_END_H - 1, 0,0,0,0,0,0,0},	// total num of attribute


	// 0x0001 - 0x0007  GAP
	{7,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gapServiceUUID), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharVal),(u8*)(&my_characterUUID), (u8*)(my_devNameCharVal), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devName), (u8*)(&my_devNameUUID), (u8*)(my_devName), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharVal),(u8*)(&my_characterUUID), (u8*)(my_appearanceCharVal), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_appearance), (u8*)(&my_appearanceUUID), 	(u8*)(&my_appearance), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamCharVal),(u8*)(&my_characterUUID), (u8*)(my_periConnParamCharVal), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_periConnParameters),(u8*)(&my_periConnParamUUID), 	(u8*)(&my_periConnParameters), 0, 0},


	// 0x0008 - 0x000B GATT
	{4,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gattServiceUUID), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_serviceChangeCharVal),(u8*)(&my_characterUUID), 		(u8*)(my_serviceChangeCharVal), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (serviceChangeVal), (u8*)(&serviceChangeUUID), 	(u8*)(&serviceChangeVal), 0, 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof (serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0, 0},


	// 0x000C - 0x000E  device Information Service
	{3,ATT_PERMISSIONS_READ,2,2,(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_devServiceUUID), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnCharVal),(u8*)(&my_characterUUID), (u8*)(my_PnCharVal), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof (my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0, 0},


    // 0x000F - 0x0016 SPP
	{8,ATT_PERMISSIONS_READ,2,16,(u8*)(&my_primaryServiceUUID), 	(u8*)(&TelinkSppServiceUUID), 0, 0},
    // server to client TX
	{0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSppDataServer2ClientCharVal),(u8*)(&my_characterUUID), 		(u8*)(TelinkSppDataServer2ClientCharVal), 0, 0},				//prop
	{0,ATT_PERMISSIONS_READ,16,sizeof(SppDataServer2ClientData),(u8*)(&TelinkSppDataServer2ClientUUID),  (u8*)(SppDataServer2ClientData), 0, 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,2,(u8*)&clientCharacterCfgUUID,(u8*)(&SppDataServer2ClientDataCCC), 0, 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSPPS2CDescriptor),(u8*)&userdesc_UUID,(u8*)(&TelinkSPPS2CDescriptor), 0, 0},
	// client to server RX
	{0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSppDataClient2ServerCharVal),(u8*)(&my_characterUUID), 		(u8*)(TelinkSppDataClient2ServerCharVal), 0, 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,16,sizeof(SppDataClient2ServerData),(u8*)(&TelinkSppDataClient2ServerUUID), (u8*)(SppDataClient2ServerData), &module_onReceiveData, 0},	//value
	{0,ATT_PERMISSIONS_READ,2,sizeof(TelinkSPPC2SDescriptor),(u8*)&userdesc_UUID,(u8*)(&TelinkSPPC2SDescriptor), 0, 0},
};

/**
 * @brief      Initialize the attribute table
 * @param[in]  none
 * @return     none
 */
void	my_att_init(void)
{
	bls_att_setAttributeTable((u8 *)my_Attributes);
}

#endif  //end of FEATURE_TEST_MODE
