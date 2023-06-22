#include <stdint.h>
#include <stdbool.h>

#define NODES_MAX_NUM 8

void system_status_init();

bool sys_stat_get_fire_detected();
bool sys_stat_get_pump();
bool sys_stat_get_alarm();
bool sys_stat_get_online(uint8_t node_id);
int sys_stat_get_temp(uint8_t node_id);
int sys_stat_get_smoke(uint8_t node_id);
int sys_stat_get_fire(uint8_t node_id);
int64_t sys_stat_get_last_message_time(uint8_t node_id);

void sys_stat_set_fire_detected(bool fire_detected);
void sys_stat_set_pump(bool pump_stat);
void sys_stat_set_alarm(bool alarm_stat);
void sys_stat_set_online(uint8_t node_id, bool online);
void sys_stat_set_temp(uint8_t node_id, int temp);
void sys_stat_set_smoke(uint8_t node_id, int smoke);
void sys_stat_set_fire(uint8_t node_id, int fire);
void sys_stat_set_last_message_time(uint8_t node_id, int64_t time);