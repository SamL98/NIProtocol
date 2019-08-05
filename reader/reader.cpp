#include "client/client.h"
#include <stdio.h>
#include <stdlib.h>

#define kWheelCtrl 0x7703
#define kButtonsCtrl 0x5003
#define kWheelBtn 0xb
#define kOneBtn 0xc
#define kTwoBtn 0xd
#define kFiveBtn 0xe
#define kFourBtn 0xf
#define kFiveBtn 0x8
#define kSixBtn 0x9
#define kSevenBtn 0xa
#define kEightBtn 0xb
#define kNineBtn 0x4
#define kTenBtn 0x5
#define kElevenBtn 0x6
#define kTwelveBtn 0x7
#define kThirteenBtn 0x0
#define kFourteenBtn 0x1
#define kFifteenBtn 0x2
#define kSixteenBtn 0x3

struct wheel_msg_t {
	uint32_t btn;
	int32_t  state;
}

struct wheel_cmd_t {
	uint32_t 		  counter;
	uint32_t 		  unk;
	uint32_t 		  nmsgs;
	struct wheel_msg_t *msgs;
};

struct button_msg_t {
	uint32_t unk;
	uint32_t btn;
	uint32_t pressure;
}

struct buttons_cmd_t {
	uint32_t 			counter;
	uint32_t 			unk;
	uint32_t 			nmsgs;
	struct button_msg_t *msgs;
};

void handleWheelCmd(struct wheel_cmd_t cmd)
{

}

void handleButtonsCmd(struct buttons_cmd_t cmd)
{

}

struct wheel_cmd_t parse_wheel_cmd(uint32_t *data)
{
	struct wheel_cmd_t cmd;
	struct wheel_msg_t *msgs;
	size_t 			   i;

	cmd.counter = *data;
	cmd.nmsgs = *(data+8);

	msgs = malloc(cmd.nmsgs * sizeof(wheel_msg_t))
	if (!msgs) {
		printf("Couldnt malloc %d msgs\n", cmd.nmsgs);
		return NULL;
	}

	data += 12;

	for (i=0; i<cmd.nmsgs; i++){
		struct wheel_msg_t msg;
		msg.btn = *(data);
		msg.state = *(data + 4);
		msgs[i] = msg;

		data += sizeof(wheel_msg_t);
	}

	cmd.msgs = msgs;

	return cmd;
}

struct buttons_cmd_t parse_buttons_cmd(uint32_t *data)
{
	struct buttons_cmd_t cmd;
	struct button_msg_t  *msgs;
	size_t 			     i;

	cmd.counter = *data;
	cmd.nmsgs = *(data+8);

	msgs = malloc(cmd.nmsgs * sizeof(buttn_msg_t))
	if (!msgs) {
		printf("Couldnt malloc %d msgs\n", cmd.nmsgs);
		return NULL;
	}

	data += 12;

	for (i=0; i<cmd.nmsgs; i++){
		struct button_msg_t msg;
		msg.btn = *data;
		msg.pressure = *(data + 8);
		msgs[i] = msg;
		
		data += sizeof(button_msg_t);
	}

	cmd.msgs = msgs;

	return cmd;
}

void hardware_callback(uint32_t *data, size_t length)
{
	uint32_t ctrl = *data;

	if (ctrl == kWheelCtrl) 
	{
		struct wheel_cmd_t wheel_cmd = parse_wheel_cmd(data+4);
		if (!wheel_cmd) {
			printf("Couldn't instantiate wheel command\n");
			return;
		}

		handleWheelCmd(wheel_cmd);
		free(wheel_cmd.msgs);
	}
	else if (ctrl == kButtonsCtrl) 
	{
		struct buttons_cmd_t buttons_cmd = parse_buttons_cmd(data+4);
		if (!wheel_cmd) {
			printf("Couldn't instantiate buttons command\n");
			return;
		}

		handleButtonscmd(buttons_cmd);
		free(buttons_cmd.msgs);
	}
	else
		printf("Unrecognized control code: %x", ctrl);
}

int main() 
{
	registerCallback((maschine_callback)hardware_callback);
}