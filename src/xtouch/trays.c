
#include <stdint.h>

char tray_types[5][16];
uint16_t tray_temps[5];
uint64_t tray_status[5];

char* get_tray_type(uint8_t tray_id){
    return tray_types[tray_id+1];
}

void set_tray_type(uint8_t tray_id, char* tray_type){
    strcpy(tray_types[tray_id+1],tray_type);
}


uint16_t get_tray_temp(uint8_t tray_id){
    return tray_temps[tray_id+1];
}
void set_tray_temp(uint8_t tray_id, uint16_t tray_temp){
    tray_temps[tray_id] = tray_temp;
}


uint64_t get_tray_status(uint8_t tray_id){
    return tray_status[tray_id];
}
void set_tray_status(uint8_t tray_id, uint64_t tray_state){
    tray_status[tray_id] = tray_state;
}
