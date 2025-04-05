#ifndef __USART_H
#define __USART_H

void usart1_init(uint32_t baudrate);
void usart2_init(uint32_t baudrate);

void usart_send(uint8_t *data, uint8_t length);

#endif
