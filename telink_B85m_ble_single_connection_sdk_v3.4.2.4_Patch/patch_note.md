## V3.4.2.4_Patch_0004

### Features
* N/A.

### Bug Fixes
    - Fixed(B87): Fix compile error when using API cpu_long_sleep_wakeup().

### Refactoring
* N/A.

### BREAKING CHANGES
* N/A.


### Features
* N/A.

### Bug Fixes
    - 修复（B87）：修复调用API cpu_long_sleep_wakeup()时的编译错误。

### Refactoring
* N/A.

### BREAKING CHANGES
* N/A.




## V3.4.2.4_Patch_0003

### Features
* N/A.

### Bug Fixes
* **Driver**
    - Fixed: Fix the issue where BIT(7) of the analog register 0x44 is mistakenly set to 1 in abnormal conditions when the wake-up source is zero, causing the cpu_sleep_wakeup() function to return an abnormal value.

* **Stack**
    - Fixed: Fix handling of invalid "ATT_OP_FIND_INFO_REQ" parameter. When the peripheral device receives "ATT_OP_FIND_INFO_REQ" that the start handle is zero or the start handle exceeds the end handle, the ATT server needs to send "ATT_ERROR_RSP", rather than sending an invalid "ATT_OP_FIND_INFO_RSP" packet.
    - Fixed: When the central devices use some special parameters of the connection request, the local device will connect unsuccessfully.
    - Fixed: The input parameter's format of the API blc_att_setEffectiveMtuSize() error, leading to the central role can not use an effective MTU size larger than 255.

### Refactoring
* N/A.

### BREAKING CHANGES
* N/A.


### Features
* N/A.

### Bug Fixes
* **Driver**
    - 修复：模拟寄存器0x44在唤醒源为零的异常情况下，bit7会被误置为1，导致cpu_sleep_wakeup()函数返回值异常。

* **Stack**
    - 修复：修复对无效“ATT_OP_FIND_INFO_REQ”请求参数的处理，当peripheral设备接收到的“ATT_OP_FIND_INFO_REQ”的开始句柄为零或开始句柄超过结束句柄时，应当回复“ATT_ERROR_RSP”，而不是发送一个无效的“ATT_OP_FIND_INFO_RSP”报文。
    - 修复：在central device使用某些特定参数的连接请求，local device会连接失败。
    - 修复：API blc_att_setEffectiveMtuSize()的输入参数格式错误，导致central role无法使用超过255的effective MTU size。

### Refactoring
* N/A.

### BREAKING CHANGES
* N/A.




## V3.4.2.4_Patch_0002

### Features
* N/A.

### Bug Fixes
* **Stack**
    - Fixed: The local device acting as the Peripheral role connects with the peer device acting as the Central role. When the local device terminates the connection, the terminate reason is "HCI_ERR_CONN_TERM_BY_LOCAL_HOST" instead of "HCI_ERR_REMOTE_USER_TERM_CONN".
    - Fixed: When the local device uses the resolvable Private Address(RPA), the identity Address(IDA) in the Identity Address Information packet is incorrectly distributed as the RPA. IDA can only be a public device address or a static random address.

* **Application**
    - Fixed: Fix the "feature_phy_test", "feature_privacy_slave", "feature_ota_hid" and "feature_soft_uart" compile errors.
    - Fixed: The bit[1] of the analog register "DEEP_ANA_REG0" has been used in driver code, so modify the bit[1] to bit[4] in "b85m_ble_sample" and "b85m_ble_remote".

### Refactoring
* **Link**
    - Clean the link warning "warning: dot moved backwards before '.text'".

* **Application**
    - Add the "FLASH_P25Q80U_SUPPORT_EN" macro control in ble_flash.c and custom_pair.c.

### BREAKING CHANGES
* N/A.



### Features
* N/A.

### Bug Fixes
* **Stack**
    - 修复：作为Peripheral角色的本地设备与Central角色的对端设备连接，如果本地设备主动断连，断连原因应为“HCI_ERR_CONN_TERM_BY_LOCAL_HOST”而不是“HCI_ERR_REMOTE_USER_TERM_CONN”。
    - 修复：当本地设备使用可解析地址（RPA）时，Identity Address Information报文中的身份地址（IDA）会被错误地分发为RPA。IDA只能是公共设备地址或静态随机地址。

* **Application**
    - 修复：修复“feature_phy_test”，“feature_privacy_slave”，“feature_ota_hid”和“feature_soft_uart”的编译错误。
    - 修复：模拟寄存器“DEEP_ANA_REG0”的bit[1]已在driver程序中被使用，所以将“b85m_ble_sample”和“b85m_ble_remote”中的bit[1]修改为bit[4]。 

### Refactoring
* **Link**
    - 清理link警告“warning: dot moved backwards before '.text'”。

* **Application**
    - 在ble_flash.c和custom_pair.c中添加“FLASH_P25Q80U_SUPPORT_EN”宏控制。

### BREAKING CHANGES
* N/A.




## V3.4.2.4_Patch_0001

### Features
* N/A.


### Bug Fixes
* Fix an issue: When the read or write permission for attribute values is insufficient, the ATT Server does not respond with the ATT_ERROR_RSP PDU, causing the failure of the Service Discovery.

### BREAKING CHANGES
* N/A.



### Features
* N/A.

### Bug Fixes
* 修复：修复属性值读写权限不足时，ATT Server未回复ATT_ERROR_RSP PDU，导致服务发现失败。


### BREAKING CHANGES
* N/A.
