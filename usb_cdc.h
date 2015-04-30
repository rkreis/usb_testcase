#ifndef S2U_USB_CDC_H
#define S2U_USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

void usb_init(void);
void usb_poll(void);
void usb_rx(char *data, int len);
void usb_tx(char *data, int len);

#ifdef __cplusplus
}
#endif

#endif
