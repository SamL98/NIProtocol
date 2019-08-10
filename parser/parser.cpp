#include "parser.h"
#include <stdio.h>

uint32_t
btn_code_to_btn_num(uint32_t code)
{
	uint32_t r,c;
	r = code / 4;
	c = code % 4;
	return (3 - r) * 4 + c + 1;
}

void 
parse_wheel_msg(uint32_t *data,
				mk2_msg *msg)
{
	msg->type = WheelType;
	msg->msg.wheel_msg.btn = *data;
	msg->msg.wheel_msg.state = *(data+1);
}

void
parse_button_msg(uint32_t *data,
				 mk2_msg *msg)
{
	msg->type = ButtonType;
	msg->msg.button_msg.btn = btn_code_to_btn_num(*data);
	msg->msg.button_msg.pressure = *(data+2);
}

mk2_msg *
parse_packet(char *packet, 
			 size_t packetLen,
			 size_t *nmsgs)
{
	uint32_t *fields,
			 ctrl;
	size_t 	 msgLen,
			 msgStructLen,
			 i;
	void	 (*parse_fn)(uint32_t*, mk2_msg*);
	mk2_msg  *msgs;
	MsgType  msgType;

	msgs = NULL;
	*nmsgs = 0;

	fields = (uint32_t *)packet;
	ctrl = *fields;
	fields += 3;
	*nmsgs = *fields;
	fields += 1;

	if (ctrl == kWheelCtrl1 || ctrl == kWheelCtrl2) {
		msgType = WheelType;
		msgLen = kWheelMsgLen;
		msgStructLen = sizeof(mk2_wheel_msg);
		parse_fn = &parse_wheel_msg;
	}
	else if (ctrl == kButtonsCtrl) {
		msgType = ButtonType;
		msgLen = kButtonMsgLen;
		msgStructLen = sizeof(mk2_button_msg);
		parse_fn = &parse_button_msg;
	}
	else {
		printf("Unrecognized control code: %x\n", ctrl);
		return NULL;
	}

	if (!(msgs = (mk2_msg *)malloc(msgStructLen * (*nmsgs)))) {
		printf("Couldn't malloc %lu msgs\n", *nmsgs);
		return NULL;
	}
	
	for (i=0; i<(*nmsgs); i++) {
		parse_fn(fields, &msgs[i]);
		fields += msgLen / 4;
	}

	return msgs;
}
