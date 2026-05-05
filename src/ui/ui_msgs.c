#include "ui.h"
#include "ui_msgs.h"

void ui_msg_send(enum XPTOUCH_MESSAGE msg, unsigned long long data1, unsigned long long data2)
{
    struct XPTOUCH_MESSAGE_DATA eventData;
    eventData.data = data1;
    eventData.data2 = data2;
    lv_msg_send(msg, &eventData);
}

