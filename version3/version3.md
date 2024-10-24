- dw define word 2字节
- dd define double 4字节
- db define byte 1个字节
- 修改了mbr跳转地址：加上了拼凑的0x300。疑问：为什么不能mbr直接跳转到LOADER_BASE_ADDR,loader中在跳转到loader_start处？

- 物理内存检测效果
![](../asset/phy_mem_check.png)