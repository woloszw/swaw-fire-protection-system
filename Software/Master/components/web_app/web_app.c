#include <stdio.h>
#include <stdlib.h>
#include <string.h> //Requires by memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "led_strip.h"
#include <esp_http_server.h>
#include "esp_log.h"
#include "web_app.h"
#include "net_connection.h"
#include "system_status.h"

esp_err_t get_req_handler(httpd_req_t *req);
esp_err_t pump_on_handler(httpd_req_t *req);
esp_err_t pump_off_handler(httpd_req_t *req);
esp_err_t alarm_on_handler(httpd_req_t *req);
esp_err_t alarm_off_handler(httpd_req_t *req);

static const char *TAG = "web_app"; // TAG for debug

extern led_strip_handle_t led_strip;

char page_src[] = "<!DOCTYPE html> \
<html> \
<head> \
<style type=\"text/css\"> \
html {  font-family: Arial;  display: inline-block;  margin: 0px auto;  text-align: center;} \
h1{  color: #070812;  padding: 2vh;} \
.button {  display: inline-block;  background-color: #b30000; //red color  border: none;  border-radius: 4px;  color: white;  padding: 16px 40px;  text-decoration: none;  font-size: 30px;  margin: 2px;  cursor: pointer;} \
.button2 {  background-color: #364cf4; //blue color} \
.content {   padding: 50px;} \
.card-grid {  max-width: 800px;  margin: 0 auto;  display: grid;  grid-gap: 2rem;  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));} \
.card {  background-color: white;  box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);} \
.card-title {  font-size: 1.2rem;  font-weight: bold;  color: #034078} \
</style> \
<title>Fire detection system dashboard</title> \
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
<link rel=\"icon\" href=\"data:,\"> \
<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\"    integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\"> \
<link rel=\"stylesheet\" type=\"text/css\" > \
</head> \
<body> \
<h2>Fire detection system dashboard</h2> \
<div class=\"content\"> \
<div class=\"card-grid\"> \
<div class=\"card\"> \
<h1> Effectors control </h1> \
<p><i class=\"fa-regular fa-pump fa-2x\" style=\"color:#c81919;\"></i>     <strong>Manual pump control</strong></p>";

char pump_state[] = "<p>Pump state: <strong> %d </strong></p> \
<p>          <a href=\"/pump2on\"><button class=\"button\">ON</button></a>          <a href=\"/pump2off\"><button class=\"button button2\">OFF</button></a>        </p> \
<p><i class=\"fa-regular fa-pump fa-2x\" style=\"color:#c81919;\"></i>     <strong>Manual alarm control</strong></p>";

char pump_state_parsed[320];

char alarm_state[] = "<p>Alarm state: <strong> %d </strong></p> \
<p>          <a href=\"/alarm2on\"><button class=\"button\">ON</button></a>          <a href=\"/alarm2off\"><button class=\"button button2\">OFF</button></a>        </p> \
<hr /> \
<h1> Nodes readings </h1> \
<p> <a href=\"/\"><button class=\"button\">Refresh</button></a> </p>";

char alarm_state_parsed[320];

char node_state[] = "<h2> Node %d </h2> \
<p> Temperature: %d C </p> \
<p> Smoke: %d </p> \
<p> Fire: %d </p>";

char node_state_parsed[120];

char ending_seq[] = "</div> \
</div> \
</div> \
</body> \
</html>";

esp_err_t parse_webpage_code(httpd_req_t *req){
    // send opening sequence
    httpd_resp_send_chunk(req, page_src, HTTPD_RESP_USE_STRLEN);

    // update pump state
    sprintf(pump_state_parsed, pump_state, sys_stat_get_pump());
    httpd_resp_send_chunk(req, pump_state_parsed, HTTPD_RESP_USE_STRLEN);

    // update alarm state
    sprintf(alarm_state_parsed, alarm_state, sys_stat_get_alarm());
    httpd_resp_send_chunk(req, alarm_state_parsed, HTTPD_RESP_USE_STRLEN);

    // update nodes
    for(int i = 0; i < NODES_MAX_NUM; i++){
        if(sys_stat_get_online(i)){
            sprintf(node_state_parsed, node_state, 
            i, sys_stat_get_temp(i), sys_stat_get_smoke(i), sys_stat_get_fire(i));
            httpd_resp_send_chunk(req, node_state_parsed, HTTPD_RESP_USE_STRLEN);
        }
    }

    // send closing sequence
    httpd_resp_send_chunk(req, ending_seq, HTTPD_RESP_USE_STRLEN);
    return httpd_resp_send_chunk(req, NULL, 0);
}

void web_app_task(void* params);

// HTTP uri list
httpd_uri_t uri_array[] = {
    {.uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL},

    {.uri = "/pump2on",
    .method = HTTP_GET,
    .handler = pump_on_handler,
    .user_ctx = NULL},

    {.uri = "/pump2off",
    .method = HTTP_GET,
    .handler = pump_off_handler,
    .user_ctx = NULL},

    {.uri = "/alarm2on",
    .method = HTTP_GET,
    .handler = alarm_on_handler,
    .user_ctx = NULL},

    {.uri = "/alarm2off",
    .method = HTTP_GET,
    .handler = alarm_off_handler,
    .user_ctx = NULL},
    };

// HTTP handlers
esp_err_t get_req_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "HTTP get requested\n");
    return parse_webpage_code(req);
}

esp_err_t pump_on_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "HTTP pump on requested\n");
    sys_stat_set_pump(true);
    return parse_webpage_code(req);
}

esp_err_t pump_off_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "HTTP pump off requested\n");
    sys_stat_set_pump(false);
    return parse_webpage_code(req);
}

esp_err_t alarm_on_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "HTTP alarm on requested\n");
    sys_stat_set_alarm(true);
    return parse_webpage_code(req);
}

esp_err_t alarm_off_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "HTTP alarm off requested\n");
    sys_stat_set_alarm(false);
    return parse_webpage_code(req);
}

esp_err_t web_app_start(void){

    ESP_LOGI(TAG, "starting web app");
    web_connect();

    size_t uri_array_size = sizeof(uri_array)/sizeof(uri_array[0]);
    setup_server(uri_array, uri_array_size);
    ESP_LOGI(TAG, "numner of uris created: %d", uri_array_size);

    return ESP_OK;
}