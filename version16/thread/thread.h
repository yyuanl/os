#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"
#include "list.h"
// 自定义通用函数类型 它将在很多线程函数中作为形参类型
typedef void thread_func(void*);

// 进程或线程的状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};

//中断栈intr_stack
struct intr_stack {
    uint32_t vec_no; // kernerl.s中压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;

    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    //以下有cpu从低特权级进入高特权级是压入
    uint32_t error_code;
    void (*eip) (void); // ?? 
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

/*
线程自己的栈，用用存储线程中待执行的函数
次结构在线程自己的内核中位置不固定
仅用在switch_to 时保存的线程环境
实际位置取决于实际运行情况
*/
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;
    /*
    线程第一次执行时，eip指向待调用的函数kernel_thread
    其他时候，eip是指向switch_to的返回地址
    */
   void (*eip) (thread_func* func,void* func_arg);

   //以下仅供第一次被调度上cpu时使用
   // 参数unuset_ret 只为占位置充数为返回地址
   void (*unused_retaddr);
   thread_func* function; //由kernel_thread所调用的函数名
   void* func_arg;//由kernel_thread所调用的函数所需的参数
};

//进程或线程的pcb,程序控制块
struct task_struct {
    uint32_t* self_kstack; //各内核线程都用自己的内核栈；
    enum task_status status;
    char name[16];
    uint8_t priotity; //线程优先级
    uint8_t ticks; //每次再处理器上执行的时间嘀嗒数

    //此任务自上cpu运行后至今占用了多少cpu嘀嗒数；也就是此任务执行了多久
    uint32_t elapsed_ticks;

    //用户线程在一般队列中的结点
    struct list_elem general_tag;

    
    //用户线程队列thread_all_list中的结点
    struct list_elem all_list_tag;

    uint32_t* pddir;

    uint32_t stack_magic; //栈的边界标记，用于检测栈的溢出
};
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);

#endif
