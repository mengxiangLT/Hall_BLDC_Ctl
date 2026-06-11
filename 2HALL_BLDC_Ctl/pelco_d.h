#ifndef __PELCO_D_H__
#define __PELCO_D_H__

#include "gd32f30x.h"

/* Pelco-D 命令结构体 */
typedef struct {
    uint8_t sync_byte;      /* 同步字节，固定0xFF */
    uint8_t address;        /* 设备地址 1-255 */
    uint8_t cmd1;           /* 命令码1 */
    uint8_t cmd2;           /* 命令码2 */
    uint8_t data1;          /* 数据1 (Pan速度) */
    uint8_t data2;          /* 数据2 (Tilt速度) */
    uint8_t checksum;       /* 校验码 */
} PelcoD_Command;

/* 命令解析结果 */
typedef struct {
    uint8_t valid;          /* 命令是否有效 */
    uint8_t address;        /* 目标地址 */
    uint8_t is_broadcast;   /* 是否广播地址(0xFF) */
    
    /* 云台控制 */
    uint8_t pan_right;      /* 右转 */
    uint8_t pan_left;       /* 左转 */
    uint8_t tilt_up;        /* 上转 */
    uint8_t tilt_down;      /* 下转 */
    uint8_t pan_speed;      /* 水平速度 0-63, 0xFF=最高速 */
    uint8_t tilt_speed;     /* 垂直速度 0-63, 0xFF=最高速 */
    
    /* 镜头控制 */
    uint8_t zoom_in;        /* 变倍+ */
    uint8_t zoom_out;       /* 变倍- */
    uint8_t focus_near;     /* 聚焦近 */
    uint8_t focus_far;      /* 聚焦远 */
    uint8_t iris_close;     /* 光圈关 */
    uint8_t iris_open;      /* 光圈开 */
    
    /* 预置位 */
    uint8_t set_preset;     /* 设置预置位 */
    uint8_t call_preset;    /* 调用预置位 */
    uint8_t clear_preset;   /* 清除预置位 */
    uint8_t preset_id;      /* 预置位ID (0-255) */
    
    /* 其他 */
    uint8_t auto_scan;      /* 自动扫描 */
    uint8_t flip;           /* 图像翻转 */
    uint8_t reset;          /* 复位 */
} PelcoD_ParseResult;

/* 函数声明 */
void pelco_d_set_address(uint8_t addr);
void pelco_d_uart_init(uint32_t baudrate);
void pelco_d_receive_byte(uint8_t data);
uint8_t pelco_d_checksum_check(uint8_t *data, uint8_t len);
void pelco_d_parse_command(PelcoD_Command *cmd, PelcoD_ParseResult *result);
void pelco_d_process_command(PelcoD_ParseResult *result);
void pelco_d_send_response(uint8_t address, uint8_t *data, uint8_t len);

#endif /* __PELCO_D_H__ */

