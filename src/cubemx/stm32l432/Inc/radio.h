/* radio.h:
 *
 *  radio include file
 */
#ifndef __RADIO_H__
#define __RADIO_H__

/*
 *  radio functions
 */

void radio_init();
int radio_send_data(char *data, uint8_t length);
int radio_recv_data(char *buffer, uint8_t length);

#endif /* __RADIO_H__ */
