#include "memory.h"
#include "bitmap.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "sync.h"

#define PG_SIZE 4096

#define MEM_BITMAP_BASE 0xc009a000 //位图首地址

#define K_HEAP_START 0xc0100000 //内核虚拟地址3G，再跨过低端1M内存

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

struct pool {
   struct bitmap pool_bitmap;	 // 本内存池用到的位图结构,用于管理物理内存
   uint32_t phy_addr_start;	 // 本内存池所管理物理内存的起始地址
   uint32_t pool_size;		 // 本内存池字节容量
   struct lock lock;		 // 申请内存时互斥
};

struct pool kernel_pool,user_pool;
struct virtual_addr kernel_vaddr; //给内核分配虚拟地址

static void* vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
   int vaddr_start = 0, bit_idx_start = -1;
   uint32_t cnt = 0;
   if (pf == PF_KERNEL) {     // 内核内存池
      bit_idx_start  = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
      if (bit_idx_start == -1) {
	 return NULL;
      }
      while(cnt < pg_cnt) {
	 bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
      }
      vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
   } else {	     // 用户内存池	
      struct task_struct* cur = running_thread();
      bit_idx_start  = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
      if (bit_idx_start == -1) {
	 return NULL;
      }

      while(cnt < pg_cnt) {
	 bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
      }
      vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;

   /* (0xc0000000 - PG_SIZE)做为用户3级栈已经在start_process被分配 */
      ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));
   }
   return (void*)vaddr_start;
}


/* 在用户空间中申请4k内存,并返回其虚拟地址 */
void* get_user_pages(uint32_t pg_cnt) {
   lock_acquire(&user_pool.lock);
   void* vaddr = malloc_page(PF_USER, pg_cnt);
   memset(vaddr, 0, pg_cnt * PG_SIZE);
   lock_release(&user_pool.lock);
   return vaddr;
}


//用于为指定的虚拟地址申请一个物理页，传入参数是这个虚拟地址，要申请的物理页所在的地址池的标志。申请失败，返回null
void* get_a_page(enum pool_flags pf, uint32_t vaddr) {
	struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
	lock_acquire(&mem_pool->lock);
	struct task_struct* cur = running_thread();
	int32_t bit_idx = -1;
	/* 若当前是用户进程申请用户内存,就修改用户进程自己的虚拟地址位图 */
	if (cur->pgdir != NULL && pf == PF_USER) {
		bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
		ASSERT(bit_idx > 0);
		bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);
	} 
	else if (cur->pgdir == NULL && pf == PF_KERNEL){
	/* 如果是内核线程申请内核内存,就修改kernel_vaddr. */
		bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
		ASSERT(bit_idx > 0);
		bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
	} 
	else {
		PANIC("get_a_page:not allow kernel alloc userspace or user alloc kernelspace by get_a_page");
	}
	void* page_phyaddr = palloc(mem_pool);
	if (page_phyaddr == NULL)
		return NULL;
	page_table_add((void*)vaddr, page_phyaddr); 
	lock_release(&mem_pool->lock);
	return (void*)vaddr;
}


//将虚拟地址转换成真实的物理地址
uint32_t addr_v2p(uint32_t vaddr) {
   uint32_t* pte = pte_ptr(vaddr);	//将虚拟地址转换成页表对应的页表项的地址
   return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));		//(*pte)的值是页表所在的物理页框地址,去掉其低12位的页表项属性+虚拟地址vaddr的低12位
}


// 计算出虚拟地址vaddr对应pte页表项物理地址的对应虚拟地址
uint32_t* pte_ptr(uint32_t vaddr) {
    //拼凑出一个虚拟地址 用该虚拟地址可以访问到vaddr的pte地址
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr)*4);
    return pte;
}

// 计算出虚拟地址vaddr对应pde页表目录项物理地址的对应虚拟地址
uint32_t* pde_ptr(uint32_t vaddr) {
    uint32_t* pde = (uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}

// 分配一个物理页，成功返回页框的物理地址
static void* palloc(struct pool* m_pool) {
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap,1);
    if (bit_idx == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap,bit_idx,1);
    uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
    return (void*)page_phyaddr;
}
//页表中添加虚拟地址_vaddr和物理地址_page_phyaddr的映射
static void page_table_add(void* _vaddr,void* _page_phyaddr){
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);
    if (*pde & 0x00000001) {//目录项已存在
        ASSERT(!(*pte & 0x00000001));
        if(!(*pte & 0x00000001)){ // pte应该不存在要创建
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); //往页表项的地址写值，物理地址和属性
        }
    }else { // 页目录不存在
        uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool); // 从内核分配一页作为页表
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1); //复制，指向页表

        // 虚拟地址转换规则一致，访问pte就访问到了上面新申请页表，将他清零
        memset((void*)((int)pte & 0xfffff000),0,PG_SIZE);
        ASSERT(!(*pte & 0x00000001));
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

// 分配多个页空间，成功则返回起始虚拟地址
void* malloc_page(enum pool_flags pf,uint32_t pg_cnt) {
/*
1 通过 vaddr_get 在虚拟内存池中申请虚拟地址
2 通过 palloc 在物理内存池中申请物理页
3 通过 page_table_add 将以上得到的虚拟地址和物理地址在页表中完成映射
*/
    void* vaddr_start = vaddr_get(pf,pg_cnt);
    if (vaddr_start == NULL) {
        return NULL;
    }
    uint32_t vaddr = (uint32_t)vaddr_start,cnt = pg_cnt;
    struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

    while(cnt-- > 0){
        void* page_phyaddr = palloc(mem_pool);
        if (page_phyaddr == NULL) {
            return NULL;
        }
        page_table_add((void*)vaddr,page_phyaddr);
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

/* 从内核物理内存池中申请pg_cnt页内存,成功则返回其虚拟地址,失败则返回NULL */
void* get_kernel_pages(uint32_t pg_cnt) {
	lock_acquire(&kernel_pool.lock);
   	void* vaddr =  malloc_page(PF_KERNEL, pg_cnt);
   	if (vaddr != NULL) {	   // 若分配的地址不为空,将页框清0后返回
      	memset(vaddr, 0, pg_cnt * PG_SIZE);
   	}
	lock_release(&kernel_pool.lock);
   	return vaddr;
}


//初始化内核物理内存池与用户物理内存池
static void mem_pool_init(uint32_t all_mem) {
   	put_str("   mem_pool_init start\n");
   	uint32_t page_table_size = PG_SIZE * 256;	  // 页表大小= 1页的页目录表+第0和第768个页目录项指向同一个页表+
                                                  // 第769~1022个页目录项共指向254个页表,共256个页表
   	uint32_t used_mem = page_table_size + 0x100000;	  // 已使用内存 = 1MB + 256个页表
   	uint32_t free_mem = all_mem - used_mem;
   	uint16_t all_free_pages = free_mem / PG_SIZE;        //将所有可用内存转换为页的数量，内存分配以页为单位，丢掉的内存不考虑
   	uint16_t kernel_free_pages = all_free_pages / 2;     //可用内存是用户与内核各一半，所以分到的页自然也是一半
   	uint16_t user_free_pages = all_free_pages - kernel_free_pages;   //用于存储用户空间分到的页

/* 为简化位图操作，余数不处理，坏处是这样做会丢内存。
好处是不用做内存的越界检查,因为位图表示的内存少于实际物理内存*/
   	uint32_t kbm_length = kernel_free_pages / 8;			  // 内核物理内存池的位图长度,位图中的一位表示一页,以字节为单位
   	uint32_t ubm_length = user_free_pages / 8;			  // 用户物理内存池的位图长度.

   	uint32_t kp_start = used_mem;				  // Kernel Pool start,内核使用的物理内存池的起始地址
   	uint32_t up_start = kp_start + kernel_free_pages * PG_SIZE;	  // User Pool start,用户使用的物理内存池的起始地址

   	kernel_pool.phy_addr_start = kp_start;       //赋值给内核使用的物理内存池的起始地址
   	user_pool.phy_addr_start   = up_start;       //赋值给用户使用的物理内存池的起始地址

   	kernel_pool.pool_size = kernel_free_pages * PG_SIZE;     //赋值给内核使用的物理内存池的总大小
   	user_pool.pool_size	 = user_free_pages * PG_SIZE;       //赋值给用户使用的物理内存池的总大小

   	kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;     //赋值给管理内核使用的物理内存池的位图长度
   	user_pool.pool_bitmap.btmp_bytes_len	  = ubm_length;   //赋值给管理用户使用的物理内存池的位图长度

/*********    内核内存池和用户内存池位图   ***********
 *   位图是全局的数据，长度不固定。
 *   全局或静态的数组需要在编译时知道其长度，
 *   而我们需要根据总内存大小算出需要多少字节。
 *   所以改为指定一块内存来生成位图.
 *   ************************************************/
// 内核使用的最高地址是0xc009f000,这是主线程的栈地址.(内核的大小预计为70K左右)
// 32M内存占用的位图是2k.内核内存池的位图先定在MEM_BITMAP_BASE(0xc009a000)处.
   	kernel_pool.pool_bitmap.bits = (void*)MEM_BITMAP_BASE;      //管理内核使用的物理内存池的位图起始地址
							       
/* 用户内存池的位图紧跟在内核内存池位图之后 */
   	user_pool.pool_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length);     //管理用户使用的物理内存池的位图起始地址
   /******************** 输出内存池信息 **********************/
   	put_str("      kernel_pool_bitmap_start:");put_int((int)kernel_pool.pool_bitmap.bits);
   	put_str(" kernel_pool_phy_addr_start:");put_int(kernel_pool.phy_addr_start);
   	put_str("\n");
   	put_str("      user_pool_bitmap_start:");put_int((int)user_pool.pool_bitmap.bits);
   	put_str(" user_pool_phy_addr_start:");put_int(user_pool.phy_addr_start);
   	put_str("\n");

   /* 将位图置0*/
   	bitmap_init(&kernel_pool.pool_bitmap);
   	bitmap_init(&user_pool.pool_bitmap);

	lock_init(&kernel_pool.lock);
   	lock_init(&user_pool.lock);

   /* 下面初始化内核虚拟地址的位图,按实际物理内存大小生成数组。*/
   	kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;      // 赋值给管理内核可以动态使用的虚拟地址池（堆区）的位图长度，
         //其大小与管理内核可使用的物理内存池位图长度相同，因为虚拟内存最终都要转换为真实的物理内存，可用虚拟内存大小超过可用物理内存大小在
         //我们这个简单操作系统无意义（现代操作系统中有意义，因为我们可以把真实物理内存不断换出，回收，来让可用物理内存变相变大)

  /* 位图的数组指向一块未使用的内存,目前定位在内核内存池和用户内存池之外*/
   	kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + kbm_length + ubm_length);   //赋值给管理内核可以动态使用的虚拟内存池（堆区）的位图起始地址

   	kernel_vaddr.vaddr_start = K_HEAP_START;     //赋值给内核可以动态使用的虚拟地址空间的起始地址
   	bitmap_init(&kernel_vaddr.vaddr_bitmap);     //初始化管理内核可以动态使用的虚拟地址池的位图
   	put_str("   mem_pool_init done\n");
}


void mem_init() {
    put_str("mem_init start\n");
    uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
    mem_pool_init(mem_bytes_total);
    put_str("mem_init done\n");
}