#include "includes.h"


extern float set_speed;
/* 在 visca.c 中添加以下函数 */

/**
 * @brief 解析VISCA命令 (增强版，支持相对运动)
 */
void visca_parse_packet(ViscaPacket *packet, ViscaParsedCommand *cmd)
{
    uint8_t i;
    
    memset(cmd, 0, sizeof(ViscaParsedCommand));
    
    if (!packet->complete) {
        return;
    }
    
    cmd->camera_id = packet->receiver;
    cmd->command_type = packet->type;
    
    /* 检查是否是相对运动命令 (payload前两个字节是 0x06 0x03) */
    if (packet->payload_len >= 12 && 
        packet->payload[0] == VISCA_PTZ_RELATIVE_MOVE_H &&
        packet->payload[1] == VISCA_PTZ_RELATIVE_MOVE_L) {
        
        /* 相对运动命令 */
        cmd->is_relative_move = 1;
				if(packet->payload[0] == VISCA_PTZ_RELATIVE_MOVE_H && packet->payload[1] == VISCA_PTZ_RELATIVE_MOVE_L)
				{
						//执行相对运动命令
				}
				else
						return;
        
        /* 提取速度 (Byte2=Pan速度, Byte3=Tilt速度) */
        if (packet->payload_len >= 4) {
            cmd->pan_speed = packet->payload[2];
            cmd->tilt_speed = packet->payload[3];
        }

        /* 提取Pan位移 (Byte4-7, 共4字节，大端序) */
        if (packet->payload_len >= 8) {
            cmd->pan_displacement = (int32_t)(
                ((uint32_t)packet->payload[4] << 24) |
                ((uint32_t)packet->payload[5] << 16) |
                ((uint32_t)packet->payload[6] << 8)  |
                (uint32_t)packet->payload[7]);
        }
				if(packet->payload[4] & 0x80)
						cmd->pan_dir = VISCA_PTZ_LEFT;
				else
						cmd->pan_dir = VISCA_PTZ_RIGHT;
				
        /* 提取Tilt位移 (Byte8-11, 共4字节，大端序) */
        if (packet->payload_len >= 12) {
            cmd->tilt_displacement = (int32_t)(
                ((uint32_t)packet->payload[8] << 24) |
                ((uint32_t)packet->payload[9] << 16) |
                ((uint32_t)packet->payload[10] << 8)  |
                (uint32_t)packet->payload[11]);
        }
				if(packet->payload[8] & 0x80)
            cmd->tilt_dir = VISCA_PTZ_DOWN;
				else
						cmd->tilt_dir = VISCA_PTZ_UP;
        /* 保存原始参数 */
        cmd->param_cnt = packet->payload_len;
        for (i = 0; i < packet->payload_len && i < 12; i++) {
            cmd->param[i] = packet->payload[i];
        }
        
    } else if (packet->payload_len >= 2) {
        /* 标准命令解析 */
        cmd->is_relative_move = 0;
//        cmd->category = packet->payload[0];
//        cmd->operation = packet->payload[1];
        
        if (packet->payload_len > 2) {
            cmd->param_cnt = packet->payload_len - 2;
            for (i = 0; i < cmd->param_cnt && i < 8; i++) {
                cmd->param[i] = packet->payload[i + 2];
            }
        }
    }
}

/**
 * @brief 处理VISCA命令 (增强版，支持相对运动)
 */
void visca_process_command(ViscaParsedCommand *cmd)
{
    /* 处理相对运动命令 */
    if (cmd->is_relative_move) {
        /* 限制速度范围 */
        if (cmd->pan_speed < VISCA_PAN_SPEED_MIN) 
            cmd->pan_speed = VISCA_PAN_SPEED_MIN;
        if (cmd->pan_speed > VISCA_PAN_SPEED_MAX) 
            cmd->pan_speed = VISCA_PAN_SPEED_MAX;
        if (cmd->tilt_speed < VISCA_TILT_SPEED_MIN) 
            cmd->tilt_speed = VISCA_TILT_SPEED_MIN;
        if (cmd->tilt_speed > VISCA_TILT_SPEED_MAX) 
            cmd->tilt_speed = VISCA_TILT_SPEED_MAX;
        
        /* 处理相对运动 */
        // relative_move_handler(cmd->pan_speed, cmd->tilt_speed, 
        //                       cmd->pan_displacement, cmd->tilt_displacement);
        
        /* 示例：打印相对运动参数 */
        // printf("Relative Move: Pan Speed=%d, Tilt Speed=%d, ", 
        //        cmd->pan_speed, cmd->tilt_speed);
        // printf("Pan Disp=%d, Tilt Disp=%d\r\n", 
        //        cmd->pan_displacement, cmd->tilt_displacement);
//        return;
    }
    
    /* 原有命令处理 */
    switch (cmd->pan_dir) {
				case VISCA_PTZ_LEFT:
						/* 控制云台向左 */
						set_speed =(float) -cmd->pan_displacement/1024;
						break;
        case VISCA_PTZ_RIGHT:
            /* 控制云台向右 */
						set_speed = (float)cmd->pan_displacement/1024;
            break;
        case VISCA_PTZ_STOP:
            /* 停止云台 */
            break;
        case VISCA_PTZ_HOME:
            /* 回零 */
						break;
				default:break;
     }
		switch (cmd->tilt_dir) {
        case VISCA_PTZ_UP:
						/* 控制云台向上 */
						//set_speed = (float)cmd->tilt_displacement/1024;
            // ptz_control_up(cmd->param[0]);
            break;
        case VISCA_PTZ_DOWN:
            /* 控制云台向下 */
						//set_speed =(float) -cmd->tilt_displacement/1024;
            break;
				default:break;
    }
}

/**
 * @brief 发送VISCA相对运动命令
 * @param camera_id 相机ID (1-7)
 * @param pan_speed Pan速度 (0x01-0x18)
 * @param tilt_speed Tilt速度 (0x01-0x14)
 * @param pan_displacement Pan位移量 (有符号整数)
 * @param tilt_displacement Tilt位移量 (有符号整数)
 */
void visca_send_relative_move(uint8_t camera_id, 
                              uint8_t pan_speed, 
                              uint8_t tilt_speed,
                              int32_t pan_displacement, 
                              int32_t tilt_displacement)
{
    uint8_t cmd[15];
    uint8_t idx = 0;
    
    /* 限制速度范围 */
    if (pan_speed < VISCA_PAN_SPEED_MIN) pan_speed = VISCA_PAN_SPEED_MIN;
    if (pan_speed > VISCA_PAN_SPEED_MAX) pan_speed = VISCA_PAN_SPEED_MAX;
    if (tilt_speed < VISCA_TILT_SPEED_MIN) tilt_speed = VISCA_TILT_SPEED_MIN;
    if (tilt_speed > VISCA_TILT_SPEED_MAX) tilt_speed = VISCA_TILT_SPEED_MAX;
    
    /* 包头 */
    cmd[idx++] = 0x80 | (camera_id & 0x07);  /* 地址 */
    cmd[idx++] = 0x01;                       /* Command */
    
    /* 相对运动命令码 */
    cmd[idx++] = VISCA_PTZ_RELATIVE_MOVE_H;  /* 0x06 */
    cmd[idx++] = VISCA_PTZ_RELATIVE_MOVE_L;  /* 0x03 */
    
    /* 速度 */
    cmd[idx++] = pan_speed;
    cmd[idx++] = tilt_speed;
    
    /* Pan位移 (大端序，4字节) */
    cmd[idx++] = (pan_displacement >> 24) & 0xFF;
    cmd[idx++] = (pan_displacement >> 16) & 0xFF;
    cmd[idx++] = (pan_displacement >> 8) & 0xFF;
    cmd[idx++] = pan_displacement & 0xFF;
    
    /* Tilt位移 (大端序，4字节) */
    cmd[idx++] = (tilt_displacement >> 24) & 0xFF;
    cmd[idx++] = (tilt_displacement >> 16) & 0xFF;
    cmd[idx++] = (tilt_displacement >> 8) & 0xFF;
    cmd[idx++] = tilt_displacement & 0xFF;
    
    /* 终止符 */
    cmd[idx++] = VISCA_TERMINATOR;
    
    /* 发送命令 */
    visca_send_packet(cmd, idx);
}

/**
 * @brief 发送VISCA绝对位置命令
 * @param camera_id 相机ID (1-7)
 * @param pan_speed Pan速度 (0x01-0x18)
 * @param tilt_speed Tilt速度 (0x01-0x14)
 * @param pan_position Pan绝对位置 (0x0000-0x4000)
 * @param tilt_position Tilt绝对位置 (0x0000-0x4000)
 */
void visca_send_absolute_move(uint8_t camera_id,
                              uint8_t pan_speed,
                              uint8_t tilt_speed,
                              uint16_t pan_position,
                              uint16_t tilt_position)
{
    uint8_t cmd[11];
    uint8_t idx = 0;
    
    /* 限制速度范围 */
    if (pan_speed < VISCA_PAN_SPEED_MIN) pan_speed = VISCA_PAN_SPEED_MIN;
    if (pan_speed > VISCA_PAN_SPEED_MAX) pan_speed = VISCA_PAN_SPEED_MAX;
    if (tilt_speed < VISCA_TILT_SPEED_MIN) tilt_speed = VISCA_TILT_SPEED_MIN;
    if (tilt_speed > VISCA_TILT_SPEED_MAX) tilt_speed = VISCA_TILT_SPEED_MAX;
    
    /* 限制位置范围 (标准VISCA范围 0x0000-0x4000) */
    if (pan_position > 0x4000) pan_position = 0x4000;
    if (tilt_position > 0x4000) tilt_position = 0x4000;
    
    /* 包头 */
    cmd[idx++] = 0x80 | (camera_id & 0x07);
    cmd[idx++] = 0x01;
    
    /* 绝对位置命令码 */
    cmd[idx++] = VISCA_PTZ_ABSOLUTE_MOVE_H;  /* 0x02 */
    cmd[idx++] = VISCA_PTZ_ABSOLUTE_MOVE_L;  /* 0x00 */
    
    /* 速度 */
    cmd[idx++] = pan_speed;
    cmd[idx++] = tilt_speed;
    
    /* Pan位置 (2字节) */
    cmd[idx++] = (pan_position >> 8) & 0xFF;
    cmd[idx++] = pan_position & 0xFF;
    
    /* Tilt位置 (2字节) */
    cmd[idx++] = (tilt_position >> 8) & 0xFF;
    cmd[idx++] = tilt_position & 0xFF;
    
    /* 终止符 */
    cmd[idx++] = VISCA_TERMINATOR;
    
    visca_send_packet(cmd, idx);
}

/**
 * @brief 发送停止命令
 * @param camera_id 相机ID (1-7)
 */
void visca_send_stop(uint8_t camera_id)
{
    uint8_t cmd[] = {
        0x80 | (camera_id & 0x07),
        0x01,
        0x01,           /* 云台类别 */
        VISCA_PTZ_STOP, /* 停止操作码 */
        VISCA_TERMINATOR
    };
    
    visca_send_packet(cmd, sizeof(cmd));
}

/* 接收状态机状态 */
#define VISCA_STATE_IDLE             0       /* 空闲状态，等待包头 */
#define VISCA_STATE_HEADER           1       /* 接收包头 */
#define VISCA_STATE_PAYLOAD          2       /* 接收载荷 */
#define VISCA_STATE_TERMINATOR       3       /* 接收终止符 */

/* VISCA数据包超时时间(ms) */
#define VISCA_PACKET_TIMEOUT         50

/* 默认波特率 */
#define VISCA_DEFAULT_BAUDRATE       9600

/**
 * @brief 初始化环形队列
 */
void visca_ring_buffer_init(ViscaRingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->overflow_cnt = 0;
    memset((void*)rb->buffer, 0, VISCA_RING_BUFFER_SIZE);
}

/**
 * @brief 向环形队列写入一个字节
 */
bool visca_ring_buffer_push(ViscaRingBuffer *rb, uint8_t data)
{
    uint32_t next_head = (rb->head + 1) % VISCA_RING_BUFFER_SIZE;
    
    if (next_head == rb->tail) {
        rb->overflow_cnt++;
        return false;
    }
    
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    
    return true;
}

/**
 * @brief 从环形队列读取一个字节
 */
bool visca_ring_buffer_pop(ViscaRingBuffer *rb, uint8_t *data)
{
    if (rb->head == rb->tail) {
        return false;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % VISCA_RING_BUFFER_SIZE;
    
    return true;
}

/**
 * @brief 获取环形队列中有效数据数量
 */
uint32_t visca_ring_buffer_available(ViscaRingBuffer *rb)
{
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return VISCA_RING_BUFFER_SIZE - rb->tail + rb->head;
    }
}

/**
 * @brief 清空环形队列
 */
void visca_ring_buffer_flush(ViscaRingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

/**
 * @brief 初始化VISCA接收器
 */
void visca_receiver_init(ViscaReceiver *vr, uint8_t address)
{
    visca_ring_buffer_init(&vr->rx_ring);
    visca_ring_buffer_init(&vr->frame_ring);
    
    memset(&vr->current_packet, 0, sizeof(ViscaPacket));
    vr->rx_state = VISCA_STATE_IDLE;
    vr->last_byte_time = 0;
    vr->address = address & 0x1F;  /* 地址范围0-31 */
}


/**
 * @brief 检查接收地址是否匹配
 */
static bool visca_address_match(ViscaReceiver *vr, uint8_t address)
{
    /* 广播地址或匹配本机地址 */
    return (address == VISCA_ADDRESS_BROADCAST) || (address == vr->address);
}

/**
 * @brief 接收VISCA数据包(状态机实现)
 */
void visca_receive_byte(ViscaReceiver *vr, uint8_t data)
{
    uint32_t current_time = _micros()/1000;
    
    /* 超时检测：如果超时则重置状态机 */
    if (vr->rx_state != VISCA_STATE_IDLE && 
        (current_time - vr->last_byte_time) > VISCA_PACKET_TIMEOUT) {
        vr->rx_state = VISCA_STATE_IDLE;
        vr->current_packet.payload_len = 0;
    }
    
    vr->last_byte_time = current_time;
    
    /* 先存入原始环形队列（可选，用于调试） */
    visca_ring_buffer_push(&vr->rx_ring, data);
    
    /* 状态机处理 */
    switch (vr->rx_state) {
        case VISCA_STATE_IDLE:
            /* 等待包头: 最高位为1的是地址字节 */
            if (data & 0x80) {
                vr->current_packet.sender = (data >> 3) & 0x0F;
                vr->current_packet.receiver = data & 0x07;
                vr->current_packet.payload_len = 0;
                vr->rx_state = VISCA_STATE_HEADER;
            }
            break;
            
        case VISCA_STATE_HEADER:
						/* 应该判断 data 的最高位是否为0，且类型为0x01/0x09/0x0A/0x0B */
						if ((data & 0x80) == 0 && (data == 0x01 || data == 0x09 || data == 0x0A || data == 0x0B)) {
                vr->current_packet.type = data;
                vr->rx_state = VISCA_STATE_PAYLOAD;
            } else {
                /* 意外的数据，重置 */
                vr->rx_state = VISCA_STATE_IDLE;
            }
            break;
            
        case VISCA_STATE_PAYLOAD:
					  if(vr->current_packet.payload_len >= 12) {
								/* 接收有效载荷 */
								if (data == VISCA_TERMINATOR) {
										//存入终止符
										vr->current_packet.payload[vr->current_packet.payload_len++] = data;
										/* 收到终止符，数据包接收完成 */
										vr->current_packet.complete = 1;
										vr->rx_state = VISCA_STATE_TERMINATOR;
										
										/* 将完整包存入帧队列 */
										uint8_t *p = (uint8_t*)&vr->current_packet;
										for (int i = 0; i < (3 + vr->current_packet.payload_len + 1); i++) {
												visca_ring_buffer_push(&vr->frame_ring, p[i]);
										}
								}
								else
										return;
            } else {
                /* 存储载荷数据 */
                if (vr->current_packet.payload_len < VISCA_MAX_PACKET_SIZE - 4) {
                    vr->current_packet.payload[vr->current_packet.payload_len++] = data;
                }
            }
            break;
            
        case VISCA_STATE_TERMINATOR:
            /* 已经完成，等待下一个包 */
            vr->rx_state = VISCA_STATE_IDLE;
            /* 可能还有额外的终止符，忽略 */
            break;
    }
}

/**
 * @brief 从队列中获取完整数据包
 */
bool visca_get_packet(ViscaReceiver *vr, ViscaPacket *packet)
{
    uint8_t data;
    uint8_t idx = 0;
    uint8_t buffer[VISCA_MAX_PACKET_SIZE];
    
    /* 从帧队列中读取完整包 */
    while (visca_ring_buffer_pop(&vr->frame_ring, &data)) {
        buffer[idx++] = data;
        
        /* VISCA包长度可变，通过终止符判断结束 */
        if (idx >= 15 && data == VISCA_TERMINATOR) {
            /* 找到终止符，解析数据包 */
            if (idx >= 4) {  /* 至少需要包头+终止符 */
//                packet->sender = (buffer[0] >> 3) & 0x0F;
//                packet->receiver = buffer[0] & 0x07;
//                packet->type = buffer[1];
							  //前面接收数据包时已经处理过了
							  packet->sender = buffer[0];
                packet->receiver = buffer[1];
                packet->type = buffer[2];
                packet->payload_len = idx-3;
                if (packet->payload_len > 0) {
                    memcpy(packet->payload, &buffer[3], packet->payload_len);
                }
                packet->complete = 1;
                return true;
            }
            break;
        }
        
        /* 防止缓冲区溢出 */
        if (idx >= VISCA_MAX_PACKET_SIZE) {
            break;
        }
    }
    
    return false;
}

///**
// * @brief 解析VISCA命令
// */
//void visca_parse_packet(ViscaPacket *packet, ViscaParsedCommand *cmd)
//{
//    memset(cmd, 0, sizeof(ViscaParsedCommand));
//    
//    if (!packet->complete) {
//        return;
//    }
//    
//    cmd->camera_id = packet->receiver;
//    cmd->command_type = packet->type;
//    
//    /* 解析命令和参数 */
//    if (packet->payload_len >= 2) {
//        cmd->category = packet->payload[0];
//        cmd->operation = packet->payload[1];
//        
//        if (packet->payload_len > 2) {
//            cmd->param_cnt = packet->payload_len - 2;
//            memcpy(cmd->param, &packet->payload[2], cmd->param_cnt);
//        }
//    }
//}
//
///**
// * @brief 处理VISCA命令
// */
//void visca_process_command(ViscaParsedCommand *cmd)
//{
//    /* 检查命令类别 */
//    switch (cmd->category) {
//        case VISCA_CATEGORY_PTZ:
//            /* 云台控制 */
//            switch (cmd->operation) {
//                case VISCA_PTZ_UP:
//                    /* 控制云台向上 */
//                    // ptz_control_up(cmd->param[0]);  /* 速度参数 */
//								    set_speed = cmd->param[0];
//                    break;
//                case VISCA_PTZ_DOWN:
//                    /* 控制云台向下 */
//                    break;
//                case VISCA_PTZ_LEFT:
//                    /* 控制云台向左 */
//                    break;
//                case VISCA_PTZ_RIGHT:
//                    /* 控制云台向右 */
//                    break;
//                case VISCA_PTZ_STOP:
//                    /* 停止云台 */
//                    break;
//                case VISCA_PTZ_HOME:
//                    /* 回零 */
//                    break;
//            }
//            break;
//            
//        case VISCA_CATEGORY_ZOOM:
//            /* 变焦控制 */
//            switch (cmd->operation) {
//                case VISCA_ZOOM_TELE:
//                    /* 拉近变焦 */
//                    // zoom_tele(cmd->param[0]);
//                    break;
//                case VISCA_ZOOM_WIDE:
//                    /* 拉远变焦 */
//                    // zoom_wide(cmd->param[0]);
//                    break;
//                case VISCA_ZOOM_STOP:
//                    /* 停止变焦 */
//                    break;
//                case VISCA_ZOOM_DIRECT:
//                    /* 直接变焦到指定位置 */
//                    if (cmd->param_cnt >= 2) {
//                        uint16_t position = (cmd->param[0] << 8) | cmd->param[1];
//                        // zoom_set_position(position);
//                    }
//                    break;
//            }
//            break;
//            
//        case VISCA_CATEGORY_FOCUS:
//            /* 聚焦控制 */
//            break;
//            
//        case VISCA_CATEGORY_IRIS:
//            /* 光圈控制 */
//            break;
//            
//        case VISCA_CATEGORY_POWER:
//            /* 电源控制 */
//            break;
//    }
//}

/**
 * @brief 发送VISCA数据包
 */
void visca_send_packet(uint8_t *data, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        usart_data_transmit(USART0, data[i]);
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
    }
}

/**
 * @brief 发送应答包(ACK)
 */
void visca_send_ack(ViscaReceiver *vr, ViscaPacket *packet)
{
    uint8_t response[3];
    
    response[0] = 0x90 | (vr->address << 3) | 0x01;  /* 返回地址 */
    response[1] = 0x40 | (packet->type & 0x0F);       /* ACK标志 */
    response[2] = VISCA_TERMINATOR;
    
    visca_send_packet(response, 3);
}

/**
 * @brief 发送完成包(Completion)
 */
void visca_send_completion(ViscaReceiver *vr, ViscaPacket *packet)
{
    uint8_t response[4];
    
    response[0] = 0x90 | (vr->address << 3) | 0x01;
    response[1] = 0x50 | (packet->type & 0x0F);
    response[2] = 0x00;  /* 无错误 */
    response[3] = VISCA_TERMINATOR;
    
    visca_send_packet(response, 4);
}

/**
 * @brief 发送错误包
 */
void visca_send_error(ViscaReceiver *vr, ViscaPacket *packet, uint8_t error_code)
{
    uint8_t response[4];
    
    response[0] = 0x90 | (vr->address << 3) | 0x01;
    response[1] = 0x60 | (packet->type & 0x0F);
    response[2] = error_code;
    response[3] = VISCA_TERMINATOR;
    
    visca_send_packet(response, 4);
}

