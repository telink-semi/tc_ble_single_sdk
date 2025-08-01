# V3.4.2.7 (PR)

### Version
* SDK version: tc_ble_single_sdk V3.4.2.7
* Chip Version
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0)
* Hardware Version
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform Version
  - tc_platform_sdk V3.2.0
* Toolchain Version
  - TC32 ELF GCC4.3 ( IDE: [Telink IoT Studio](https://www.telink-semi.com/development-tools) )

### Features
* **Application**
	- 2.4G
	    - Add 2.4G demo in B85/B87.
		    - 2p4g_feature
			- 2p4g_genfsk_ll
			- 2p4g_tpll
			- 2p4g_tpsll

	- 2.4G & BLE concurrent
	    - Add 2.4G & BLE concurrent demo in B85/B87/TC321X.
			- ble_slave_2_4g
	
	- 2.4G & BLE switch
		- Add 2.4G & BLE switch demo in B85/B87/TC321X.
			- ble_slave_2_4g_switch

	- Modify the audio setting code structure and fix TC321X audio setting error.
	- Delete Audio DMIC setting.
	- TC321X ble_remote demo supports IR Learning.
	- Add ADC calibration in TC321X.

### Bug Fixes
* **Application**
    - Fixed: When the local Central device supports SMP while the peer Peripheral device does not support SMP, the local Central device cannot send the Connection Update Indication packet.
	- Fixed: GPIO setting error in C1T362A5 boards.
* **Controller**
	- Fixed: Connection failures caused by some special connection parameters.
* **CoC**
	- Fixed: Incorrect parameter validation logic when establishing new CoC(Connection-oriented channels) channels.
* **Driver**
	- Fixed (TC321X A0) : MCU reboot failure in start_reboot.

### Refactoring
* **Application**
    - Support using soft timer in IDLE state.
	- Modify the 512K Flash address allocation in TC321X.
	- Rename some demo.
		- Rename b85m_2p4g_feature to 2p4g_feature.
		- Rename b85m_2p4g_genfsk_ll to 2p4g_genfsk_ll.
		- Rename b85m_2p4g_tpll to 2p4g_tpll.
		- Rename b85m_2p4g_tpsll to 2p4g_tpsll.
		- Rename b85m_feature_test to ble_feature_test.
		- Rename b85m_hci to ble_hci.
		- Rename b85m_master_kma_dongle to ble_master_kma_dongle.
		- Rename b85m_module to ble_module.
		- Rename b85m_ble_remote to ble_remote.
		- Rename b85m_ble_sample to ble_sample.

* **Controller**
	- (TC321X) Optimize power consumption in connection state.

### BREAKING CHANGES
* **Driver**
	- Add rf_private_pa.c and rf_private_pa.h to B85/B87 driver_ext folder.
	- Add ext_rf_private.h to B85/B87/TC321X driver_ext folder.

### CodeSize
* **B85**
	* ble_sample
	  - Firmware size: 53.1 kBytes
	  - SRAM size: 18.2 kBytes
	  - deepsleep retention SRAM size: 14.5 kBytes
	* ble_remote
	  - Firmware size: 64.9 kBytes
	  - SRAM size: 21.1 kBytes
	  - deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
	  - Firmware size: 61.8 kBytes
	  - SRAM size: 20.4 kBytes
	  - deepsleep retention SRAM size: 16.7 kBytes
	* master_kma_dongle
	  - Firmware size: 43.2 kBytes
	  - SRAM size: 20.2 kBytes
	* genfsk_ll(stx2rx)
	  - Firmware size: 16.6 kBytes
	  - SRAM size: 9.7 kBytes
	  - deepsleep retention SRAM size: 6.5 kBytes
	* tpll(ptx)
	  - Firmware size: 17.9 kBytes
	  - SRAM size: 12.5 kBytes
	  - deepsleep retention SRAM size: 7.3 kBytes
  * tpsll(stx2rx)
	  * Firmware size: 14.6 kBytes
	  * SRAM size: 9.8 kBytes
	  * deepsleep retention SRAM size: 6.3 kBytes
	* ble_slave_2_4g(tpsll_stx2rx)
	  * Firmware size: 54.0 kBytes
	  * SRAM size: 19.9 kBytes
	  * deepsleep retention SRAM size: 15.3 kBytes
	* ble_slave_2_4g_switch(tpsll_stx2rx)
	  - Firmware size: 57.7 kBytes
	  - SRAM size: 20.0 kBytes
	  - deepsleep retention SRAM size: 15.5 kBytes
  
* **B87**
	* ble_sample
	  - Firmware size: 55.1 kBytes
	  - SRAM size: 19.9 kBytes
	  - deepsleep retention SRAM size: 16.1 kBytes
	* ble_remote
	  - Firmware size: 66.1 kBytes
	  - SRAM size: 22.9 kBytes
	  - deepsleep retention SRAM size: 17.4 kBytes
	* ble_module
	  - Firmware size: 63.9 kBytes
	  - SRAM size: 22.2 kBytes
	  - deepsleep retention SRAM size: 18.3 kBytes
	* master_kma_dongle
	  - Firmware size: 44.8 kBytes
	  - SRAM size: 21.9 kBytes
	* genfsk_ll(stx2rx)
	  - Firmware size: 18.9 kBytes
	  - SRAM size: 11.2 kBytes
	  - deepsleep retention SRAM size: 8.0 kBytes
	* tpll(ptx)
	  - Firmware size: 20.3 kBytes
	  - SRAM size: 14.0 kBytes
	  - deepsleep retention SRAM size: 8.9 kBytes
  * tpsll(stx2rx)
	  - Firmware size: 16.9 kBytes
	  - SRAM size: 11.4 kBytes
	  - deepsleep retention SRAM size: 7.8 kBytes
	* ble_slave_2_4g(tpsll_stx2rx)
	  - Firmware size: 56.0 kBytes
	  - SRAM size: 21.4 kBytes
	  - deepsleep retention SRAM size: 16.9 kBytes
	* ble_slave_2_4g_switch(tpsll_stx2rx)
	  - Firmware size: 60.2 kBytes
	  - SRAM size: 21.8 kBytes
	  - deepsleep retention SRAM size: 17.1 kBytes

* **TC321X**
	* ble_sample
	  - Firmware size: 59.2 kBytes
	  - SRAM size: 19.1 kBytes
	  - deepsleep retention SRAM size: 15.3 kBytes
	* ble_remote
	  - Firmware size: 70.5 kBytes
	  - SRAM size: 22.0 kBytes
	  - deepsleep retention SRAM size: 16.4 kBytes
	* ble_module
	  - Firmware size: 68.7 kBytes
	  - SRAM size: 21.3 kBytes
	  - deepsleep retention SRAM size: 17.3 kBytes
	* genfsk_ll(stx2rx)
	  - Firmware size: 23.3 kBytes
	  - SRAM size: 10.9 kBytes
	  - deepsleep retention SRAM size: 7.8 kBytes
	* tpll(ptx)
	  - Firmware size: 25.8 kBytes
	  - SRAM size: 13.4 kBytes
	  - deepsleep retention SRAM size: 8.2 kBytes
  * tpsll(stx2rx)
	  - Firmware size: 21.4 kBytes
	  - SRAM size: 11.1 kBytes
	  - deepsleep retention SRAM size: 7.5 kBytes
	* ble_slave_2_4g(tpsll_stx2rx)
	  - Firmware size: 60.1 kBytes
	  - SRAM size: 20.8 kBytes
	  - deepsleep retention SRAM size: 16.4 kBytes
	* ble_slave_2_4g_switch(tpsll_stx2rx)
	  - Firmware size: 63.6 kBytes
	  - SRAM size: 21.0 kBytes
	  - deepsleep retention SRAM size: 16.5 kBytes

### Version
* SDK 版本: tc_ble_single_sdk V3.4.2.7
* Chip 版本
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0)
* Hardware 版本
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform 版本
  - tc_platform_sdk V3.2.0
* Toolchain 版本
  - TC32 ELF GCC4.3 ( IDE: [Telink IoT Studio](https://www.telink-semi.com/development-tools) )

### Features
* **Application**
	- 2.4G
	    - 在 B85/B87 中添加2.4G demo。
		    - 2p4g_feature
			- 2p4g_genfsk_ll
			- 2p4g_tpll
			- 2p4g_tpsll

	- 2.4G & BLE 双模
	    - 在 B85/B87/TC321X 中添加2.4G & BLE 双模demo。
			- ble_slave_2_4g
	
	- 2.4G & BLE 切换
		- 在 B85/B87/TC321X中添加2.4G & BLE 切换demo。
			- ble_slave_2_4g_switch

	- 修改 Audio 设置代码结构，并修复 TC321X 的Audio设置错误。
	- 删除 Audio DMIC 设置。
	- TC321X  ble_remote demo 支持红外学习功能。
	- TC321X 新增 ADC 校准功能。

### Bug Fixes
* **Application**
    - 修复：当本地Central设备支持SMP而配对的Peripheral设备不支持SMP时，本地Central设备无法发送连接更新指示包。
	- 修复：C1T362A5开发板的GPIO设置错误。
* **Controller**
	- 修复：部分特殊连接参数导致的连接失败问题。
* **CoC**
	- 修复：建立新CoC(Connection-oriented channels)通道时的参数校验逻辑错误。
* **Driver**
	- 修复 (TC321X A0)：start_reboot函数无法正常重启MCU的问题。

### Refactoring
* **Application**
    - 支持在IDLE态下使用软件定时器。
	- 修改 TC321X 芯片中 512K Flash的地址分配。
	- 重命名部分 demo
		- 将 b85m_2p4g_feature 重命名为 2p4g_feature。
		- 将 b85m_2p4g_genfsk_ll 重命名为 2p4g_genfsk_ll。
		- 将 b85m_2p4g_tpll 重命名为 2p4g_tpll。
		- 将 b85m_2p4g_tpsll 重命名为 2p4g_tpsll。
		- 将 b85m_feature_test 重命名为 ble_feature_test。
		- 将 b85m_hci 重命名为 ble_hci。
		- 将 b85m_master_kma_dongle 重命名为 ble_master_kma_dongle。
		- 将 b85m_module 重命名为 ble_module。
		- 将 b85m_ble_remote 重命名为 ble_remote。
		- 将 b85m_ble_sample 重命名为 ble_sample。

* **Controller**
	- (TC321X) 优化连接状态下的功耗。

### BREAKING CHANGES
* **Driver**
	- 添加 rf_private_pa.c 和 rf_private_pa.h 到 B85/B87 的 driver_ext 文件夹。
	- 添加 ext_rf_private.h 到 B85/B87/TC321X 的 driver_ext 文件夹。

### CodeSize
* **B85**
	* ble_sample
	  - Firmware size: 53.1 kBytes
	  - SRAM size: 18.2 kBytes
	  - deepsleep retention SRAM size: 14.5 kBytes
	* ble_remote
	  - Firmware size: 64.9 kBytes
	  - SRAM size: 21.1 kBytes
	  - deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
	  - Firmware size: 61.8 kBytes
	  - SRAM size: 20.4 kBytes
	  - deepsleep retention SRAM size: 16.7 kBytes
	* master_kma_dongle
	  - Firmware size: 43.2 kBytes
	  - SRAM size: 20.2 kBytes
	* genfsk_ll(stx2rx)
	  - Firmware size: 16.6 kBytes
	  - SRAM size: 9.7 kBytes
	  - deepsleep retention SRAM size: 6.5 kBytes
	* tpll(ptx)
	  - Firmware size: 17.9 kBytes
	  - SRAM size: 12.5 kBytes
	  - deepsleep retention SRAM size: 7.3 kBytes
  * tpsll(stx2rx)
	  * Firmware size: 14.6 kBytes
	  * SRAM size: 9.8 kBytes
	  * deepsleep retention SRAM size: 6.3 kBytes
	* ble_slave_2_4g(tpsll_stx2rx)
	  * Firmware size: 54.0 kBytes
	  * SRAM size: 19.9 kBytes
	  * deepsleep retention SRAM size: 15.3 kBytes
	* ble_slave_2_4g_switch(tpsll_stx2rx)
	  - Firmware size: 57.7 kBytes
	  - SRAM size: 20.0 kBytes
	  - deepsleep retention SRAM size: 15.5 kBytes
  
* **B87**
  * ble_sample
    - Firmware size: 55.1 kBytes
    - SRAM size: 19.9 kBytes
    - deepsleep retention SRAM size: 16.1 kBytes
  * ble_remote
    - Firmware size: 66.1 kBytes
    - SRAM size: 22.9 kBytes
    - deepsleep retention SRAM size: 17.4 kBytes
  * ble_module
    - Firmware size: 63.9 kBytes
    - SRAM size: 22.2 kBytes
    - deepsleep retention SRAM size: 18.3 kBytes
  * master_kma_dongle
    - Firmware size: 44.8 kBytes
    - SRAM size: 21.9 kBytes
  * genfsk_ll(stx2rx)
    - Firmware size: 18.9 kBytes
    - SRAM size: 11.2 kBytes
    - deepsleep retention SRAM size: 8.0 kBytes
  * tpll(ptx)
    - Firmware size: 20.3 kBytes
    - SRAM size: 14.0 kBytes
    - deepsleep retention SRAM size: 8.9 kBytes
  * tpsll(stx2rx)
    - Firmware size: 16.9 kBytes
    - SRAM size: 11.4 kBytes
    - deepsleep retention SRAM size: 7.8 kBytes
  * ble_slave_2_4g(tpsll_stx2rx)
    - Firmware size: 56.0 kBytes
    - SRAM size: 21.4 kBytes
    - deepsleep retention SRAM size: 16.9 kBytes
  * ble_slave_2_4g_switch(tpsll_stx2rx)
    - Firmware size: 60.2 kBytes
    - SRAM size: 21.8 kBytes
    - deepsleep retention SRAM size: 17.1 kBytes

* **TC321X**
  * ble_sample
    - Firmware size: 59.2 kBytes
    - SRAM size: 19.1 kBytes
    - deepsleep retention SRAM size: 15.3 kBytes
  * ble_remote
    - Firmware size: 70.5 kBytes
    - SRAM size: 22.0 kBytes
    - deepsleep retention SRAM size: 16.4 kBytes
  * ble_module
    - Firmware size: 68.7 kBytes
    - SRAM size: 21.3 kBytes
    - deepsleep retention SRAM size: 17.3 kBytes
  * genfsk_ll(stx2rx)
    - Firmware size: 23.3 kBytes
    - SRAM size: 10.9 kBytes
    - deepsleep retention SRAM size: 7.8 kBytes
  * tpll(ptx)
    - Firmware size: 25.8 kBytes
    - SRAM size: 13.4 kBytes
    - deepsleep retention SRAM size: 8.2 kBytes
  * tpsll(stx2rx)
    - Firmware size: 21.4 kBytes
    - SRAM size: 11.1 kBytes
    - deepsleep retention SRAM size: 7.5 kBytes
  * ble_slave_2_4g(tpsll_stx2rx)
    - Firmware size: 60.1 kBytes
    - SRAM size: 20.8 kBytes
    - deepsleep retention SRAM size: 16.4 kBytes
  * ble_slave_2_4g_switch(tpsll_stx2rx)
    - Firmware size: 63.6 kBytes
    - SRAM size: 21.0 kBytes
    - deepsleep retention SRAM size: 16.5 kBytes



# V3.4.2.6 (PR)

### Version
* SDK version: tc_ble_single_sdk V3.4.2.6
* Chip Version
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0)
* Hardware Version
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform Version
  - tc_platform_sdk V3.1.0
* Toolchain Version
  - TC32 ELF GCC4.3 ( IDE: [Telink IDE](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/)/ [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/tools_and_sdk/Tools/IoTStudio/TelinkIoTStudio_V2024.8.zip) )

### Features
* TC321X remote demo support IR.
* TC321X support 2.4G demo（genfsk_ll/tpll/tpsll/ota/uart_fw_update/epd/2.4G test demo）.


### Bug Fixes
* **Application**
    - Fixed: The bit[2] of the analog register "DEEP_ANA_REG0" has been used in driver code, so modify the bit[2] to bit[5] in "b85m_ble_remote".

### Refactoring
* **Application**
    - Clean the "b85m_ble_sample" compile error when enabling the macro "BLE_OTA_SERVER_ENABLE".
	- Clean the "b85m_master_kma_dongle" compile error when disabling the macro "BLE_HOST_SMP_ENABLE".
	- Add the flash protection initialization error flag.

### BREAKING CHANGES
* N/A.

### Flash
* **B85**
  - ZB25WD40B
  - ZB25WD80B
  - GD25LD40C/GD25LD40E
  - P25Q40SU
  - GD25LD80C/GD25LD80E

* **B87**
  - GD25LD40C/GD25LD40E
  - GD25LD80C/GD25LD80E
  - ZB25WD40B
  - ZB25WD80B
  - P25Q80U

* **TC321X**
  - P25D40SU
  - P25Q80U
  - GD25LD40EGEG
  - P25Q16SU (Not Test)

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.3 kBytes
		- SRAM size: 18.0 kBytes
		- deepsleep retention SRAM size: 14.4 kBytes
	* ble_remote
		- Firmware size: 65.0 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 61.9 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.6 kBytes
	* master_kma_dongle
		- Firmware size: 42.9 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.6 kBytes
		- SRAM size: 18.3 kBytes
		- deepsleep retention SRAM size: 14.6 kBytes
	* ble_remote
		- Firmware size: 64.5 kBytes
		- SRAM size: 21.4 kBytes
		- deepsleep retention SRAM size: 15.9 kBytes
	* ble_module
		- Firmware size: 62.3 kBytes
		- SRAM size: 20.7 kBytes
		- deepsleep retention SRAM size: 16.8 kBytes
	* master_kma_dongle
		- Firmware size: 43.1 kBytes
		- SRAM size: 20.4 kBytes

* **TC321X**
	* ble_sample
		- Firmware size: 50.0 kBytes
		- SRAM size: 17.8 kBytes
		- deepsleep retention SRAM size: 14.1 kBytes
	* ble_remote
		- Firmware size: 64.8 kBytes
		- SRAM size: 20.5 kBytes
		- deepsleep retention SRAM size: 15.0 kBytes
	* ble_module
		- Firmware size: 63.4 kBytes
		- SRAM size: 20.0 kBytes
		- deepsleep retention SRAM size: 16.1 kBytes
	* genfsk_ll(stx2rx)
		- Firmware size: 15.2 kBytes
		- SRAM size: 9.9 kBytes
		- deepsleep retention SRAM size: 6.7 kBytes
	* tpll(ptx)
		- Firmware size: 20.3 kBytes
		- SRAM size: 11.8 kBytes
		- deepsleep retention SRAM size: 6.5 kBytes
    * tpsll(stx2rx)
		- Firmware size: 12.6 kBytes
		- SRAM size: 9.7 kBytes
		- deepsleep retention SRAM size: 6.1 kBytes

### Version
* SDK 版本: tc_ble_single_sdk V3.4.2.6
* Chip 版本
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0)
* Hardware 版本
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform 版本
  - tc_platform_sdk V3.1.0
* Toolchain 版本
  - TC32 ELF GCC4.3 ( IDE: [Telink IDE](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/)/ [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/tools_and_sdk/Tools/IoTStudio/TelinkIoTStudio_V2024.8.zip) )

### Features
* TC321X的remote demo支持IR功能。
* TC321X支持2.4G demo（genfsk_ll/tpll/tpsll/ota/uart_fw_update/epd/2.4G test demo）。

### Bug Fixes
* **Application**
  - 修复：模拟寄存器“DEEP_ANA_REG0”的bit[2]已在driver程序中被使用，所以将“b85m_ble_remote”中的bit[2]修改为bit[5]。 

### Refactoring
* **Application**
  - 清理“b85m_ble_sample”中，在使能宏“BLE_OTA_SERVER_ENABLE”时的编译错误。
  - 清理“b85m_master_kma_dongle”中，在使能宏“BLE_HOST_SMP_ENABLE”时的编译错误。
  - 添加flash保护的错误flag。

### BREAKING CHANGES
* N/A.

### Flash
* **B85**
  - ZB25WD40B
  - ZB25WD80B
  - GD25LD40C/GD25LD40E
  - P25Q40SU
  - GD25LD80C/GD25LD80E

* **B87**
  - GD25LD40C/GD25LD40E
  - GD25LD80C/GD25LD80E
  - ZB25WD40B
  - ZB25WD80B
  - P25Q80U

* **TC321X**
  - P25D40SU
  - P25Q80U
  - GD25LD40EGEG
  - P25Q16SU (未测试)

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.3 kBytes
		- SRAM size: 18.0 kBytes
		- deepsleep retention SRAM size: 14.4 kBytes
	* ble_remote
		- Firmware size: 65.0 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 61.9 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.6 kBytes
	* master_kma_dongle
		- Firmware size: 42.9 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.6 kBytes
		- SRAM size: 18.3 kBytes
		- deepsleep retention SRAM size: 14.6 kBytes
	* ble_remote
		- Firmware size: 64.5 kBytes
		- SRAM size: 21.4 kBytes
		- deepsleep retention SRAM size: 15.9 kBytes
	* ble_module
		- Firmware size: 62.3 kBytes
		- SRAM size: 20.7 kBytes
		- deepsleep retention SRAM size: 16.8 kBytes
	* master_kma_dongle
		- Firmware size: 43.1 kBytes
		- SRAM size: 20.4 kBytes

* **TC321X**
	* ble_sample
		- Firmware size: 50.0 kBytes
		- SRAM size: 17.8 kBytes
		- deepsleep retention SRAM size: 14.1 kBytes
	* ble_remote
		- Firmware size: 64.8 kBytes
		- SRAM size: 20.5 kBytes
		- deepsleep retention SRAM size: 15.0 kBytes
	* ble_module
		- Firmware size: 63.4 kBytes
		- SRAM size: 20.0 kBytes
		- deepsleep retention SRAM size: 16.1 kBytes
	* genfsk_ll(stx2rx)
		- Firmware size: 15.2 kBytes
		- SRAM size: 9.9 kBytes
		- deepsleep retention SRAM size: 6.7 kBytes
	* tpll(ptx)
		- Firmware size: 20.3 kBytes
		- SRAM size: 11.8 kBytes
		- deepsleep retention SRAM size: 6.5 kBytes
    * tpsll(stx2rx)
		- Firmware size: 12.6 kBytes
		- SRAM size: 9.7 kBytes
		- deepsleep retention SRAM size: 6.1 kBytes



# V3.4.2.5 (ER)

### Version
* SDK version: tc_ble_single_sdk V3.4.2.5
* Chip Version
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0)
* Hardware Version
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform Version
  - tc_platform_sdk V3.0.0
* Toolchain Version
  - TC32 ELF GCC4.3 ( IDE: [Telink IDE](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/)/ [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/tools_and_sdk/Tools/IoTStudio/TelinkIoTStudio_V2024.8.zip) )

### Note
* All changes up to telink_b85m_ble_single_connection_sdk V3.4.2.4_Patch0003 are included.

### Features
* Support TC321X A0
  - Support BLE 5.3 feature, only support 1M phy and 2M phy.
  - Support PM, including suspend mode, deepsleep retention mode and deepsleep mode.
  - Support Flash Protection.
  - Support Battery Voltage Check.
  - Demo illustration:
	- b85m_ble_sample
	- b85m_ble_remote
	- b85m_feature_test
	- b85m_hci
	- b85m_module

### Bug Fixes
* N/A.

### Refactoring
* remove unavailable API "blt_crypto_alg_csip_s1".

### BREAKING CHANGES
* Rename drivers/8258 to drivers/B85.
* Rename drivers/8278 to drivers/B87.
* Modify SDK name to "tc_ble_single_sdk".
* Change the structure of the project. Split it into three independent projects.

### Flash
* **8258**
  - ZB25WD40B
  - ZB25WD80B
  - GD25LD40C/GD25LD40E
  - P25Q40SU
  - GD25LD80C/GD25LD80E

* **8278**
  - GD25LD40C/GD25LD40E
  - GD25LD80C/GD25LD80E
  - ZB25WD40B
  - ZB25WD80B
  - P25Q80U

* **TC321X**
  - P25D40SU
  - P25Q80U
  - GD25LD80E (Not Test)
  - P25Q16SU (Not Test)

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.1 kBytes
		- SRAM size: 17.8 kBytes
		- deepsleep retention SRAM size: 14.3 kBytes
	* ble_remote
		- Firmware size: 65.0 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.6 kBytes
	* ble_module
		- Firmware size: 61.9 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.5 kBytes
	* master_kma_dongle
		- Firmware size: 42.9 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.4 kBytes
		- SRAM size: 18.0 kBytes
		- deepsleep retention SRAM size: 14.4 kBytes
	* ble_remote
		- Firmware size: 64.2 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 62.0 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.7 kBytes
	* master_kma_dongle
		- Firmware size: 42.8 kBytes
		- SRAM size: 20.2 kBytes

* **TC321X**
	* ble_sample
		- Firmware size: 49.7 kBytes
		- SRAM size: 17.5 kBytes
		- deepsleep retention SRAM size: 13.9 kBytes
	* ble_remote
		- Firmware size: 64.8 kBytes
		- SRAM size: 20.5 kBytes
		- deepsleep retention SRAM size: 15.0 kBytes
	* ble_module
		- Firmware size: 51.7 kBytes
		- SRAM size: 18.9 kBytes
		- deepsleep retention SRAM size: 14.9 kBytes


### Version
* SDK 版本: tc_ble_single_sdk V3.4.2.5
* Chip 版本
  - TLSR825X (B85)
  - TLSR827X (B87)
  - TC321X   (A0)
* Hardware 版本
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
* Platform 版本
  - tc_platform_sdk V3.0.0
* Toolchain 版本
  - TC32 ELF GCC4.3 ( IDE: [Telink IDE](https://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/)/ [TelinkIoTStudio_V2024.8](https://wiki.telink-semi.cn/tools_and_sdk/Tools/IoTStudio/TelinkIoTStudio_V2024.8.zip) )

### Note
* telink_b85m_ble_single_connection_sdk V3.4.2.4_Patch0003之前的所有修改均已包含。

### Features
* 支持TC321X A0；
  - 支持BLE 5.3特性，只支持1M phy和2M phy。
  - 支持PM，包括suspend mode，deepsleep retention mode和deepsleep mode.
  - 支持Flash保护。
  - 支持低电检测。
  - Demo说明：
	- b85m_ble_sample
	- b85m_ble_remote
	- b85m_feature_test
	- b85m_hci
	- b85m_module

### Bug Fixes
* N/A.

### Refactoring
* 移除API “blt_crypto_alg_csip_s1”。

### BREAKING CHANGES
* 将drivers/8258重命名为drivers/B85。
* 将drivers/8278重命名为drivers/B87。
* 修改SDK名字为“tc_ble_single_sdk”。
* 修改工程结构，将其分成三个独立的工程。

### Flash
* **8258**
  - ZB25WD40B
  - ZB25WD80B
  - GD25LD40C/GD25LD40E
  - P25Q40SU
  - GD25LD80C/GD25LD80E

* **8278**
  - GD25LD40C/GD25LD40E
  - GD25LD80C/GD25LD80E
  - ZB25WD40B
  - ZB25WD80B
  - P25Q80U

* **TC321X**
  - P25D40SU
  - P25Q80U
  - GD25LD80E (未测试)
  - P25Q16SU (未测试)

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.1 kBytes
		- SRAM size: 17.8 kBytes
		- deepsleep retention SRAM size: 14.3 kBytes
	* ble_remote
		- Firmware size: 65.0 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.6 kBytes
	* ble_module
		- Firmware size: 61.9 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.5 kBytes
	* master_kma_dongle
		- Firmware size: 42.9 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.4 kBytes
		- SRAM size: 18.0 kBytes
		- deepsleep retention SRAM size: 14.4 kBytes
	* ble_remote
		- Firmware size: 64.2 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 62.0 kBytes
		- SRAM size: 20.4 kBytes
		- deepsleep retention SRAM size: 16.7 kBytes
	* master_kma_dongle
		- Firmware size: 42.8 kBytes
		- SRAM size: 20.2 kBytes

* **TC321X**
	* ble_sample
		- Firmware size: 49.7 kBytes
		- SRAM size: 17.5 kBytes
		- deepsleep retention SRAM size: 13.9 kBytes
	* ble_remote
		- Firmware size: 64.8 kBytes
		- SRAM size: 20.5 kBytes
		- deepsleep retention SRAM size: 15.0 kBytes
	* ble_module
		- Firmware size: 51.7 kBytes
		- SRAM size: 18.9 kBytes
		- deepsleep retention SRAM size: 14.9 kBytes



## V3.4.2.4

### Version
* SDK version:telink_b85m_ble_single_connection_sdk_V3.4.2.4
* Driver Version: telink_b85m_driver_sdk V1.7.0

### Hardware
* N/A

### Note
* Copyright update causes all files to change.

* Battery Voltage Check is a very important function for mass production. The user must check battery voltage to prevent abnormal writing or erasing Flash at a low voltage.
	
* Flash protection is a critical function for mass production. 
	- Flash protection is enabled by default in SDK. User must enable this function on their final mass-production application.
	- Users should use "Unlock" command in the Telink BDT tool for Flash access during the development and debugging phase.
	- Flash protection demonstration in SDK is a reference design based on sample code. Considering that the user's final application may be different from the sample code, for example, the user's final firmware size is bigger, or the user has a different OTA design, or the user needs to store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not be directly used on user's mass production application without any change. User should refer to sample code, understand the principles and methods, and then change and implement a more appropriate mechanism according to their application if needed.

* Correct some spelling mistakes
	- For example, change blc_pm_modefy_brx_early_set to blc_pm_modify_brx_early_set.

* Some useless, redundant code was removed
	- For example, delete structure smp_m_param_save_t.


### Bug Fixes
* **software timer**
	- Fixed: Reorder the software timer under the condition that at least one timer exists after deleting one in the function "blt_soft_timer_delete".

* **Application**
	- Fixed: Whitelist sample code process peer Central device special case: use IDA(identity Address) in first pairing phase, but use RPA(Resolvable Private Address) in later reconnect phase.
	- Fixed: Direct ADV may not be connected when the Central(Master) device updates its RPA.
		- In the previous SDK, direct ADV may not be connected automatically when the Central(Master) device refreshes its RPA in "CONNECT_IND" PDU.
		- In the new SDK, if peer IRK is valid, add peer IRK to the resolving list and enable address resolution. direct ADV can be connected even Central(Master) device refreshes RPA.

* **PHY**
	- Fixed: In the BQB RX test, there is a small probability of failure when switching to S2/S8 when the payload is 255Bytes.

* **ATT**
	- Fixed: If the last entry in the ATT table of the server is the Primary Service and there is no characteristic, an incorrect handle will be returned during service discovery.

### BREAKING CHANGES 
* **Boot & Link**	
	- Optimized and simplified the implementation of cstartup.S and boot.link.
		- Add cstartup_825x.S. This file is enough for all B85 projects. The SRAM size is automatically set by judging MCU_STARTUP. Previous 825x cstartup files are deleted as below:
			- cstartup_8251_RET_16K.S/cstartup_8251_RET_32K.S/cstartup_8253_RET_16K.S/cstartup_8253_RET_32K.S/cstartup_8258_RET_16K.S/cstartup_8258_RET_32K.S

		- Add cstartup_827x.S. This file is enough for all B87 projects. The SRAM size is automatically set by judging MCU_STARTUP. Previous 825x cstartup files are deleted as below:
			- cstartup_8271_RET_16K.S/cstartup_8271_RET_32K.S/cstartup_8278_RET_16K.S/cstartup_8278_RET_32K.S/cstartup_8238_RET_16K.S/cstartup_8238_RET_32K.S

		- Add boot.link. This file is enough for all B85/B87 projects. Previous link files are deleted as below:
			- boot_16k_retn_8238.link/boot_32k_retn_8238.link
			- boot_16k_retn_8251_8253_8258.link/boot_32k_retn_8251.link/boot_32k_retn_8253_8258.link
			- boot_16k_retn_8271_8278.link/boot_32k_retn_871.link/boot_32k_retn_8278.link
			
        - Configure "SRAM_SIZE" for different ICs in the project setting.
			- B85
				- Use new macro MCU_STARTUP_8251/MCU_STARTUP_8253/MCU_STARTUP_8258 to configure IC, and MCU_STARTUP_8258 is used by default.
			    - Old macro below is not used anymore: 
				  MCU_STARTUP_8258_RET_16K, MCU_STARTUP_8258_RET_32K, 
				  MCU_STARTUP_8253_RET_16K, MCU_STARTUP_8253_RET_32K, 
				  MCU_STARTUP_8251_RET_16K, MCU_STARTUP_8251_RET_32K				
			- B87
				- Use new macro MCU_STARTUP_8271/MCU_STARTUP_8238/MCU_STARTUP_8278 to configure IC, and MCU_STARTUP_8278 is used by default.
			    - Old macro below is not used anymore: 
				  MCU_STARTUP_8278_RET_16K, MCU_STARTUP_8278_RET_32K, 
				  MCU_STARTUP_8238_RET_16K, MCU_STARTUP_8238_RET_32K, 
				  MCU_STARTUP_8271_RET_16K, MCU_STARTUP_8271_RET_32K

		- Users do not need to be concerned about how to change configurations of different deepsleep retention SRAM size(16K or 32K). It's now processed automatically.
			- "blc_app_setDeepsleepRetentionSramSize" is used to configure deepsleep retention SRAM size automatically.
			- "\_retention_size_" is defined in cstarup.S file, and it is used to automatically calculate the total size of retention SRAM.
			If "_retention_size" exceeds the maximum value(32K bytes), the compiler will report the error "Error: Retention RAM size overflow." for users to notice this.
			- Add user_config.c in vendor/common, and add "__PM_DEEPSLEEP_RETENTION_ENABLE" in this file for retention SRAM size auto calculating and alarming in boot.link.
			"__PM_DEEPSLEEP_RETENTION_ENABLE" equals to "PM_DEEPSLEEP_RETENTION_ENABLE" which is configurated by user in app_config.h.


* **Application**
	- BLE Flash information and Calibration
		- Rename blt_common.h/blt_common.c to ble_flash.h/ble_flash.c.
		- Split function "blc_app_loadCustomizedParameters" into two functions "blc_app_loadCustomizedParameters_normal"	and "blc_app_loadCustomizedParameters_deepRetn".
		The first one is used when the MCU is powered on or wakes up from deepsleep mode. The second one is used when MCU wakes up from deepsleep retention mode.
		- Add API "blc_flash_read_mid_get_vendor_set_capacity" in "blc_readFlashSize_autoConfigCustomFlashSector" to get Flash mid, vendor and capacity information.

* **ATT**
	- Some old ATT APIs are not recommended to use anymore, there are better APIs replaced for each one of them. Hide the old API declaration in the stack. Keep only some external declarations in ble_comp.h in case some users persistently use these APIs.

		- Hide API "bls_att_pushNotifyData", user should use API "blc_gatt_pushHandleValueNotify".
		- Hide API "bls_att_pushIndicateData", user should use API "blc_gatt_pushHandleValueIndicate".
		- Hide API "att_req_find_info", user should use API "blc_gatt_pushFindInformationRequest".
		- Hide API "att_req_find_by_type", user should use API "blc_gatt_pushFindByTypeValueRequest".
		- Hide API "att_req_read_by_type", user should use API "blc_gatt_pushReadByTypeRequest".
		- Hide API "att_req_read", user should use API "blc_gatt_pushReadRequest".
		- Hide API "att_req_read_blob", user should use API "blc_gatt_pushReadBlobRequest".
		- Hide API "att_req_read_by_group_type", user should use API "blc_gatt_pushReadByGroupTypeRequest".
		- Hide API "att_req_write", user should use API "blc_gatt_pushWriteRequest".
		- Hide API "att_req_write_cmd", user should use API "blc_gatt_pushWriteCommand".
	- Remove API "blt_att_resetRxMtuSize".
	- Change "blt_att_setEffectiveMtuSize" to "blc_att_setEffectiveMtuSize".
	- Change "blt_att_resetEffectiveMtuSize" to "blc_att_resetEffectiveMtuSize".

* **L2CAP**

	- Hide API "blc_l2cap_pushData_2_controller". It is an internal API in the stack, and users should not use it.
	When it appeared on the previous SDK, it was used to send a GATT confirm command. Now use the new API "blc_gatt_pushConfirm" to replace it.
	- Hide macro "L2CAP_RX_BUFF_LEN_MAX". This is for internal use in the stack and should not be used by users.
	- Change ATT_RX_MTU_SIZE_MAX to ATT_MTU_MAX_SDK_DFT_BUF.
	- API "blc_l2cap_initMtuBuffer" increases the error return value.

* **SMP**
	- Delete API "blc_smp_setPeerAddrResSupportFlg". It is a special API for local RPA usage on previous SDK with a special library provided to customers if they need it. 
	Now new local RPA feature is included in the general library, so this special API is not needed. Users can refer to feature privacy for new designs.
	- Delete smp_alg.h, change it to algorithm\crypto\crypto_alg.h, and rename the APIs in it.

* **Controller**
	- Hide stack internal APIs, and users should not use them.
		- blc_ll_readMaxAdvDataLength
		- blc_ll_setInitTxDataLength
		- blc_ll_GetBrxNextTick
		- blc_ll_get_macAddrRandom
		- blc_ll_get_macAddrPublic
		- blc_ll_readMaxAdvDataLength
	- Remove API “blc_ll_setAclConnMaxOctetsNumber”.
	- Hide stack internal data structure：
		- ll_data_extension_t
		- rf_pkt_pri_adv_t
		- ll_ext_adv_t
		- Some structures in ble_format.h

* **HCI**
	- Delete the data structure definition "event_connection_update_t", and users can use "hci_le_connectionUpdateCompleteEvt_t" to replace it.
	- Hide API "blc_hci_le_getLocalSupportedFeatures". It is a stack internal API, and users should not use it.

* **OTA** 
	- Hide API "bls_ota_clearNewFwDataArea" and use API "blc_ota_initOtaServer_module" instead.
	- Change "bls_ota_set_fwSize_and_fwBootAddr" to "blc_ota_setFirmwareSizeAndBootAddress"

* **Others** 
	- Fix spelling mistakes.
	- Remove ext_pm.h and ext_rf.h, and the API open to users has been moved to ext_misc.h
	- Remove application\usbstd\usbhw.c, application\usbstd\usbhw.h and application\usbstd\usbhw_i.h, and use usbhw.h and usbhw.c in the driver instead.
	- Remove the ADC calibration data structure "adc_vref_ctr_t" in adc.h/adc.c. The original ADC calibration data at the application layer is stored in the adc_gpio_calib_vref, adc_gpio_calib_vref_offset and adc_vbat_calib_vref. Refer to telink_b85m_driver_sdk_Release_Note.

### Features
* **Application**
	- Add Flash Protection module and sample 
		- Add flash_prot.h/flash_prot.c in vendor/common folder as a common module for all applications.
		- Add Flash protection sample code in project B85m_ble_sample/B85m_ble_remote/B85m_ble_module/B85m_master_kma_dongle. User can refer to the sample code and make sure the Flash protection function	must be enabled on their own application.
		- "app_flash_protection_operation" is a common entrance for all Flash operations (Flash write and erase), Flash locking and unlocking are processed in this function.
		- "blc_appRegisterStackFlashOperationCallback" and "flash_prot_op_cb" are used to process some stack internal Flash operation callback which may need Flash locking and unlocking.
		- The Telink OTA and SMP module already add callbacks related to blc_flashProt to inform the application layer that they need to write/erase Flash. Then, the application layer processes Flash lock and unlock.
		- If users do not use Telink OTAs, they need to refer to this design to add similar callback functions in their own OTA code for upper-layer use.
		- Now only firmware is protected in the sample code. System data and user data are not protected now.
		- IRQ disabling time too long in driver API "flash_write_status", BLE system interrupt is delayed, causing RX data error. 
		Driver API "flash_write_status" be replaced by "flash_write_status" in flash_prot.c to solve this issue by calling LinkLayer API "blc_ll_write_flash_status".

	- Add tlkapi_debug module to print logs
		- Add tlkapi_debug.h/tlkapi_debug.c in vendor/common folder as a common module for all applications.
		- Add debug.h in stack/ble/debug. Add APIS "blc_debug_enableStackLog", "blc_debug_addStackLog", "blc_debug_removeStackLog" to control the print stack log.
		- Use GPIO simulating UART TX sequence to output data information to PC UART tool, same function with "printf" function in old SDK.
		- API "tlkapi_printf" and "tlkapi_send_string_data" are recommended for usage, with some macro in app_config.h to control log enabling such as "APP_LOG_EN".
		- Only 1M baud rate can be used. When the system clock is 16M/24M, the assembly is used to control the UART TX timing. When the system clock is 32M/48M, the system_timer is used to control the UART TX timing. IRQ disabling and restoring are used in function "uart_putc" to guarantee UART TX byte data integrity is not broken by IRQ. 
		If the baud rate is too low, IRQ disabling time too long, BLE system interrupt and user interrupt will be affected which may lead to critical error.

	- Add stack initialization error check for all applications.
		- Add API "blc_contr_checkControllerInitialization" to check if any controller initialization fails.
		- Add API "blc_host_checkHostInitialization" to check if any host initialization fails.	
		- Add enumeration definition "init_err_t" in ble_common.h to distinguish error types.
		- Users should add some UI alarm action to know whether an initialization error happens, and then check the reason according to the definition of enumeration "init_err_t".

* **feature_test**
	- Add feature_privacy_slave: Sample code for testing peripheral RPA. This sample code to get peer Central(Master) IDA(identity address) if RPA(resolved private address) is used in packet "CONNECT_IND".

	- Add feature_privacy_master: Sample code for testing Central RPA.

	- Add feature_soft_uart: Sample code for TX and RX function of soft_uart.

	- Add feature_usb_cdc: Sample code for USB CDC of Central.

	- Add Sample code for SC OOB in feature_smp_security.
	
	- Add feature_COC_slave: Sample code for testing peripheral L2CAP COC.

	- Add feature_multi_local_dev: Sample code for Multi Local Device.

	- Add feature_md_master: Sample code for More Data of Central.

	- Add feature_md_slave: Sample code for More Data of Peripheral.

* **Controller**
	- Add a new Telink-defined Controller Event "BLT_EV_FLAG_ADV_TX_EACH_CHANNEL", which is triggered before transmitting ADV PDU for all ADV channels (37/38/39)

	- Add a new Telink-defined Controller event "BLT_EV_FLAG_SCAN_REQ", which is triggered after a Scan Request PDU is received.

	- Add API "blc_contr_setBluetoothVersion" to set the controller core version in "LL_VERSION_IND" PDU, and the default version is 5.3.

	- Add timestamp(system timer tick) in Advertising report event for legacy Scanning.
		- Attention that this is a customized function, not a standard BLE function.
		- Use API "blc_ll_advReport_setRxPacketTickEnable" to enable this feature.
		- Check Advertising report data with the new data structure "hci_tlk_advReportWithRxTickEvt_t".
	- Add macro "GET_RXADD_FROM_CONNECT_EVT_DATA" to get RX address type from the callback data of controller event "BLT_EV_FLAG_CONNECT".

	- Add API "blc_ll_scanReq_filter_en", which is used to set whether to filter the scan device.

	- Add API “bls_ll_setScanChannelMap”, which is used to set the channel map of scanning.

	- Add API “blc_ll_get_connEffectiveMaxTxOctets” and "blc_ll_get_connEffectiveMaxRxOctets", which are uesd to get maximum of effective Tx octets and Rx octets in connection.

	- Add API “blc_ll_setAdvIntervalCheckEnable”, which is used to set whether to check the ADV interval setting.

	- Add API “bls_pm_getNexteventWakeupTick”, which is used to get the next wake-up tick。

* **HCI**
	- Add API “blc_hci_le_readBufferSize_cmd”, which is used to read buffer size.	

* **GATT**
	- Add APIs:
		- blc_gatt_pushMultiHandleValueNotify is used to notify a client of the multiple values with different handles of a Characteristic Value from a server.
		- blc_gatt_pushPrepareWriteRequest is used to send prepare write request to server.
		- blc_gatt_pushExecuteWriteRequest is used to send execute/cancel write request to server.
		- blc_gatt_pushConfirm is used to send confirm.
		- blc_gatt_pushReadMultiRequest is used to send read multiple values request to server and value length is known fixed.
		- blc_gatt_pushReadMultiVariableRequest is used to send read multiple values request to server and value length is variable or unknown.
		- blc_gatt_pushErrResponse is used to send ATT Error Response.

* **ATT**
	- Add API "blc_att_holdAttributeResponsePayloadDuringPairingPhase", which is used to set whether blocking ATT Response PDU in the pairing stage.
	- Add API "blc_att_enableReadReqReject", which is used to set whether the automatically sends the Error Response based on the returned value by the read callback.
	- Add API "blc_att_enableWriteReqReject", which is used to set whether the automatically sends the Error Response based on the returned value by the write callback.

* **L2CAP**	
	- Add Peripheral L2CAP COC feature.
		- Add stack/ble/host/signaling/folder, and add signaling.h in this folder.
		- Add some APIs for this feature.
			- blc_l2cap_registerCocModule is used to register the COC Module.
			- blc_l2cap_disconnectCocChannel is used to disconnect with the COC Channel.
			- blc_l2cap_createLeCreditBasedConnect is used to create LE Credit Based connection.
			- blc_l2cap_createCreditBasedConnect is used to create Credit Based connection.
			- blc_l2cap_sendCocData is used to send COC Data.
		- Add some host events in gap_event.h 
			- GAP_EVT_L2CAP_COC_CONNECT is triggered by the COC connection.
			- GAP_EVT_L2CAP_COC_DISCONNECT is triggered by the COC disconnection.
			- GAP_EVT_L2CAP_COC_RECONFIGURE is triggered by the COC reconfigure.
			- GAP_EVT_L2CAP_COC_RECV_DATA is triggered when receiving COC Data.
			- GAP_EVT_L2CAP_COC_SEND_DATA_FINISH is triggered after COC Data is sent.
			- GAP_EVT_L2CAP_COC_CREATE_CONNECT_FINISH is triggered after the create COC channel command is sent.

* **SMP**
	- Add Peripheral SC OOB.
		- Add some APIs for this feature.
			- blc_smp_generateScOobData is used to generate SC OOB data.
			- blc_smp_setScOobData is used to set SC OOB data.
		- Add some host events in gap_event.h.
			- GAP_EVT_MASK_SMP_TK_SEND_SC_OOB_DATA is triggered when the peer requests SC OOB.
			- GAP_EVT_MASK_SMP_TK_REQUEST_SC_OOB is triggered when SMP requests SC OOB data.

	- Add data structure definition "smp_param_master_t" corresponding to Central(Master) SMP pairing information Flash storage.   
		- peer_id_adrType/peer_id_addr/local_irk/peer_irk can be used for whitelisting and resolving lists.

	- Add API "blm_smp_getPairedDeviceNumber" only for the Central(Master) role to get the current paired device number.

	- Add API "blm_smp_loadParametersByIndex" only for the Central(Master) role to read pairing parameters by index.

	- Add API "blc_smp_manualSetPinCode_for_debug” to Passkey Entry set pinCode manually in the Passkey Entry.
  
	- Add API "blc_smp_enableSecureConnections” to enable Secure Connection.

* **OTA** 
	- Add one OTA error type "OTA_MCU_NOT_SUPPORTED", B85 does not support the OTA PDU which is bigger than 80 due to some special reason.
	- Add API "blc_ota_getCurrentUsedMultipleBootAddress" to read the current used multiple boot addresses.

* **Privacy**	
	- Support privacy local RPA for both BLE Peripheral(Slave) and BLE Central(Master)
		- Add API "blc_ll_initPrivacyLocalRpa" to enable the local RPA feature.
		- Add API "blc_smp_generateLocalIrk" to generate local IRK if needed.
		- Add API "blc_app_isIrkValid" to check if IRK(local IRK or peer IRK) stored in Flash is valid. 


* **Others**
	- Add Peripheral Multi Local Device function
		- It's a customized feature, not BLE standard function. One device can be virtually extended to up to 4 devices with different MAC addresses (public or static random).
		These devices can be switched dynamically, with only one device at any time, but can not be concurrent.
		- Add stack/ble/device folder, and add multi_device.h in this folder.
		- Add LinkLayer API "blc_ll_setMultipleLocalDeviceEnable"/"blc_ll_setLocalDeviceIndexAndIdentityAddress"/"blc_ll_setCurrentLocalDevice_by_index" for this feature
		- Add SMP API "blc_smp_multi_device_param_getCurrentBondingDeviceNumber"/"bls_smp_multi_device_param_loadByIndex" to read specific device pairing information.
		- Add feature_multi_local_dev sample code in vendor/B85m_feature_test to demonstrate how to use this feature.

	- Add USB CDC function
    	- Add API “usb_cdc_tx_data_to_host" to send data to the USB host.
    	- Add API “usb_cdc_rx_data_to_host" to receive data from the USB host.
    	- Add API “usb_cdc_irq_data_process” to handle USB cdc data in IRQ.

	- Google Audio has been upgraded from 0.4e to 1.0 and supports audio DLE.

	- Add version information function.
		- Add API "blc_get_sdk_version" to get the SDK version.
		- Add the SDK version at the end of the bin file.
		- Add SDK version at the post-build.

	- Add mcu_config.h, including features supported by the MCU.

	- Add ext_calibration.c and ext_calibration.h to support ADC calibration, including flash two-point GPIO calibration, flash single-point GPIO calibration, flash single-point VBAT calibration (supported only by B87) and efuse single-point calibration; Supports flash voltage calibration.

	- Add software_uart.h and software_uart.c

### Refactoring
* **Application**
	- add app_common.h/app_common.c to process some common initialization or settings for all application projects.
		- Move SYS_CLK_TYPE/CLOCK_SYS_CLOCK_1S/CLOCK_SYS_CLOCK_1MS/CLOCK_SYS_CLOCK_1US definition from app_config.h to app_common.h. 
		Users now only define CLOCK_SYS_CLOCK_HZ in app_config.h. 
		- add API "blc_app_setDeepsleepRetentionSramSize" in app_common to automatically set deepsleep retention SRAM size for all application projects.
		- add API "blc_app_checkControllerHostInitialization" in app_common to check stack initialization error for the application project.
		- add API "blc_app_isIrkValid" in app_common to check if local or peer IRK is valid, which can be used in the whitelist and resolving list of relevant applications.

	- Add app_buffer.h/app_buffer.C to configure the default FIFO size for all applications.

	- Battery Check is moved to common, and the application only handles the relevant UI parts.

	- Add simple_sdp.h/simple_sdp.c to handle the common simple SDP of the central role.

	- Add custom_pair.h/custom_pair.c to handle the common custom pair of the central role.

	- use "rf_drv_ble_init" to replace "rf_drv_init(RF_MODE_BLE_1M)" in main.c.
		- If some users need to use other RF modes, "rf_drv_init" can now be used to switch RF mode by changing the parameter "rf_mode".

	- Development board configuration
		- Delete redundant configuration information for LED and Keyboard in app_config.h of each project. Users can select the development board by BOARD_SELECT.
		- Add the folder vendor/common/boards and the following files that define each development board configuration:
			- boards_config.h, C1T139A3.h, C1T139A30.h, C1T139A5.h, C1T197A30.h, C1T197A5.h, C1T201A3.h

* **BLE General**
	- Add stack/ble/ble_comp.h for some API/macro/data structure names in the old SDK compatible with the new SDK.
	- Add stack initialization error check for all applications.

* **Feature Test**
	- Rename feature_master_dle to feature_DLE_master.
	- Rename feature_slave_dle to feature_DLE_slave.
	- Remove feature_phy_extend_adv, code merge into feature_extend_adv.
	
* **Controller**
	- Add stack/ble/ble_controller/controller.h. Put some controller-relevant enumeration and API declaration in this file.
	- Add callback data structure for some Telink-defined controller events.
		- "tlk_contr_evt_connect_t" is for "BLT_EV_FLAG_CONNECT".
		- "tlk_contr_evt_terminate_t" is for "BLT_EV_FLAG_TERMINATE".
		- "tlk_contr_evt_dataLenExg_t" is for "BLT_EV_FLAG_DATA_LENGTH_EXCHANGE".
		- "tlk_contr_evt_chnMapRequest_t" is for "BLT_EV_FLAG_CHN_MAP_REQ".
		- "tlk_contr_evt_chnMapUpdate_t" is for "BLT_EV_FLAG_CHN_MAP_UPDATE".
		- "tlk_contr_evt_connParaReq_t" is for "BLT_EV_FLAG_CONN_PARA_REQ".
		- "tlk_contr_evt_connParaUpdate_t" is for "BLT_EV_FLAG_CONN_PARA_UPDATE".
		- "tlk_contr_evt_versionIndRev_t" is for "BLT_EV_FLAG_VERSION_IND_REV".
		- "tlk_contr_evt_scanReq_t" is for "BLT_EV_FLAG_SCAN_REQ".
  
* **SMP**
	- Add return value indicating initialization error for API "blc_smp_param_setBondingDeviceMaxNumber".
	- Adjust ACL central(Master) SMP parameters.
		- Delete unused data structure "smp_m_param_save_t", it's an incorrect structure in the old SDK.
		- Delete unused data structure "mac_adr_t".
		- Add data structure "smp_param_master_t" for user reference when reading SMP storage parameters.

* **Algorithm**
	- Adjust the file structure of the algorithm.

* **Others**
	- Delete MULTI_BOOT_ADDR_0x80000 from drivers/8258/driver_ext/mcu_boot.h, B85 does not support boot from 0x80000.
	- Move rf_pa.c and rf_pa.h into driver_ext.
	- Move compiler.h into common.

### Performance Improvements

* **Application**
	- Move “blc_readFlashSize_autoConfigCustomFlashSector” and “blc_app_loadCustomizedParameters_normal” into “user_init_normal” to improve code efficiency.


### Known issues
  - Firmware size increase about 5kBytes for "ble_sample"
	- Print log: About 1.5kBytes
	- Flash protection: About 1kBytes
	- RPA: About 2.5kBytes

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.1 kBytes
		- SRAM size: 17.7 kBytes
		- deepsleep retention SRAM size: 14.2 kBytes
	* ble_remote
		- Firmware size: 64.6 kBytes
		- SRAM size: 20.9 kBytes
		- deepsleep retention SRAM size: 15.5 kBytes
	* ble_module
		- Firmware size: 61.1 kBytes
		- SRAM size: 20.1 kBytes
		- deepsleep retention SRAM size: 16.4 kBytes
	* master_kma_dongle
		- Firmware size: 42.8 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.3 kBytes
		- SRAM size: 17.9 kBytes
		- deepsleep retention SRAM size: 14.3 kBytes
	* ble_remote
		- Firmware size: 64.1 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 61.4 kBytes
		- SRAM size: 20.3 kBytes
		- deepsleep retention SRAM size: 16.5 kBytes
	* master_kma_dongle
		- Firmware size: 42.7 kBytes
		- SRAM size: 20.1 kBytes

### Version
* SDK版本:telink_b85m_ble_single_connection_sdk_V3.4.2.4
* Driver版本: telink_b85m_driver_sdk V1.6.0

### Hardware
* N/A

### Note
* Copyright更新，引起所有文件发生变化。

* 电池电压检测是量产的重要功能。
	- 用户必须检查电池电压，以防止在低电压下Flash写入或擦除异常。
	
* Flash保护是量产的必要功能。
	- SDK中默认开启Flash保护。用户必须在最终的量产应用中启用此功能。
	- 在开发和调试阶段，用户可以使用Telink BDT工具中的“Unlock”命令进行Flash解锁。
	- SDK中的Flash保护是基于示例代码的参考设计。考虑到用户的最终应用可能与示例代码不同，如用户的最终固件尺寸更大，或者用户有不同的OTA设计，或者用户需要在Flash的其他区域存储更多的数据，这些差异都意味着SDK中的Flash保护参考设计不可能在不做任何更改的情况下直接用于用户的量产应用。用户应参考示例代码，了解原理和方法，然后根据自己的应用更改和实现更适合的机制。

* 纠正一些拼写错误
	- 如修改blc_pm_modefy_brx_early_set为blc_pm_modify_brx_early_set。

* 删除了一些无用的、冗余的代码
	- 如删除结构体“smp_m_param_save_t”。


### Bug Fixes
* **Software Timer**
	- 修复：调用API "blt_soft_timer_delete"删除一个software timer任务后，在至少存在一个software timer任务的情况下，才对software timer任务重新排序。

* **Application**
	- 修复：白名单示例代码处理对端Central设备特殊情况:在第一个配对阶段使用IDA(Identity Address)，但在后面的重新连接阶段使用RPA(Resolvable Private Address)。

	- 修复：在旧版SDK中，当Central(Master)设备使用RPA地址，且更新CONNECT_IND PDU中的RPA时，直连广播可能不会自动连接。新版SDK中，如果对端IRK有效，将对端IRK添加到解析列表中并启用地址解析后，即使Central(Master)设备刷新RPA，也可以连接直连广播。

* **PHY**
	- 修复：在BQB RX测试中，payload为255Bytes，切换到S2/S8时，会小概率出现失败。

* **ATT**
	- 修复：当server端ATT表最后一项为Primary Service并且无characteristic，会导致服务发现过程中返回错误的handle。


### BREAKING CHANGES 
* **Boot & Link**	
	- 优化cstartup. S和boot.link的实现。
	- 添加cstartup_825x.S，这个文件适用于所有B85工程，SRAM大小通过判断MCU_STARTUP自动设置。以下旧版cstartup文件移除：
		- cstartup_8251_RET_16K.S/cstartup_8251_RET_32K.S/cstartup_8253_RET_16K.S/cstartup_8253_RET_32K.S/cstartup_8258_RET_16K.S/cstartup_8258_RET_32K.S
	- 添加cstartup_827x.S，这个文件适用于所有B87工程，SRAM大小通过判断MCU_STARTUP自动设置。以下旧版cstartup文件移除：
		- cstartup_8271_RET_16K.S/cstartup_8271_RET_32K.S/cstartup_8278_RET_16K.S/cstartup_8278_RET_32K.S/cstartup_8238_RET_16K.S/cstartup_8238_RET_32K.S
	- 添加boot.link，这个文件适用于所有B85/B87工程。以下旧版link文件移除：
		- boot_16k_retn_8238.link/boot_32k_retn_8238.link
		- boot_16k_retn_8251_8253_8258.link/boot_32k_retn_8251.link/boot_32k_retn_8253_8258.link
		- boot_16k_retn_8271_8278.link/boot_32k_retn_871.link/boot_32k_retn_8278.link

	- 根据不同芯片，在project setting中配置"SRAM_SIZE"
		- B85
			- 使用新的宏 MCU_STARTUP_8251/MCU_STARTUP_8253/MCU_STARTUP_8258 来配置芯片, 默认配置为 MCU_STARTUP_8258
			- 以下这些旧的宏不再使用: 
				MCU_STARTUP_8258_RET_16K, MCU_STARTUP_8258_RET_32K, 
				MCU_STARTUP_8253_RET_16K, MCU_STARTUP_8253_RET_32K, 
				MCU_STARTUP_8251_RET_16K, MCU_STARTUP_8251_RET_32K				
		- B87
			- 使用新的宏 MCU_STARTUP_8271/MCU_STARTUP_8238/MCU_STARTUP_8278 来配置芯片, 默认配置为 MCU_STARTUP_8278
			- 以下这些旧的宏不再使用: 
				MCU_STARTUP_8278_RET_16K, MCU_STARTUP_8278_RET_32K, 
				MCU_STARTUP_8238_RET_16K, MCU_STARTUP_8238_RET_32K, 
				MCU_STARTUP_8271_RET_16K, MCU_STARTUP_8271_RET_32K
	- 自动配置deepsleep retention SRAM大小(16K或32K)，用户无需关注。
		- "blc_app_setDeepsleepRetentionSramSize"用来自动配置deepsleep retention SRAM大小。
		- "\_retention_size_"在cstarup.S中定义，用于自动计算retention SRAM大小。
		如果“_retention_size_”超过最大值32K字节，编译器将报告错误"Error: Retention RAM size overflow."，以便用户注意到这一点。
		- 在vendor/common中添加user_config.c，在此文件中添加宏"__PM_DEEPSLEEP_RETENTION_ENABLE"，用于在boot.link中自动计算和报警retention SRAM size。
		  "__PM_DEEPSLEEP_RETENTION_ENABLE"等于用户在app_config.h中配置的"PM_DEEPSLEEP_RETENTTION_ENABLE"。


* **Application**
	- 调整BLE Flash配置和校准
		- 将blt_common.h/blt_common.c重命名为ble_flash.h/ble_flash.c。
		- 将函数"blc_app_loadCustomizedParameters"拆分为两个函数"blc_app_loadCustomizedParameters_normal"和"blc_app_loadCustomizedParameters_deepRetn"。第一个是在MCU上电或从deepsleep模式唤醒时使用，第二个是当MCU从deepsleep retention模式唤醒时使用的。
		- 添加API “blc_flash_read_mid_get_vendor_set_capacity”以获取Flash mid、供应商和容量信息。

* **ATT**
	- 一些旧的ATT API不建议建议使用，每个API有更好的替代API，旧API声明已隐藏。在ble_comp.h中只保留一些外部声明，以防某些用户继续使用这些API。
		- 隐藏API "bls_att_pushNotifyData"，用户应使用API "blc_gatt_pushHandleValueNotify"
		- 隐藏API "bls_att_pushIndicateData"，用户应使用API "blc_gatt_pushHandleValueIndicate"
		- 隐藏API "att_req_find_info"，用户应使用API "blc_gatt_pushFindInformationRequest"
		- 隐藏API “att_req_find_by_type”，用户应使用API “blc_gatt_pushFindByTypeValueRequest”
		- 隐藏API "att_req_read_by_type"，用户应使用API "blc_gatt_pushReadByTypeRequest"
		- 隐藏API “att_req_read”，用户应使用API “blc_gatt_pushReadRequest”
		- 隐藏API "att_req_read_blob"，用户应使用API "blc_gatt_pushReadBlobRequest"
		- 隐藏API "att_req_read_by_group_type"，用户应使用API "blc_gatt_pushReadByGroupTypeRequest"
		- 隐藏API “att_req_write”，用户应使用API “blc_gatt_pushWriteRequest”
		- 隐藏API "att_req_write_cmd"，用户应使用API "blc_gatt_pushWriteCommand"
	- 移除API “blt_att_resetRxMtuSize”
	- blt_att_setEffectiveMtuSize修改为blc_att_setEffectiveMtuSize
	- blt_att_resetEffectiveMtuSize修改为blc_att_resetEffectiveMtuSize
	
* **L2CAP**
	- 隐藏API “blc_l2cap_pushData_2_controller”。此为协议栈内部API，用户不应使用它。在旧版本中，被用来发送Confirm命令，可使用API “blc_gatt_pushConfirm”替换它。
	- 隐藏宏 “L2CAP_RX_BUFF_LEN_MAX”。此为协议栈内部使用，用户不应使用它。
	- 修改ATT_RX_MTU_SIZE_MAX为ATT_MTU_MAX_SDK_DFT_BUF。
	- API “blc_l2cap_initMtuBuffer”增加错误返回值。

* **SMP**
	- 删除API “blc_smp_setPeerAddrResSupportFlg”。这是一个特殊的API，用于在旧版本中的Local RPA，如果用户需要，可为客户提供特殊的库。目前通用库中包含了新的Local RPA特性，不再需要该API。用户可以参考Privacy功能重新设计。
	- 删除smp_alg.h，修改为algorithm\crypto\crypto_alg.h，并对其中API重命名


* **Controller**
	- 隐藏API “blc_ll_readMaxAdvDataLength”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_readMaxAdvDataLength”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_setInitTxDataLength”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_GetBrxNextTick”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_get_macAddrRandom”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_get_macAddrPublic”。这是一个协议栈内部API，用户不应使用它。
	- 隐藏API “blc_ll_readMaxAdvDataLength”。这是一个协议栈内部API，用户不应使用它。
	- 删除API “blc_ll_setAclConnMaxOctetsNumber”。
	- 隐藏协议栈内部使用的结构体：
		- ll_data_extension_t
		- rf_pkt_pri_adv_t
		- ll_ext_adv_t
		- ble_format.h中部分结构体
	
* **HCI**
	- 删除"event_connection_update_t"，用户可使用"hci_le_connectionUpdateCompleteEvt_t".
	- 隐藏API “blc_hci_le_getLocalSupportedFeatures“。这是一个协议栈内部API，用户不应使用它。
	
* **OTA** 
	- 隐藏API "bls_ota_clearNewFwDataArea"，不建议再使用。建议使用API “blc_ota_initOtaServer_module”替代。
	- bls_ota_set_fwSize_and_fwBootAddr修改为blc_ota_setFirmwareSizeAndBootAddress

* **Others** 
	- 修复拼写错误
	- 移除ext_pm.h和ext_rf.h，对用户开放的API已移动到ext_misc.h
	- 移除application\usbstd\usbhw.c、application\usbstd\usbhw.h和application\usbstd\usbhw_i.h，使用driver中usbhw.h和usbhw.c替代
	- 移除adc.h/adc.c中关于ADC校准数据结构“adc_vref_ctr_t”的相关内容，原有的应用层ADC校准数据由变量“adc_gpio_calib_vref”、“adc_gpio_calib_vref_offset”和“adc_vbat_calib_vref”进行存储。
	参考telink_b85m_driver_sdk_Release_Note。

### Features
* **Application**
	- 新增Flash Protection模块和示例
		- 在vendor/common文件夹中添加flash_prot.h/flash_prot.c作为所有应用程序的通用模块。
		- 在B85m_ble_sample/B85m_ble_remote/B85m_ble_module/B85m_master_kma_dongle中增加Flash保护示例代码，用户可以参考示例代码，并必须确保Flash保护功能在应用程序上启用。
		- “app_flash_protection_operation”是所有Flash操作(Flash写、擦除)的通用入口，Flash加锁和解锁都在这个函数中进行。
		- “blc_appRegisterStackFlashOperationCallback”和“flash_prot_op_cb”用于处理一些协议栈内部Flash操作的回调，这些回调可能需要Flash加锁和解锁。
			- 协议栈OTA模块已经正确增加blc_flashProt相关回调，告知应用层SMP需要write/erase Flash的操作。应用层需要对应处理Flash lock/unlock。
			- 如果客户不使用Telink OTA，需参考此设计在OTA代码中添加类似的回调函数供上层使用。
		- 现在只有固件在示例代码中受到保护，系统数据和用户数据现在不受保护。
		- 驱动API “flash_write_status”中中断禁用时间过长，BLE系统中断会延迟，导致接收数据错误。将驱动API “flash_write_status”替换为flash_prot.c中的“flash_write_status”，通过调用LinkLayer API “blc_ll_write_flash_status”来解决此问题。

	- 新增tlkapi_debug模块打印日志
		- 在vendor/common文件夹中新增tlkapi_debug.h与tlkapi_debug.c，作为所有应用程序的通用模块。
		- 在stack/ble/debug文件夹中新增debug.h，新增API“blc_debug_enableStackLog”，“blc_debug_addStackLog”，blc_debug_removeStackLog”控制协议栈log打印。
		- 使用GPIO模拟UART TX时序将打印信息输出，功能与旧版SDK中的“printf”功能相同。
		- 建议使用API "tlkapi_printf"和"tlkapi_send_string_data"，并在app_config.h中使用一些宏来控制日志启用，例如"APP_LOG_EN"。
		- 只支持1M波特率，系统时钟为16M/24M时通过汇编指令控制UART TX时序，系统时钟为32M/48M时通过system_timer控制UART TX时序。IRQ禁用和恢复在函数“uart_put”中使用，以保证UART TX字节数据的完整性不被IRQ破坏。如果波特率过低，IRQ禁用时间过长，会影响BLE系统中断或者用户中断，可能导致严重错误。

	- 新增协议栈初始化错误检查
		- 新增API “blc_contr_checkControllerInitialization”检查是否有Controller初始化失败。
		- 新增API "blc_host_checkHostInitialization"检查是否有Host初始化失败。
		- 在ble_common.h中增加枚举定义“init_err_t”，区分错误类型。用户应添加UI报警动作来确认是否发生初始化错误，并根据“init_err_t”中的定义确定原因。

* **Feature Test**
	- 新增feature_emi_test：EMI测试的示例代码。

	- 新增feature_privacy_slave：测试peripheral RPA解析的示例代码。如果在CONNECT_IND包中使用RPA(Resolved Private Address)，该示例代码可通过对端设备的IDA解析。

	- 新增feature_privacy_master：测试Central RPA解析的示例代码。

	- 新增feature_soft_uart：Software UART TX和RX功能的示例代码。

	- 新增feature_usb_cdc：Central端USB CDC的示例代码。

	- 新增feature_smp_security：SC OOB的示例代码。MDSI、MISI模式下从机支持UART输入。
	
	- 新增feature_COC_slave：Peripheral端L2CAP COC的示例代码。

	- 新增feature_multi_local_dev：Multi Local Device示例代码。

	- 新增feature_md_master：Central端More Data的测试代码。

	- 新增feature_md_slave：Peripheral端More Data的测试代码。

* **Controller**
	- 新增新的Telink定义的Controller事件“BLT_EV_FLAG_ADV_TX_EACH_CHANNEL”，在各个ADV通道(37/38/39)发送ADV包前触发。

	- 新增新的Telink定义的Controller事件“BLT_EV_FLAG_SCAN_REQ”，在收到Scan Request后触发。

	- 新增API “blc_contr_setBluetoothVersion”，用于在“LL_VERSION_IND” PDU中设置Controller版本，默认版本为5.3。

	- 在Legacy扫描的Advertising Report事件中添加时间戳(以System Timer Tick计)。
		- 注意，这是定制功能，而不是标准的BLE功能。
		- 使用API “blc_ll_advReport_setRxPacketTickEnable”来启用此功能。
		- 使用新结构体“hci_tlk_advReportWithRxTickEvt_t”确认Advertising Report数据。

	- 新增宏“GET_RXADD_FROM_CONNECT_EVT_DATA”，从Controller事件“BLT_EV_FLAG_CONNECT”的回调数据中获取RX地址类型。
	
	- 新增API “blc_ll_scanReq_filter_en”，用于设置发送Scan Request是否进行过滤。

	- 新增API “bls_ll_setScanChannelMap”，用于设置Scan的Channel Map。

	- 新增API “blc_ll_get_connEffectiveMaxTxOctets”和blc_ll_get_connEffectiveMaxRxOctets，用于获取生效的最大DLE长度。

	- 新增API “blc_ll_setAdvIntervalCheckEnable”，用于设置是否检查广播interval参数。

	- 新增API “bls_pm_getNexteventWakeupTick”，用于获取下次唤醒时间。

* **HCI**
	- 新增API “blc_hci_le_readBufferSize_cmd”，用于读取buffer大小。

* **GATT**
	- 新增如下API：
		- blc_gatt_pushMultiHandleValueNotify用于支持server端向client端同时发送不同ATT handle对应的值。
		- blc_gatt_pushPrepareWriteRequest用于向server端发送Prepare Write命令。
		- blc_gatt_pushExecuteWriteRequest用于向server端发送执行/取消写操作的命令。
		- blc_gatt_pushConfirm用于发送Confirm。
		- blc_gatt_pushReadMultiRequest用于发送同时读取多个ATT handle值的命令，并且值的长度固定。
		- blc_gatt_pushReadMultiVariableRequest用于发送用于发送同时读取多个ATT handle值的命令，并且值的长度可变。
		- blc_gatt_pushErrResponse用于发送Error Response。

* **ATT**
	- 新增API “blc_att_holdAttributeResponsePayloadDuringPairingPhase”用于设置在配对阶段是否阻塞ATT Response PDU。
	- 新增API “blc_att_enableReadReqReject”用于设置是否协议栈根据读回调返回值自动发送Error Response。
	- 新增API “blc_att_enableWriteReqReject”用于设置是否协议栈根据写回调返回值自动发送Error Response。

* **L2CAP**	
	- 支持Peripheral L2CAP COC
	- 新增文件夹stack/ble/host/signaling，并在该文件夹中新增signaling.h.
		- 新增如下API：
			- blc_l2cap_registerCocModule用于注册COC模块。
			- blc_l2cap_disconnectCocChannel用于对COC通道断开连接。
			- blc_l2cap_createLeCreditBasedConnect用于建立基于LE credit的连接。
			- blc_l2cap_createCreditBasedConnect用于建立基于credit的连接。
			- blc_l2cap_sendCocData用于发送COC数据。
		- gap_event.h中新增以下host event：
			- GAP_EVT_L2CAP_COC_CONNECT 在COC连接时触发。
			- GAP_EVT_L2CAP_COC_DISCONNECT 在COC断连时触发。
			- GAP_EVT_L2CAP_COC_RECONFIGURE 在COC连接通道参数更新时触发。
			- GAP_EVT_L2CAP_COC_RECV_DATA 在COC通道接收到新数据时触发。
			- GAP_EVT_L2CAP_COC_SEND_DATA_FINISH 在COC通道数据发送完成时触发。
			- GAP_EVT_L2CAP_COC_CREATE_CONNECT_FINISH 在创建COC通道指令发送成功时触发。

* **SMP**
	- 支持Peripheral SC OOB
		- 新增如下API：
			- blc_smp_generateScOobData用于生成SC OOB数据
			- blc_smp_setScOobData用于设置SC OOB数据

		- 新增如下Host event及对应参数结构体：
			- GAP_EVT_MASK_SMP_TK_SEND_SC_OOB_DATA 在对端请求SC OOB时触发。
			- GAP_EVT_MASK_SMP_TK_REQUEST_SC_OOB 在SMP请求SC OOB数据时触发。

	- 增加Central(master) SMP配对信息对应的结构体定义“smp_param_master_t”。
		- peer_id_adrType/peer_id_addr/local_irk/peer_irk可用于创建白名单和解析列表。

	- 新增API "blm_smp_getPairedDeviceNumber"用于Central(Master)设备获取当前配对的设备编号。
	- 新增API "blm_smp_loadParametersByIndex"用于中央(主)设备读取配对参数的索引。
	- 新增API "blc_smp_manualSetPinCode_for_debug”用于Passkey Entry中手动设置PinCode
	- 新增API "blc_smp_enableSecureConnections”用于使能Secure Connection

* **OTA** 
	- 增加一个OTA错误类型“OTA_MCU_NOT_SUPPORTED”，B85由于特殊原因不支持大于80的OTA PDU。
	- 添加API“blc_ota_getCurrentUsedMultipleBootAddress”


* **Privacy**	
	- 支持BLE从设备和主设备的Privacy Local RPA
		- 新增API "blc_ll_initPrivacyLocalRpa"用于使能Local RPA功能。
		- 新增API "blc_smp_generateLocalIrk"用于生成所需的Local IRK。
		- 新增API "blc_app_isIrkValid"检查存在在Flash中的IRK（Local IRK或Peer IRK）是否可用。


* **Others**
	- 新增Peripheral Multi Local Device功能
		- 此为定制功能，不是BLE标准功能。一个设备可以虚拟扩展到多达4个设备，具有不同的MAC地址(public或static random)。示例Demo中，这些设备可以动态切换，每次只生效一个设备，不能并发。
		- 新增文件夹stack/ble/device,并在该文件夹中新增multi_device.h。
		- 新增链路层API "blc_ll_setMultipleLocalDeviceEnable"、" blc_ll_setlocaldeviceindexanddidentityaddress "和"blc_ll_setCurrentLocalDevice_by_index"。
		- 新增SMP API "blc_smp_multi_device_param_getCurrentBondingDeviceNumber"和"bls_smp_multi_device_param_loadByIndex"。
		- 移除配对信息存储中的peer_key_size，修改为联合体comb_flg_t。
	
	- 新增USB CDC功能
		- 新增API “usb_cdc_tx_data_to_host”向USB Host发送数据。
		- 新增API “usb_cdc_rx_data_to_host”接受USB Host发送的数据。
		- 新增API “usb_cdc_irq_data_process”处理USB CDC中断数据。

	-	Google Audio由0.4e升级到1.0，支持audio dle。

	- 新增版本信息功能
		- 增加API “blc_get_sdk_version”获取SDK版本。
		- 在bin文件的末尾添加SDK版本，在common文件夹下新增sdk_version.c和sdk_version.h。
		- 在编译后显示SDK版本。

	- 新增mcu_config.h，包含MCU对应的功能支持
	- 新增ext_calibration.c和ext_calibration.h，支持ADC校准，包括flash两点GPIO校准，flash单点GPIO校准，flash单点VBAT校准（只有B87支持）和efuse单点校准；支持flash电压值校准。
	- 新增software_uart.h和software_uart.c，供feature_soft_uart使用

### Refactoring
* **Application**
	- 添加app_common.h/app_common.c，用于处理所有应用项目的一些通用功能。
		- 将SYS_CLK_TYPE/CLOCK_SYS_CLOCK_1S/CLOCK_SYS_CLOCK_1MS/CLOCK_SYS_CLOCK_1US定义从app_config.h移至app_common.h。用户只需在app_config.h中定义CLOCK_SYS_CLOCK_HZ。
		- 在app_common中增加API “blc_app_setDeepsleepRetentionSramSize”，自动设置所有应用项目的Deepsleep Retention保留SRAM大小。
		- 在app_common中增加API "blc_app_checkControllerHostInitialization"，用于检查应用项目的协议栈初始化错误。
		- 在app_common中增加API "blc_app_isIrkValid"来检查本地或对端IRK是否有效，可用于白名单和地址解析相关的应用。

	- 添加app_buffer.h/app_buffer.C，用于处理所有应用项目的FIFO大小的配置。

	- Battery Check移动到common下，应用只处理UI相关的部分。

	- 添加simple_sdp.h/simple_sdp.c处理central端通用的simple sdp流程。

	- 添加custom_pair.h/custom_pair.c处理central端通用的custom pair流程。

	- 使用"rf_drv_ble_init"代替main.c中的"rf_drv_init(RF_MODE_BLE_1M)"。
		- 如果用户需要使用其他射频模式，可以改变“rf_drv_init”参数“rf_mode”切换射频模式。

	- 开发板选择
		- 删除每个工程app_config.h里的LED、Keyboard的冗余配置信息，用户可以通过定义BOARD_SELECT来选择开发板
		- 新增vendor/common/boards文件夹和以下定义各个硬件配置的文件：
			- boards_config.h、C1T139A3.h、C1T139A30.h、C1T139A5.h、C1T197A30.h、C1T197A5.h、C1T201A3.h

* **BLE General**
	- 新增stack/ble/ble_comp.h，用于新版本SDK兼容旧版本SDK中一些API/宏/结构体
	- 为所有应用程序添加协议栈初始化错误检查

* **Feature Test**

	- feature_master_dle修改为feature_DLE_master
	- feature_slave_dle修改为feature_DLE_slave
	- 删除feature_phy_extend_adv，示例合入feature_extend_adv
	
* **Controller**
	- 新增stack/ble/ble_controller/controller.h。在这个文件中放入一些Controller相关的枚举和API声明。

	- 为一些Telink定义的Controller事件添加回调数据结构体：
		- "tlk_contr_evt_connect_t"对应"BLT_EV_FLAG_CONNECT"
		- "tlk_contr_evt_terminate_t"对应"BLT_EV_FLAG_TERMINATE"
		- "tlk_contr_evt_dataLenExg_t"对应"BLT_EV_FLAG_DATA_LENGTH_EXCHANGE"
		- "tlk_contr_evt_chnMapRequest_t"对应"BLT_EV_FLAG_CHN_MAP_REQ"
		- "tlk_contr_evt_chnMapUpdate_t"对应"BLT_EV_FLAG_CHN_MAP_UPDATE"
		- "tlk_contr_evt_connParaReq_t"对应"BLT_EV_FLAG_CONN_PARA_REQ"
		- "tlk_contr_evt_connParaUpdate_t"对应"BLT_EV_FLAG_CONN_PARA_UPDATE"
		- "tlk_contr_evt_versionIndRev_t"对应"BLT_EV_FLAG_VERSION_IND_REV"	
		- "tlk_contr_evt_scanReq_t"对应"BLT_EV_FLAG_SCAN_REQ"

* **SMP**
	- API “blc_smp_param_setBondingDeviceMaxNumber”新增初始化错误返回值
	- ACL central(Master) SMP参数
		- 删除未使用且错误的结构体“smp_m_param_save_t”。
		- 删除未使用的结构体 "mac_adr_t"。
		- 增加结构体“smp_param_master_t”，用户读取SMP存储参数时参考。

* **Algorithm**
	- 调整algorithm文件结构

* **Others**
	- drivers/8258/driver_ext/mcu_boot.h中删除MULTI_BOOT_ADDR_0x80000，B85不支持0x80000启动
	- 移动rf_pa.c和rf_pa.h到driver_ext文件夹中
	- 移动compiler.h到common文件夹中

### Performance Improvements
   - “blc_readFlashSize_autoConfigCustomFlashSector”和“blc_app_loadCustomizedParameters_normal”移动到函数“user_init_normal”中调用，以提高代码效率。


### Known issues
  - "ble_sample" Firmware大小增加约5kBytes
	- Print log: 约1.5kBytes
	- Flash protection: 约1kBytes
	- RPA: 约2.5kBytes

### CodeSize
* **B85**
	* ble_sample
		- Firmware size: 51.1 kBytes
		- SRAM size: 17.7 kBytes
		- deepsleep retention SRAM size: 14.2 kBytes
	* ble_remote
		- Firmware size: 64.6 kBytes
		- SRAM size: 20.9 kBytes
		- deepsleep retention SRAM size: 15.5 kBytes
	* ble_module
		- Firmware size: 61.1 kBytes
		- SRAM size: 20.1 kBytes
		- deepsleep retention SRAM size: 16.4 kBytes
	* master_kma_dongle
		- Firmware size: 42.8 kBytes
		- SRAM size: 19.9 kBytes
  
* **B87**
	* ble_sample
		- Firmware size: 51.3 kBytes
		- SRAM size: 17.9 kBytes
		- deepsleep retention SRAM size: 14.3 kBytes
	* ble_remote
		- Firmware size: 64.1 kBytes
		- SRAM size: 21.1 kBytes
		- deepsleep retention SRAM size: 15.7 kBytes
	* ble_module
		- Firmware size: 61.4 kBytes
		- SRAM size: 20.3 kBytes
		- deepsleep retention SRAM size: 16.5 kBytes
	* master_kma_dongle
		- Firmware size: 42.7 kBytes
		- SRAM size: 20.1 kBytes


## V3.4.2.2_Patch_0001

### Features
* SDK version：telink_B85m_ble_single_connection_sdk_v3.4.2.2_patch_0001.
* Add new feature: Host support connection-oriented channels(COC).
* Add telink defined controller event "BLT_EV_FLAG_ADV_TX_EACH_CHANNEL" to meet some customer's special requirement.
* Add a new API "blc_smp_sendKeypressNotify" to send Keypress Notification during TK input phrase in secure connection.
* Add a new API "blc_ll_setBluetoothVersion" for changing default Bluetooth version to meet some customer's special requirement.


### Bug Fixes
* Fix an issue: Central device send "CONNECT_IND" PDU with some extreme parameters(WinOffset is 0, Interval - WinSize = 1) ,and transmiting this packet at the beginning of the timing range specified by WinSize,  will trigger peripheral device connect establish fail with a low probability.
* Fix an issue: Some special Central device sends "LL_PAUSE_ENC_REQ" request at encryption procedure, may trigger peripheral device working at error flow with very low probability, leading to disconnection.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_B85m_ble_single_connection_sdk_v3.4.2.2_patch_0001.
* 增加支持connection-oriented channels(COC).
* 增加telink自定义controller事件BLT_EV_FLAG_ADV_TX_EACH_CHANNEL，以满足一些客户的特殊要求。
* 增加API blc_smp_sendKeypressNotify，支持secure connection的Keypress Notification。
* 增加API blc_ll_setBluetoothVersion，用于更改默认蓝牙版本，以满足一些客户的特殊要求。

### Bug Fixes
* 修复:Central设备发送一个特殊的CONNECT_IND包 (WinOffset为0,Interval - WinSize = 1)，并且在WinSize指定时间范围的开始发送数据包，将有很低概率导致peripheral设备连接建立失败。
* 修复:一些特殊的Central设备在加密过程中发送“LL_PAUSE_ENC_REQ”请求，极低的概率可能会触发Peripheral设备工作错误，导致断开连接。


### BREAKING CHANGES
* N/A.


## V3.4.2.2


### Features
* SDK version:telink_b85m_ble_single_connection_sdk_V3.4.2.2
* Normalizes the naming of content related to private schemas.


### BREAKING CHANGES
* N/A.


### Features
* SDK 版本:telink_b85m_ble_single_connection_sdk_V3.4.2.2
* 规范化与私有模式相关内容的命名

### BREAKING CHANGES
* N/A.

## V3.4.2.1_Patch_0006

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0006.
* Support P25Q80U for B87.


### Bug Fixes
* Fix an issue that very few B85-chips may run abnormally when the clock is 48M.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0005.
* B87支持P25Q80U.


### Bug Fixes
* 修复了少数b85芯片使用48M时钟时可能运行异常的问题.


### BREAKING CHANGES
* N/A.

## V3.4.2.1_Patch_0005

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0005.
* Add MCU_STALL function, use bls_adv_peak_current_optimize API to decrease ADV_power.
* Optimize ADV function, use bls_adv_decrease_time_optimize API to decrease ADV_power.


### Bug Fixes
* Fix EXT_ADV_Bug: it can not advertise in some cases.
* Fix an issue: secure_connect failed in some specific cases.
* Fix an issue: T_IFS is not 150us correctly while SCANNING_IN_ADV_AND_CONN_SLAVE_ROLE.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0005.
* 增加MCU_STALL功能，使用bls_adv_peak_current_optimize，开启此功能可降低广播功耗。
* 优化广播功能，使用bls_adv_decrease_time_optimize，开启此功能可降低广播功耗。


### Bug Fixes
* 修复扩展广播bug：一些情况下广播不出来的问题。
* 修复问题：secure connect在一些特定情况下会失败。
* 修复问题：在SCANNING_IN_ADV_AND_CONN_SLAVE_ROLE下T_IFS 150us时间不准。


### BREAKING CHANGES
* N/A.

## V3.4.2.1_Patch_0004

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0004.
* Add write request reject API.
* The maximal SMP bond number is changed to 8.
* Support 8238.

### Bug Fixes
* Fix an issue: the IRK of the same device may change


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0004.
* 增加write request reject API.
* 最大SMP bond number修改为8。
* 支持8238


### Bug Fixes
* 修复一个问题：相同设备的IRK未保持一致。


### BREAKING CHANGES
* N/A.

## V3.4.2.1_Patch_0003

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0003.


### Bug Fixes
* Fix issue when update firmware from version before 3.4.2.0 to 3.4.2.0 and later by OTA, user may fail to reconnect and have to re-pair product.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0003.


### Bug Fixes
* 修复一个问题：当客户程序从3.4.2.0之前的版本通过OTA升级到3.4.2.0及以后，会遇到无法重连，只能重新配对。


### BREAKING CHANGES
* N/A.


## V3.4.2.1_Patch_0002

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0002.


### Bug Fixes
* Fix boundary conditions issue of ATT.


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0002.


### Bug Fixes
* 增加处理ATT边界条件下的错误.


### BREAKING CHANGES
* N/A.

## V3.4.2.1_Patch_0001

### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0001.


### Bug Fixes
* Add parameter cheack when doing EXCHANGE_MTU_SIZE .


### BREAKING CHANGES
* N/A.



### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.1_patch_0001.


### Bug Fixes
* 增加处理EXCHANGE_MTU_SIZE前的参数监测.


### BREAKING CHANGES
* N/A.

## V3.4.2.1


### Features
* SDK version:telink_b85m_ble_single_connection_sdk_V3.4.2.1
* Compatible with Linux 


### BREAKING CHANGES
* N/A.


### Features
* SDK 版本:telink_b85m_ble_single_connection_sdk_V3.4.2.1
* 增加Linux编译支持 

### BREAKING CHANGES
* N/A.

## v3.4.2.0


### Features
* SDK version：telink_b85m_ble_single_connection_sdk_v3.4.2.0
* Support B85(8251/8253/8258) & B87(8271/8273/8278) series chips 
* Support BLE 5.0 feature 
	+ BLE Coded PHY (S2,S8) and 2M PHY
	+ Channel Selection Algorithm(CSA #2)
	+ Extended Advertisement with different PHYs
* Add long packet transmission in OTA
* Add Zbit and Puya flash
* Demo illustration：
	+ ble remote
	+ ble sample
	+ ble module
	+ feature test
	+ driver test
	+ hci
	+ master kma dongle 


### BREAKING CHANGES
* N/A.


### Features
* SDK 版本：telink_b85m_ble_single_connection_sdk_v3.4.2.0
* 支持B85(8251/8253/8258) & B87(8271/8273/8278)系列芯片
* 支持BLE 5.0特性
	+ BLE Coded PHY (S2,S8) 以及2M PHY
	+ 跳频算法#2(CSA #2)
	+ 不同PHY下的扩展广播
* 增加OTA长包模式传输
* 新增Zbit flash 以及Puya flash
* Demo描述：
	+ ble remote
	+ ble sample
	+ ble module
	+ feature test
	+ driver test
	+ hci
	+ master kma dongle 

### BREAKING CHANGES
* N/A.
