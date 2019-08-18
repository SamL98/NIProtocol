#include "bonjour_callbacks.h"

void register_callback(CFNetServiceRef service, 
					   CFStreamError *error, 
					   void *info)
{
	printf("Bonjour service successfully registered\n");
}