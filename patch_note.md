## V3.4.2.8_Patch_0001

### Version

- SDK version: tc_ble_single_sdk V3.4.2.8_Patch_0001
- Chip Version
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0/A1)
- Hardware Version
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
- Platform Version
  - tc_platform_sdk V3.3.1
- Toolchain Version
  - TC32 ELF GCC4.3 ( IDE: [Telink IoT Studio](https://www.telink-semi.com/development-tools) )

### Features

- N/A

### Bug Fixes

- **PM**
  - For TC321X
    - Fixed the disconnection issue caused by frequent timer wake-ups.
      - Detailed Description: When the BLE_APP_PM_ENABLE is enabled, if the application frequently wakes up the system via software timers, clock recovery abnormalities may occur during wake-up. This issue may also occur with low probability under conditions where software timers are not used.
      - After Fix: The clock recovery is functioning normally, and instances of disconnection will no longer occur.
      - Update Recommendation: Mandatory update.
- **Application**
  - For B85
    - Fixed the issue of garbled data when using software-simulated serial communication.
      - Detailed Description: For the B85 chip, under the condition of a 48 MHz system clock frequency, garbled output occurs when using software-simulated serial communication.	
      - After Fix: With the fix implemented, it is now possible to use software-simulated serial communication normally under a 48 MHz system clock frequency.
      - Update Recommendation: Evaluate if needed.

### BREAKING CHANGES

- N/A.

### Refractor

- **link**
  - (B85/B87): Add a new sector "cstartup_ram_funcs".

### Known Issues

- **Application**
  - (TC321X): In the `ble_remote` reference design, after switching the key mode to IR mode, if the chip enters deep sleep and then wakes up, the key mode state will be lost. It is necessary to reconfigure it to IR mode.

### Version

- SDK 版本: tc_ble_single_sdk V3.4.2.8_Patch_0001
- Chip 版本
  - B85: TLSR825X
  - B87: TLSR827X
  - TC321X (A0/A1)
- Hardware 版本
  - B85: C1T139A30_V1_2, C1T139A5_V1_4, C1T139A3_V2_0
  - B87: C1T197A30_V1_1, C1T197A5_V1_1, C1T201A3_V1_0
  - TC321X: C1T357A20_V1_1, C1T362A5_V1_0
- Platform 版本
  - tc_platform_sdk V3.3.1
- Toolchain 版本
  - TC32 ELF GCC4.3 ( IDE: [Telink IoT Studio](https://www.telink-semi.com/development-tools) )

### Features

- N/A

### Bug Fixes

- **PM**
  - For TC321X
    - 修复了因软件定时器频繁唤醒导致的断连问题。
      - 详细描述：当启用 BLE_APP_PM_ENABLE时，如果应用层通过软件定时器频繁唤醒系统，可能会在唤醒过程中出现时钟恢复异常。即使在不使用软件定时器的条件下，该问题也有可能低概率发生。
      - 修复效果：时钟恢复正常，不再出现断连的情况。
      - 更新建议：必须更新。
  
- **Application**
  - For B85
    - 修复了软件模拟串口输出乱码。
      - 详细描述：B85系统时钟在48M条件下，使用软件模拟串口会出现乱码。
      - 修复效果：可以在48M主频下，正常使用软件模拟串口。
      - 更新建议：自行评估。

### BREAKING CHANGES

- N/A.

### Refractor

- link
  - (B85/B87): 添加"cstartup_ram_funcs" 段。

### Known Issues

- **Application**
  - (TC321X): 在`ble_remote`中按键模式切换为IR模式后，如果芯片进入deep sleep，唤醒回来后会丢失按键模式状态，需要重新设置为IR模式。
