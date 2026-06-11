#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

//#include "gd32f30x.h"
//#include <stdbool.h>
#include "includes.h"

/* Pelco-D 帧最大长度（标准7字节，扩展最多16字节） */
#define PELCO_D_MAX_FRAME_LEN    16

/* 环形队列大小（2的幂次方，便于取模运算） */
#define RING_BUFFER_SIZE         64

/* 环形队列结构体 */
typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];   /* 数据缓冲区 */
    volatile uint32_t head;             /* 写指针 */
    volatile uint32_t tail;             /* 读指针 */
    uint32_t size;                      /* 队列大小 */
    uint32_t dropped;                   /* 丢包计数（可选） */
} RingBuffer;

/* Pelco-D 帧解析相关 */
typedef struct {
    RingBuffer rx_ring;                 /* 接收环形队列 */
    uint8_t frame_buffer[PELCO_D_MAX_FRAME_LEN];  /* 临时帧缓冲区 */
    uint8_t frame_index;                /* 当前帧索引 */
    uint32_t last_byte_time;            /* 上次接收字节时间 */
    uint8_t frame_ready;                /* 帧完整标志 */
} PelcoD_RingBuffer;

extern PelcoD_RingBuffer g_pelco_ring;

/* 函数声明 */
void ring_buffer_init(RingBuffer *rb);
bool ring_buffer_is_empty(RingBuffer *rb);
bool ring_buffer_is_full(RingBuffer *rb);
uint32_t ring_buffer_available(RingBuffer *rb);
uint32_t ring_buffer_free_space(RingBuffer *rb);
bool ring_buffer_push(RingBuffer *rb, uint8_t data);
bool ring_buffer_pop(RingBuffer *rb, uint8_t *data);
uint32_t ring_buffer_peek(RingBuffer *rb, uint8_t *data, uint32_t len);
void ring_buffer_flush(RingBuffer *rb);
uint8_t pelco_d_ring_parse_command(PelcoD_RingBuffer *prb, PelcoD_Command *cmd);


void pelco_d_ring_init(PelcoD_RingBuffer *prb);
void pelco_d_ring_push_byte(PelcoD_RingBuffer *prb, uint8_t data);
uint8_t pelco_d_ring_get_frame(PelcoD_RingBuffer *prb, uint8_t *frame, uint8_t *len);

#endif /* __RING_BUFFER_H__ */


