- 配置8253时钟中断芯片，加快时钟中断

（1）counter_port 是计数器的端口号，用来指定初值 counter_value 的目的端口号。
（2）counter_no 用来在控制字中指定所使用的计数器号码，对应于控制字中的 SC1 和 SC2 位。
（3）rwl 用来设置计数器的读/写/锁存方式，对应于控制字中的 RW1 和 RW0 位。
（4）counter_mode 用来设置计数器的工作方式，对应于控制字中的 M2～M0 位。
（5）counter_value 用来设置计数器的计数初值，由于此值是16 位，所以我们用了uint16_t 来定义它。
frequency_set功能是把操作的计数器 counter_no、读写锁属性 rwl、计数器工作模式 counter_mode 写入模式控制寄存器并赋予计数器的计数初值为 counter_value。