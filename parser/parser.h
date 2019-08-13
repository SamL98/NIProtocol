#include <stdlib.h>

#define kWheelCtrl 0x3774e00
#define kButtonCtrl 0x3734e00
#define kPadCtrl 0x3504e00

enum MsgType { ButtonType, PadType };

typedef struct {
	uint32_t btn;
	int32_t  state;
} mk2_button_msg;

typedef struct {
	uint32_t btn;
	uint32_t pressure;
} mk2_pad_msg;

typedef struct {
	enum MsgType type;
	union {
		mk2_pad_msg 	pad_msg;
		mk2_button_msg  button_msg;
	} msg;
} mk2_msg;

uint32_t btn_code_to_num(uint32_t code);
uint32_t btn_num_to_code(uint32_t num);
int parse_packet(char *packet, size_t packetLen, mk2_msg *msg);
void display_msg(mk2_msg msg);
