#include <CoreFoundation/CoreFoundation.h>

void conn_read_callback(CFSocketRef s,
						CFSocketCallBackType callbackType,
						CFDataRef address,
						void *data,
						void *info);

void conn_accept_callback(CFSocketRef s,
					 	  CFSocketCallBackType callbackType,
					 	  CFDataRef address,
					 	  void *data,
					 	  void *info);