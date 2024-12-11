#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"


// 初始化位图 逐字节赋值0
void bitmap_init(struct bitmap* btmp){
    memset(btmp->bits,0,btmp->btmp_bytes_len);
}
//判断bit_index位是否为1 是返回true
int bitmap_scan_test(struct bitmap* btmp,uint32_t bit_index) {
    uint32_t i = bit_index / 8;
    uint32_t j = bit_index % 8;
    return (btmp->bits[i]) & (BITMAP_MASK << j);
}

// 找到连续cnt个位 值是0
int bitmap_scan(struct bitmap* btmp, uint32_t cnt){
    uint32_t idx_byte = 0;
    while((idx_byte < btmp->btmp_bytes_len) && (0xff == btmp->bits[idx_byte])){
        idx_byte++;
    }
    ASSERT(idx_byte < btmp->btmp_bytes_len);
    if(idx_byte == btmp->btmp_bytes_len){
        return -1; // 找不到可用的连续内存
    }

    int idx_bit = 0;
    while( (uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]){
        idx_bit++;
    }

    int idx_bit_start = 8 * idx_byte + idx_bit;//位级下标
    if (cnt == 1) {
        return idx_bit_start;
    }

    uint32_t bit_left_num = btmp->btmp_bytes_len - (idx_bit_start + 1); //剩余位的个数
    uint32_t idx_next_bit = idx_bit_start + 1;
    uint32_t count = 1;

    idx_bit_start = -1;
    while(bit_left_num-- > 0){
        if(!bitmap_scan_test(btmp,idx_next_bit)){
            count++;
        }else{
            count = 0;
        }
        if(count == cnt){
            idx_bit_start = idx_next_bit - cnt + 1; //加1是因为自己算一个
            break;
        }
        idx_next_bit++;
    }
    return idx_bit_start;
}

void bitmap_set(struct bitmap* btmp, uint32_t bit_idx, int8_t value){
    ASSERT((value == 0) || (value == 1));
    uint32_t idx_byte = bit_idx / 8;
    uint32_t offset = bit_idx % 8;
    if(value){
        btmp->bits[idx_byte] |= (BITMAP_MASK << offset);
    }else{
        btmp->bits[idx_byte] &= ~(BITMAP_MASK << offset);
    }
}