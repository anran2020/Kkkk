
combine：通道组合并联业务功能模块，简称并机设备；
	实现了，任意通道的组合并联；

1.并机的设备对象架构：
	box,电源箱；
	module，电源箱的通道，即下位机的一个通道，在中位机抽象成一个模块，即最小可控单元，在中位机相当于硬件驱动接口；
	channel，并机通道，由一个或者多个module组成，即下位机的通道组合并联称为一个并机通道；
	