//#include "ring_buffer.h"
//#include <string.h>
#include "includes.h"

/* 外部系统时间获取函数 */

/**
 * @brief 初始化环形队列
 * @param rb 环形队列指针
 */
void ring_buffer_init(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->size = RING_BUFFER_SIZE;
    rb->dropped = 0;
    memset((void*)rb->buffer, 0, RING_BUFFER_SIZE);
}

/**
 * @brief 检查队列是否为空
 * @param rb 环形队列指针
 * @return true=空, false=非空
 */
bool ring_buffer_is_empty(RingBuffer *rb)
{
    return (rb->head == rb->tail);
}

/**
 * @brief 检查队列是否已满
 * @param rb 环形队列指针
 * @return true=满, false=未满
 */
bool ring_buffer_is_full(RingBuffer *rb)
{
    return ((rb->head + 1) % rb->size) == rb->tail;
}

/**
 * @brief 获取队列中有效数据字节数
 * @param rb 环形队列指针
 * @return 有效数据字节数
 */
uint32_t ring_buffer_available(RingBuffer *rb)
{
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return rb->size - rb->tail + rb->head;
    }
}

/**
 * @brief 获取队列剩余空间
 * @param rb 环形队列指针
 * @return 剩余空间字节数
 */
uint32_t ring_buffer_free_space(RingBuffer *rb)
{
    return rb->size - ring_buffer_available(rb) - 1;
}

/**
 * @brief 向队列中写入一个字节
 * @param rb 环形队列指针
 * @param data 要写入的数据
 * @return true=成功, false=失败(队列满)
 */
bool ring_buffer_push(RingBuffer *rb, uint8_t data)
{
    uint32_t next_head = (rb->head + 1) % rb->size;
    
    /* 检查队列是否已满 */
    if (next_head == rb->tail) {
        rb->dropped++;
        return false;
    }
    
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    
    return true;
}

/**
 * @brief 从队列中读取一个字节
 * @param rb 环形队列指针
 * @param data 输出数据指针
 * @return true=成功, false=失败(队列空)
 */
bool ring_buffer_pop(RingBuffer *rb, uint8_t *data)
{
    if (ring_buffer_is_empty(rb)) {
        return false;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    
    return true;
}

/**
 * @brief 查看队列中的数据（不删除）
 * @param rb 环形队列指针
 * @param data 输出数据缓冲区
 * @param len 要查看的字节数
 * @return 实际返回的字节数
 */
uint32_t ring_buffer_peek(RingBuffer *rb, uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t available = ring_buffer_available(rb);
    uint32_t read_idx = rb->tail;
    uint32_t copy_len = (len < available) ? len : available;
    
    for (i = 0; i < copy_len; i++) {
        data[i] = rb->buffer[read_idx];
        read_idx = (read_idx + 1) % rb->size;
    }
    
    return copy_len;
}

/**
 * @brief 清空环形队列
 * @param rb 环形队列指针
 */
void ring_buffer_flush(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->dropped = 0;
}

/**
 * @brief 初始化Pelco-D环形队列
 * @param prb Pelco-D环形队列指针
 */
void pelco_d_ring_init(PelcoD_RingBuffer *prb)
{
    ring_buffer_init(&prb->rx_ring);
    memset(prb->frame_buffer, 0, PELCO_D_MAX_FRAME_LEN);
    prb->frame_index = 0;
    prb->last_byte_time = 0;
    prb->frame_ready = 0;
}

/**
 * @brief 向Pelco-D环形队列中压入一个字节（带帧检测）
 * @param prb Pelco-D环形队列指针
 * @param data 要压入的字节
 */
void pelco_d_ring_push_byte(PelcoD_RingBuffer *prb, uint8_t data)
{
    uint32_t current_time = _micros();
    
    /* 帧超时检测：如果超过10ms没有新数据，认为是新的一帧 */
    if ((current_time - prb->last_byte_time) > 10000 && prb->frame_index > 0) {
        /* 上一帧超时未完成，丢弃 */
        prb->frame_index = 0;
        prb->frame_ready = 0;
    }
    
    prb->last_byte_time = current_time;
    
    /* 将数据存入环形队列 */
    if (!ring_buffer_push(&prb->rx_ring, data)) {
        /* 队列满，丢弃最旧的数据 */
        uint8_t dummy;
        ring_buffer_pop(&prb->rx_ring, &dummy);
        ring_buffer_push(&prb->rx_ring, data);
    }
    
    /* 尝试从环形队列中提取完整帧 */
    /* 注意：Pelco-D帧以0xFF开头 */
    if (data == 0xFF && prb->frame_index == 0) {
        /* 新的帧开始 */
        prb->frame_buffer[prb->frame_index++] = data;
    } else if (prb->frame_index > 0) {
        /* 正在接收帧 */
        if (prb->frame_index < PELCO_D_MAX_FRAME_LEN) {
            prb->frame_buffer[prb->frame_index++] = data;
            
            /* 检查是否收到完整帧（标准7字节） */
            if (prb->frame_index >= 7) {
                prb->frame_ready = 1;
            }
        } else {
            /* 帧过长，丢弃 */
            prb->frame_index = 0;
            prb->frame_ready = 0;
        }
    }
}

/**
 * @brief 从Pelco-D环形队列中获取一帧数据
 * @param prb Pelco-D环形队列指针
 * @param frame 输出帧缓冲区
 * @param len 输出帧长度
 * @return 1=有帧, 0=无帧
 */
uint8_t pelco_d_ring_get_frame(PelcoD_RingBuffer *prb, uint8_t *frame, uint8_t *len)
{
    if (!prb->frame_ready) {
        return 0;
    }
    
    /* 复制帧数据 */
    *len = prb->frame_index;
    memcpy(frame, prb->frame_buffer, prb->frame_index);
    
    /* 重置帧状态 */
    prb->frame_index = 0;
    prb->frame_ready = 0;
    
    return 1;
}

/**
 * @brief 从环形队列中解析Pelco-D命令（高级函数）
 * @param prb Pelco-D环形队列指针
 * @param cmd 输出命令结构体
 * @return 1=成功, 0=失败
 */
uint8_t pelco_d_ring_parse_command(PelcoD_RingBuffer *prb, PelcoD_Command *cmd)
{
    uint8_t frame[PELCO_D_MAX_FRAME_LEN];
    uint8_t len;
    uint8_t i;
    uint8_t sum = 0;
    
    /* 获取完整帧 */
    if (!pelco_d_ring_get_frame(prb, frame, &len)) {
        return 0;
    }
    
    /* 检查帧长度（至少7字节） */
    if (len < 7) {
        return 0;
    }
    
    /* 检查同步字节 */
    if (frame[0] != 0xFF) {
        return 0;
    }
    
    /* 验证校验和 */
    for (i = 1; i < 6; i++) {
        sum += frame[i];
    }
    
    if (sum != frame[6]) {
        return 0;  /* 校验失败 */
    }
    
    /* 填充命令结构体 */
    cmd->sync_byte = frame[0];
    cmd->address = frame[1];
    cmd->cmd1 = frame[2];
    cmd->cmd2 = frame[3];
    cmd->data1 = frame[4];
    cmd->data2 = frame[5];
    cmd->checksum = frame[6];
    
    return 1;
}

