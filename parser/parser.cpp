#include "parser.h"
#include <stdio.h>

uint32_t
btn_code_to_num(uint32_t code)
{
	// Thinking of the buttons as a 2d array,
	// the button code is the linear index into that array, i.e.
	//
	// 0, 1, 2, 3
	// 4, 5, 6, 7
	// 8, 9, 10, 11
	// 12, 13, 14, 15
	//
	// We want to convert that to the button number that appears on the MK2, i.e.
	//
	// 13, 14, 15, 16
	// 9, 10, 11, 12
	// 5, 6, 7, 8
	// 1, 2, 3, 4
	//
	// Do this by taking the linear index of the code when reflected
	// about the y-axis and adding 1.

	uint32_t r,c;
	r = code / 4;
	c = code % 4;
	return (3 - r) * 4 + c + 1;
}

uint32_t
btn_num_to_code(uint32_t num)
{
	// Perform the inverse of btn_num_to_btn_code

	uint32_t r,c;
	num -= 1;
	r = 3 - num / 4;
	c = num % 4;
	return r * 4 + c;
}

void 
parse_wheel_msg(uint32_t *data,
				mk2_msg *msg)
{
	// The layout of a wheel msg is:
	// 		btn	   |    state
	//	--4 bytes-- -- 4 bytes--
	//
	// Where btn is 0 when scrolling and 11 when clicking.
	// When scrolling, state is 1 when scrolling clockwise and -1 when counterclockwise.
	// When clicking, the state is 1 when down and 0 when up.

	msg->type = WheelType;
	msg->msg.wheel_msg.btn = *data;
	msg->msg.wheel_msg.state = *(data+1);
}

void
parse_button_msg(uint32_t *data,
				 mk2_msg *msg)
{
	// The layout of a button msg is:
	//		btn	   |  unknown  | pressure
	//	--4 bytes-- --4 bytes-- --4 bytes--
	//
	// Where btn is the button code described in `btn_code_to_btn_num`
	// and pressure is the amount of pressure being applied to the button.

	msg->type = ButtonType;
	msg->msg.button_msg.btn = btn_code_to_num(*data);
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

	// The header of a packet is formatted as such:
	//	  ctrl    |  counter  |  unknown   |   nmsgs
	// --4 bytes-- --4 bytes-- --4 bytes--  --4 bytes--
	//
	// Where ctrl specified the packet type (wheel or button)
	// and nmsgs specifies the number of messages in the packet.
	// We don't care about the counter.
	
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
