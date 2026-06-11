#ifndef __VISCA_H__
#define __VISCA_H__

#include "gd32f30x.h"
#include <stdbool.h>

#if 1

/* VISCA 协议相关定义 */
#define VISCA_MAX_PACKET_SIZE       20      /* 最大数据包长度(相对运动需要15字节) */
#define VISCA_HEADER_SIZE           3       /* 包头长度: 发送者+接收者+类型 */
#define VISCA_FOOTER_SIZE           1       /* 包尾: 0xFF终止符 */

/* 环形队列大小 */
#define VISCA_RING_BUFFER_SIZE      128     /* 必须是2的幂次方 */

/* VISCA 命令类型 */
typedef enum {
    VISCA_CMD_COMMAND   = 0x01,      /* 命令包 */
    VISCA_CMD_INQUIRY   = 0x09,      /* 查询包 */
    VISCA_CMD_REPLY     = 0x0A,      /* 应答包 */
    VISCA_CMD_DEVICE    = 0x0B,      /* 设备信息包 */
} ViscaCommandType;

/* VISCA 数据包结构体 */
typedef struct {
    uint8_t sender;                  /* 发送者地址 (0x00-0x1F) */
    uint8_t receiver;                /* 接收者地址 (0x01-0x1F) */
    uint8_t type;                    /* 包类型 (0x01,0x09,0x0A,0x0B) */
    uint8_t payload[VISCA_MAX_PACKET_SIZE - 3];
    uint8_t payload_len;             /* 有效载荷长度 */
    uint8_t complete;                /* 是否完整包 */
} ViscaPacket;

/* 解析后的VISCA命令 */
typedef struct {
    uint8_t camera_id;               /* 相机ID (1-7) */
    uint8_t command_type;            /* 命令类型 */
    uint8_t pan_dir;                 /* 水平方向（右正左负） */
		uint8_t tilt_dir;                /* 垂直方向（上正下负） */
    uint8_t param[12];               /* 参数(相对运动需要更多空间) */
    uint8_t param_cnt;               /* 参数个数 */
    
    /* 相对运动专用字段 */
    uint8_t pan_speed;               /* Pan速度 */
    uint8_t tilt_speed;              /* Tilt速度 */
    int32_t pan_displacement;        /* Pan位移量(有符号) */
    int32_t tilt_displacement;       /* Tilt位移量(有符号) */
    uint8_t is_relative_move;        /* 是否为相对运动命令 */
} ViscaParsedCommand;

/* 环形队列结构体 */
typedef struct {
    uint8_t buffer[VISCA_RING_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t overflow_cnt;  /* 溢出计数 */
} ViscaRingBuffer;

/* VISCA 接收器结构体 */
typedef struct {
    ViscaRingBuffer rx_ring;         /* 接收环形队列 */
    ViscaRingBuffer frame_ring;      /* 完整帧队列 */
    ViscaPacket current_packet;      /* 当前正在接收的数据包 */
    uint8_t rx_state;                /* 接收状态机状态 */
    uint32_t last_byte_time;         /* 上次接收字节时间 */
    uint8_t address;                 /* 本机地址 (默认0x01) */
} ViscaReceiver;

/* 函数声明 */
void visca_uart_init(uint32_t baudrate);
void visca_receiver_init(ViscaReceiver *vr, uint8_t address);
void visca_ring_buffer_init(ViscaRingBuffer *rb);
bool visca_ring_buffer_push(ViscaRingBuffer *rb, uint8_t data);
bool visca_ring_buffer_pop(ViscaRingBuffer *rb, uint8_t *data);
uint32_t visca_ring_buffer_available(ViscaRingBuffer *rb);
void visca_ring_buffer_flush(ViscaRingBuffer *rb);
void visca_receive_byte(ViscaReceiver *vr, uint8_t data);
bool visca_get_packet(ViscaReceiver *vr, ViscaPacket *packet);
void visca_parse_packet(ViscaPacket *packet, ViscaParsedCommand *cmd);
void visca_process_command(ViscaParsedCommand *cmd);
void visca_send_packet(uint8_t *data, uint8_t len);
void visca_send_ack(ViscaReceiver *vr, ViscaPacket *packet);
void visca_send_completion(ViscaReceiver *vr, ViscaPacket *packet);
void visca_send_error(ViscaReceiver *vr, ViscaPacket *packet, uint8_t error_code);

/* VISCA 标准命令宏 */
#define VISCA_ADDRESS_BROADCAST      0x08
#define VISCA_TERMINATOR             0xFF

/* 命令类别 */
#define VISCA_CATEGORY_PTZ           0x01    /* 云台控制 */
#define VISCA_CATEGORY_ZOOM          0x04    /* 变焦控制 */
#define VISCA_CATEGORY_FOCUS         0x02    /* 聚焦控制 */
#define VISCA_CATEGORY_IRIS          0x03    /* 光圈控制 */
#define VISCA_CATEGORY_POWER         0x00    /* 电源控制 */

/* 云台命令操作码 */
#define VISCA_PTZ_UP                 0x01
#define VISCA_PTZ_DOWN               0x02
#define VISCA_PTZ_LEFT               0x03
#define VISCA_PTZ_RIGHT              0x04
#define VISCA_PTZ_UPRIGHT            0x05
#define VISCA_PTZ_UPLEFT             0x06
#define VISCA_PTZ_DOWNRIGHT          0x07
#define VISCA_PTZ_DOWNLEFT           0x08
#define VISCA_PTZ_STOP               0x00
#define VISCA_PTZ_HOME               0x06
#define VISCA_PTZ_RESET              0x07

/* 相对运动命令 */
#define VISCA_PTZ_RELATIVE_MOVE_H    0x06    /* 相对运动命令高字节 */
#define VISCA_PTZ_RELATIVE_MOVE_L    0x03    /* 相对运动命令低字节 */

/* 绝对位置命令 */
#define VISCA_PTZ_ABSOLUTE_MOVE_H    0x02    /* 绝对位置命令高字节 */
#define VISCA_PTZ_ABSOLUTE_MOVE_L    0x00    /* 绝对位置命令低字节 */

/* 变焦操作码 */
#define VISCA_ZOOM_TELE              0x02    /* 拉近 */
#define VISCA_ZOOM_WIDE              0x03    /* 拉远 */
#define VISCA_ZOOM_STOP              0x00
#define VISCA_ZOOM_DIRECT            0x07    /* 直接变焦到指定位置 */

/* 速度范围定义 */
#define VISCA_PAN_SPEED_MIN          0x01    /* Pan最小速度 */
#define VISCA_PAN_SPEED_MAX          0x18    /* Pan最大速度(24) */
#define VISCA_TILT_SPEED_MIN         0x01    /* Tilt最小速度 */
#define VISCA_TILT_SPEED_MAX         0x14    /* Tilt最大速度(20) */

/* 位移量范围定义 */
#define VISCA_PAN_DISPLACEMENT_MIN   -32768  /* Pan最小位移 */
#define VISCA_PAN_DISPLACEMENT_MAX   32767   /* Pan最大位移 */
#define VISCA_TILT_DISPLACEMENT_MIN  -32768  /* Tilt最小位移 */
#define VISCA_TILT_DISPLACEMENT_MAX  32767   /* Tilt最大位移 */

#else
/* VISCA 协议相关定义 */
#define VISCA_MAX_PACKET_SIZE       20      /* 最大数据包长度 */
#define VISCA_HEADER_SIZE           3       /* 包头长度: 发送者+接收者+类型 */
#define VISCA_FOOTER_SIZE           1       /* 包尾: 0xFF终止符 */

/* 环形队列大小 */
#define VISCA_RING_BUFFER_SIZE      128     /* 必须是2的幂次方 */

/* VISCA 命令类型 */
typedef enum {
    VISCA_CMD_COMMAND   = 0x01,      /* 命令包 */
    VISCA_CMD_INQUIRY   = 0x09,      /* 查询包 */
    VISCA_CMD_REPLY     = 0x0A,      /* 应答包 */
    VISCA_CMD_DEVICE    = 0x0B,      /* 设备信息包 */
} ViscaCommandType;

/* VISCA 数据包结构体 */
typedef struct {
    uint8_t sender;                  /* 发送者地址 (0x00-0x1F) */
    uint8_t receiver;                /* 接收者地址 (0x01-0x1F) */
    uint8_t type;                    /* 包类型 (0x01,0x09,0x0A,0x0B) */
    uint8_t payload[VISCA_MAX_PACKET_SIZE - 3];
    uint8_t payload_len;             /* 有效载荷长度 */
    uint8_t complete;                /* 是否完整包 */
} ViscaPacket;

/* 解析后的VISCA命令 */
typedef struct {
    uint8_t camera_id;               /* 相机ID (1-7) */
    uint8_t command_type;            /* 命令类型 */
    uint8_t category;                /* 命令类别 (0x01=云台,0x04=变焦等) */
    uint8_t operation;               /* 操作码 */
    uint8_t param[8];                /* 参数 */
    uint8_t param_cnt;               /* 参数个数 */
} ViscaParsedCommand;

/* 环形队列结构体 */
typedef struct {
    uint8_t buffer[VISCA_RING_BUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t overflow_cnt;  /* 溢出计数 */
} ViscaRingBuffer;

/* VISCA 接收器结构体 */
typedef struct {
    ViscaRingBuffer rx_ring;         /* 接收环形队列 */
    ViscaRingBuffer frame_ring;      /* 完整帧队列 */
    ViscaPacket current_packet;      /* 当前正在接收的数据包 */
    uint8_t rx_state;                /* 接收状态机状态 */
    uint32_t last_byte_time;         /* 上次接收字节时间 */
    uint8_t address;                 /* 本机地址 (默认0x01) */
} ViscaReceiver;

/* 函数声明 */
void visca_uart_init(uint32_t baudrate);
void visca_receiver_init(ViscaReceiver *vr, uint8_t address);
void visca_ring_buffer_init(ViscaRingBuffer *rb);
bool visca_ring_buffer_push(ViscaRingBuffer *rb, uint8_t data);
bool visca_ring_buffer_pop(ViscaRingBuffer *rb, uint8_t *data);
uint32_t visca_ring_buffer_available(ViscaRingBuffer *rb);
void visca_ring_buffer_flush(ViscaRingBuffer *rb);
void visca_receive_byte(ViscaReceiver *vr, uint8_t data);
bool visca_get_packet(ViscaReceiver *vr, ViscaPacket *packet);
void visca_parse_packet(ViscaPacket *packet, ViscaParsedCommand *cmd);
void visca_process_command(ViscaParsedCommand *cmd);
void visca_send_packet(uint8_t *data, uint8_t len);
void visca_send_ack(ViscaReceiver *vr, ViscaPacket *packet);
void visca_send_completion(ViscaReceiver *vr, ViscaPacket *packet);
void visca_send_error(ViscaReceiver *vr, ViscaPacket *packet, uint8_t error_code);

/* VISCA 标准命令宏 */
#define VISCA_ADDRESS_BROADCAST      0x08
#define VISCA_TERMINATOR             0xFF

/* 命令类别 */
#define VISCA_CATEGORY_PTZ           0x01    /* 云台控制 */
#define VISCA_CATEGORY_ZOOM          0x04    /* 变焦控制 */
#define VISCA_CATEGORY_FOCUS         0x02    /* 聚焦控制 */
#define VISCA_CATEGORY_IRIS          0x03    /* 光圈控制 */
#define VISCA_CATEGORY_POWER         0x00    /* 电源控制 */

/* 云台操作码 */
#define VISCA_PTZ_UP                 0x01
#define VISCA_PTZ_DOWN               0x02
#define VISCA_PTZ_LEFT               0x03
#define VISCA_PTZ_RIGHT              0x04
#define VISCA_PTZ_UPRIGHT            0x05
#define VISCA_PTZ_UPLEFT             0x06
#define VISCA_PTZ_DOWNRIGHT          0x07
#define VISCA_PTZ_DOWNLEFT           0x08
#define VISCA_PTZ_STOP               0x00
#define VISCA_PTZ_HOME               0x06
#define VISCA_PTZ_RESET              0x07

/* 变焦操作码 */
#define VISCA_ZOOM_TELE              0x02    /* 拉近 */
#define VISCA_ZOOM_WIDE              0x03    /* 拉远 */
#define VISCA_ZOOM_STOP              0x00
#define VISCA_ZOOM_DIRECT            0x07    /* 直接变焦到指定位置 */
#endif
#endif /* __VISCA_H__ */

