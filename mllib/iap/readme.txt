IAP模块：

依赖模块：mcuflash、ntp、ctp、utp；

在线升级，结合flash读写、通讯协议，实现升级设备程序的功能；

1> iap.c.h文件实现iap功能，标准化升级接口和业务逻辑，移植时不需要改动。

2> RA2A1、RA6M4……跟mcu相关的文件夹，
实现在线升级，和boot与app两段代码相关的底层操作，
接口文件iap_mcu_port.c.h;

3>xx_protocol文件夹，协议、硬件平台相关功能实现：
区分协议(ntp\ctp\utp……)，区分硬件平台(硬件+设备类型)


4，关于模块移植，
iap_mcu_port.c.h需要根据不同的mcu编写；xx_protocol文件夹下写相应的协议接口。