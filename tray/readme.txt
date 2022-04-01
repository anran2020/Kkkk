
打开宏 DebugVersion
gcc func.c log.c timer.c box.c flow.c entry.c host.c uart.c plc.c boot.c protect.c channel.c tray.c

gcc upper.c log.c func.c -o client

gcc upper.c log.c func.c -o upper

basic.h
enum.h define.h type.h
log.h func.h timer.h cause.h
flow.h
device.h(entry.h)
box.h host.h rs485.h protect.h channel.h tray.h


电源柜约束：
1、全盘同一种串并类型box，要么全是并联，要么全是极简串联，要么全是旁路串联。以后也不支持不同种类。
2、不支持一个box的通道分属不同托盘。以后也不支持。
3、支持不同种类托盘，如同一中位机下，即有并联托盘，也有串联托盘。
4、一个托盘可以两根can线，每根can线下的拨码无限制。
5、托盘内的box，量程可以不同、每主通道串数可以不同、每box的主通道数可以不同。
上位机约束：
1、进入修调的模式暂仅按整盘，以后需要时可以增加按箱。
2、暂仅支持按通道采样模式，暂不支持整盘采样模式。

ram map
工步、保护：160k
采样缓存：128k
数据结构：64k
消息缓存：64k
flash驱动：32k
其它驱动：32k

---------------缩写，有些为通用，部分为自定-----------------
abnm:  abnormal
aby:   array of byte,按字节计的数组
accu:  accumulate   累加的
ack:   acknowledge  与command配对,用于与下层设备,如下位机
addr:  address
aglt:  agilent   安捷伦
amt:   amount   数量. 不用有歧义的number,数量用amount,动态计数用counter,序号用sequence,编号索引用index.
ap:    air press
arg:   argument
bin:   binary
blk:   block
buf:   buffer
byps:  bypass 旁路
cap:   capacity
cb:    control block
cali:  calibrate 电源柜修调
chk:   check
chn:   channel
cli:   command line interface
clt:   client
cmd:   command
cmplt: complete
cmmn:  common
cnt:   counter   计数
cmmu:  communication
ctnu:  continue
cri:   critical  临界非法值
crnt:  current   当前的
cur:   current   电流
dbg:   debug
def:   default
del:   delete
dld:   download
dscr:  describe
dev:   device--本代码中,dev特指中位机设备
disp:  dispatch
dst:   destination
dyn:   dynamic
ena:   enable
evt:   event
exp:   expression
expr:  expire
fixt:  fixture   工装
fluct: fluctuate  波动
func:  function
grp:   group
id:    identity
idx:   index
init:  initialize
intv:  interval
lvl:   level
itf:   interface
mand:  mandatory
mem:   memory
mbr:   member
mgr:   manager
mntn:  maintain
modu:  modulo
ms:    millionsecond
msg:   message
ndbd:   needlebed
nega:  negative
nml:   normal
np:    negative-pressure
ntfy:  notify
obj:   object
opr:   operate
opt:   option
paral: parallel 并联
param: parameter
pld:   payload
pos:   position
posi:  positive
prec:  precision  精度
pres:  present
prev:  previous
pri:   priority
proc:  process
prot:  protect
proto: protocol
recv:  receive
ref:   reference/refer
reg:   register
rel:   release
req:   request
res:   resource
rcd:   record
rmv:   remove
rsp:   response  与request配对
rsvd:  reserved
s:     second
sec:   section
seg:   segment
seq:   sequence
smpl:  sample
src:   source
sta:   static
std:   standard
str:   string
sw:    switch
sync:  synchronize
sz:    string with zero end 字符串
tid:   timer-id
tmpr:  temperature
ms:    millisecond
trc:   trace
u8e:   unsigned char enum
upld:   upload
upd:   update
us:    microsecond
val:   value
vol:   voltage
wi:    with
wo:    without



