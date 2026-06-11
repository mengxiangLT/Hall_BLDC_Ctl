//#include "pelco_d.h"
//#include <string.h>
#include "includes.h"

/* 接收缓冲区 */
#define PELCO_D_BUFFER_SIZE   16
static uint8_t rx_buffer[PELCO_D_BUFFER_SIZE];
static uint8_t rx_index = 0;
static uint32_t last_byte_time = 0;
static uint8_t frame_received = 0;

/* 设备地址 (可通过拨码开关或配置设置) */
static uint8_t device_address = 0x01;

/* 定时器相关 (用于帧超时检测) */

/**
 * @brief 初始化Pelco-D串口 (USART0为例)
 * @param baudrate 波特率，通常为2400
 */
#if 1
void pelco_d_uart_init(uint32_t baudrate)
{
//    /* 使能时钟 */
//    rcu_periph_clock_enable(RCU_GPIOA);
//    rcu_periph_clock_enable(RCU_USART0);
//    
//    /* 配置GPIO: PA9(TX), PA10(RX) */
//    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
//    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
//    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
//    
//    /* 配置USART0参数 */
//    usart_deinit(USART0);
//    usart_baudrate_set(USART0, baudrate);
//    usart_word_length_set(USART0, USART_WL_8BIT);
//    usart_stop_bit_set(USART0, USART_STB_1BIT);
//    usart_parity_config(USART0, USART_PM_NONE);
//    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
//    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
//    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
//    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
//    
//    /* 使能接收中断 */
//    nvic_irq_enable(USART0_IRQn, 1, 0);
//    usart_interrupt_enable(USART0, USART_INT_RBNE);
//    
//    /* 使能USART */
//    usart_enable(USART0);
    
    /* 初始化接收缓冲区 */
    memset(rx_buffer, 0, sizeof(rx_buffer));
    rx_index = 0;
    frame_received = 0;
    last_byte_time = _micros();
}
#endif
/**
 * @brief 检查Pelco-D帧是否完整
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return 1=有效帧, 0=无效
 */
uint8_t pelco_d_check_frame(uint8_t *data, uint8_t len)
{
    PelcoD_Command *cmd;
    
    if (len < 7) {
        return 0;  /* 至少7字节 */
    }
    
    cmd = (PelcoD_Command *)data;
    
    /* 检查同步字节 */
    if (cmd->sync_byte != 0xFF) {
        return 0;
    }
    
    /* 检查校验和 */
    if (!pelco_d_checksum_check(data, len)) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief 计算并验证校验和
 * @param data 数据缓冲区
 * @param len 数据长度(至少7字节)
 * @return 1=校验通过, 0=校验失败
 */
uint8_t pelco_d_checksum_check(uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t sum = 0;
    uint8_t received_checksum;
    
    if (len < 7) {
        return 0;
    }
    
    received_checksum = data[6];
    
    /* 计算字节2到字节6的累加和 */
    for (i = 1; i < 6; i++) {
        sum += data[i];
    }
    
    return (sum == received_checksum);
}

/**
 * @brief 解析Pelco-D命令
 * @param cmd Pelco-D命令结构体指针
 * @param result 解析结果指针
 */
void pelco_d_parse_command(PelcoD_Command *cmd, PelcoD_ParseResult *result)
{
    /* 清零结果 */
    memset(result, 0, sizeof(PelcoD_ParseResult));
    
    result->valid = 1;
    result->address = cmd->address;
    result->is_broadcast = (cmd->address == 0xFF);
    
    /* 提取速度值 */
    result->pan_speed = cmd->data1;
    result->tilt_speed = cmd->data2;
    
    /* 解析命令码1 (CMD1) */
	  /* 镜头控制 */
    if (cmd->cmd1 & 0x01) result->auto_scan = 1;     /* 焦距拉近,镜头焦点向近处调节 */
    if (cmd->cmd1 & 0x02) result->iris_open = 1;     /* 光圈打开 (Iris Open)	增大光圈，画面变亮 */
    if (cmd->cmd1 & 0x04) result->iris_close = 1;    /* 光圈关 */
    if (cmd->cmd1 & 0x08) result->flip = 1;					 /* 相机打开/关闭 (Camera On/Off)	控制摄像机电源，结合Sense位使用 */
    if (cmd->cmd1 & 0x10) result->focus_near = 1;    /* 聚焦近 */
    if (cmd->cmd1 & 0x20) result->focus_far = 1;     /* 聚焦远 */
    if (cmd->cmd1 & 0x40) result->zoom_in = 1;       /* 变倍+ (Tele) */
    if (cmd->cmd1 & 0x80) result->zoom_out = 1;      /* 变倍- (Wide) */
    
    /* 解析命令码2 (CMD2) */
    /* 云台控制 - 使用数据字节作为速度 */
    if (cmd->cmd2 & 0x02) result->pan_right = 1;
    if (cmd->cmd2 & 0x04) result->pan_left = 1;
    if (cmd->cmd2 & 0x08) result->tilt_up = 1;
    if (cmd->cmd2 & 0x10) result->tilt_down = 1;
		
		
    
    /* 预置位命令 (CMD1中特殊位) */
    if ((cmd->cmd1 & 0x01) && (cmd->cmd2 & 0x00)) {
        /* 设置预置位: CMD1=0x03, CMD2=0x00 */
        if (cmd->cmd1 == 0x03) {
            result->set_preset = 1;
            result->preset_id = cmd->data2;
        }
        /* 清除预置位: CMD1=0x05, CMD2=0x00 */
        else if (cmd->cmd1 == 0x05) {
            result->clear_preset = 1;
            result->preset_id = cmd->data2;
        }
        /* 调用预置位: CMD1=0x07, CMD2=0x00 */
        else if (cmd->cmd1 == 0x07) {
            result->call_preset = 1;
            result->preset_id = cmd->data2;
        }
    }
    
    /* 复位命令 */
    if (cmd->cmd1 == 0x0F && cmd->cmd2 == 0x00) {
        result->reset = 1;
    }
}

/**
 * @brief 处理解析后的Pelco-D命令
 * @param result 解析结果指针
 */
void pelco_d_process_command(PelcoD_ParseResult *result)
{
    /* 检查设备地址（广播地址0xFF或匹配本机地址） */
    if (!result->is_broadcast && result->address != device_address) {
        return;  /* 地址不匹配，忽略 */
    }
    
    /* 处理云台控制 */
    if (result->pan_right) {
        /* 控制云台右转 */
        /* 根据result->pan_speed设置速度，0x3F=满速，0xFF=最高速 */
        uint8_t speed = (result->pan_speed == 0xFF) ? 0x3F : result->pan_speed;
        // motor_control_pan(speed, DIRECTION_RIGHT);
    }
    
    if (result->pan_left) {
        uint8_t speed = (result->pan_speed == 0xFF) ? 0x3F : result->pan_speed;
        // motor_control_pan(speed, DIRECTION_LEFT);
    }
    
    if (result->tilt_up) {
        uint8_t speed = (result->tilt_speed == 0xFF) ? 0x3F : result->tilt_speed;
        // motor_control_tilt(speed, DIRECTION_UP);
    }
    
    if (result->tilt_down) {
        uint8_t speed = (result->tilt_speed == 0xFF) ? 0x3F : result->tilt_speed;
        // motor_control_tilt(speed, DIRECTION_DOWN);
    }
    
    /* 处理镜头控制 */
    if (result->zoom_in) {
        // lens_control_zoom(ZOOM_IN, result->pan_speed);
    }
    
    if (result->zoom_out) {
        // lens_control_zoom(ZOOM_OUT, result->pan_speed);
    }
    
    if (result->focus_near) {
        // lens_control_focus(FOCUS_NEAR);
    }
    
    if (result->focus_far) {
        // lens_control_focus(FOCUS_FAR);
    }
    
    if (result->iris_open) {
        // lens_control_iris(IRIS_OPEN);
    }
    
    if (result->iris_close) {
        // lens_control_iris(IRIS_CLOSE);
    }
    
    /* 处理预置位 */
    if (result->set_preset) {
        // preset_save(result->preset_id);
    }
    
    if (result->call_preset) {
        // preset_recall(result->preset_id);
    }
    
    if (result->clear_preset) {
        // preset_clear(result->preset_id);
    }
    
    /* 处理其他功能 */
    if (result->auto_scan) {
        // auto_scan_start();
    }
    
    if (result->reset) {
        // system_reset();
    }
}

/**
 * @brief 发送响应命令（如需要）
 * @param address 设备地址
 * @param data 数据缓冲区
 * @param len 数据长度
 */
void pelco_d_send_response(uint8_t address, uint8_t *data, uint8_t len)
{
    uint8_t i;
    
    for (i = 0; i < len; i++) {
        usart_data_transmit(USART0, data[i]);
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
    }
}

/**
 * @brief 接收单个字节(在串口中断中调用)
 * @param data 接收到的字节
 */
void pelco_d_receive_byte(uint8_t data)
{
    uint32_t current_time = _micros();
    
    /* 帧超时检测: 如果超过10ms没有新数据，认为是新的一帧 */
    if ((current_time - last_byte_time) > 10) {
        rx_index = 0;  /* 重置缓冲区 */
    }
    
    last_byte_time = current_time;
    
    /* 接收数据 */
    if (rx_index < PELCO_D_BUFFER_SIZE) {
        rx_buffer[rx_index++] = data;
    }
    
    /* 检查是否收到完整帧 */
    if (rx_index >= 7) {
        /* 验证帧 */
        if (pelco_d_check_frame(rx_buffer, rx_index)) {
            PelcoD_Command *cmd = (PelcoD_Command *)rx_buffer;
            PelcoD_ParseResult result;
            
            /* 解析命令 */
            pelco_d_parse_command(cmd, &result);
            
            /* 处理命令 */
            if (result.valid) {
                pelco_d_process_command(&result);
            }
        }
        
        /* 重置缓冲区等待下一帧 */
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
}

/**
 * @brief 设置设备地址
 * @param addr 新的设备地址(1-255)
 */
void pelco_d_set_address(uint8_t addr)
{
    if (addr > 0) {
        device_address = addr;
    }
}

/**
 * @brief 获取当前设备地址
 * @return 设备地址
 */
uint8_t pelco_d_get_address(void)
{
    return device_address;
}

