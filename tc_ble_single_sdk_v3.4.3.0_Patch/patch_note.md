# V3.4.3.0_Patch_0002

### Features
  - N/A

### Bug Fixes
- **SMP**
  - For B85/B87/TC321X
    - Fixed an issue where the Central could not reconnect with a Peripheral device that uses an RPA.
      - Detailed Description: After the Central completes the encrypted bonding procedure with a Peripheral using an RPA, it fails to recognize the paired device during disconnection and reconnection, resulting in a failure to reconnect.
      - After Fix: The Central device can now successfully identify and trigger the normal reconnection and encryption procedure with the Peripheral device using an RPA.
      - Update Recommendation: Evaluate if needed.
- **COC**
  - For B85/B87/TC321X
    - Fixed an issue where the Central could not establish an L2CAP CoC connection with a Peripheral device.
      - Detailed Description:When the Central attempts to establish an L2CAP CoC channel with a Peripheral, an underlying protocol stack interaction exception causes the channel setup to fail.
      - After Fix: The Central device can now normally establish an L2CAP CoC connection with the Peripheral device.
      - Update Recommendation: Evaluate if needed.
### Refactoring
- N/A

### BREAKING CHANGES
- N/A



### Features
- N/A

### Bug Fixes
- **SMP**
  - For B85/B87/TC321X
    - 修复了Central无法和使用RPA地址的Peripheral设备进行回连的问题。
      - 详细描述：当Central与使用RPA地址Peripheral设备完成加密绑定后，断连重连时Central无法识别已配对设备，导致无法回连。
      - 修复效果：Central设备可以和使用RPA地址的Peripheral设备进行正常的回连加密流程。
      - 更新建议：自行评估。
- **COC**
  - For B85/B87/TC321X
    - 修复了Central无法与Peripheral设备建立L2CAP CoC连接的问题。。
      - 详细描述：Central与Peripheral设备建立L2CAP CoC通道时，底层协议栈交互异常导致通道无法成功建立。
      - 修复效果：Central可以正常与Peripheral设备建立L2CAP CoC连接。
      - 更新建议：自行评估。


### Refactoring
- N/A

### BREAKING CHANGES
- N/A




# V3.4.3.0_Patch_0001

### Features
- **Application**
    - (B85/B87): Added the flash required for the standalone 2.4G reference sample and implemented protection for it.

### Bug Fixes
- **Audio**
  - For B85/B87/TC321X
    - Fixed the issue where voice could not be started with Google voice mode TVs when using TL_AUDIO_RCU_ADPCM_GATT_GOOGLE mode.
      - Detailed Description: When the local device operates in TL_AUDIO_RCU_ADPCM_GATT_GOOGLE voice mode, the voice channel fails to establish with the TV because the GET_CAPS_RESP packet returned by the local device contains bits that do not comply with the protocol specification.
      - After Fix: Voice can be successfully started with TVs running Google voice mode.
      - Update Recommendation: Evaluate if needed.
- **Application**
  - For TC321X
    - Fixed the issue where the 2.4G OTA test demo has a relatively high probability of OTA upgrade failure.
      - Detailed Description:Insufficient retransmission attempts and inconsistent timeout settings between master and slave.The OTA_END_RSP control packet lacks a retransmission mechanism, causing the master to mistakenly interpret the operation as failed.CRC error packets are not filtered out, resulting in the slave receiving corrupted firmware.
      - After Fix: OTA upgrade can be successfully completed under normal conditions.
      - Update Recommendation: Evaluate if needed.
### Refactoring
- N/A

### BREAKING CHANGES
- N/A



### Features
* **Application**
  - (B85/B87)：增加单独2.4G 参考示例所需要的flash并为其添加保护处理。

### Bug Fixes
- **Audio**
  - For B85/B87/TC321X
    - 修复了当使用TL_AUDIO_RCU_ADPCM_GATT_GOOGLE语音模式时，无法与支持 Google 语音模式的电视开启语音的问题。
      - 详细描述：当本地设备处于 TL_AUDIO_RCU_ADPCM_GATT_GOOGLE 语音模式时，由于本地设备回复的 GET_CAPS_RESP 数据包中存在不符合协议规范的位，导致无法与电视端建立语音通道。
      - 修复效果：可以成功与支持 Google 语音模式的电视开启语音。
      - 更新建议：自行评估。
- **Application**
  - For TC321X
    - 修复了2.4G OTA_Test demo中高概率出现升级失败问题。
      - 详细描述：重传次数不足并且master和slave超时时间不一致；OTA_END_RSP控制包没有重传逻辑导致master误判为失败；crc错包没有被过滤导致slave收到固件异常
      - 修复效果：在正常环境下能够完成OTA升级。
      - 更新建议：自行评估。


### Refactoring
- N/A

### BREAKING CHANGES
- N/A

