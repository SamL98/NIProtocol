#include <CoreFoundation/CoreFoundation.h>
#include <nimessenger.h>
#include <nihandshaker.h>

void
setupMK2(CFMessagePortRef reqPort)
{
    button_data_t button_data;
    screen_data_t screen_data;
    size_t        i;
    FILE          *fp;  

    initButtonData(&button_data);
    setPadColor(&button_data, 1, (char)255, (char)0, (char)0);

    initScreenData(&screen_data);
    readScreenDataFromFile(&screen_data, "initials_bitwise.bin");

    sendButtonDataMsg(reqPort, button_data);
    sendScreenDataMsg(reqPort, screen_data);
}

int main(int argc, const char * argv[]) {
    CFMessagePortRef reqPort,
                     notifPort;

    if (!(reqPort = doHandshake(NULL))) {
        printf("Couldn't obtain request port from handshake\n");
        return 1;
    }

    printf("Finished handshake\n");
    setupMK2(reqPort);
    printf("Performed initial MK2 setup\n");

    if (!(notifPort = createNotificationPort(kSLBootstrapPortName, 
                                             (CFMessagePortCallBack)bootstrap_notif_port_callback,
                                             (void *)reqPort))) {
        printf("Couldn't create SL bootstrap notification port\n");
        return 1;
    }

    printf("Created %s bootstrap port\n", kSLBootstrapPortName);
    CFMessagePortSetDispatchQueue(notifPort,
								  dispatch_get_main_queue());

    CFRunLoopRun();

    return 0;
}
