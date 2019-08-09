#include <stdlib.h>

#define kWheelCtrl 0x7703
#define kButtonsCtrl 0x5003
#define kWheelMsgLen 8
#define kButtonMsgLen 12

enum MsgType { WheelType, ButtonType };

typedef struct {
	uint32_t btn;
	int32_t  state;
} mk2_wheel_msg;

typedef struct {
	uint32_t btn;
	uint32_t pressure;
} mk2_button_msg;

typedef struct {
	enum MsgType type;
	union {
		mk2_button_msg button_msg;
		mk2_wheel_msg  wheel_msg;
	} msg;
} mk2_msg;

mk2_msg *parse_packet(char *packet, size_t packetLen, size_t *nmsgs);
