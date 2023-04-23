#include <stdio.h>
#include <stdlib.h>
#include <string.h> //Requires by memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>

#define LED_PIN 2

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
      <title>Fire detection system</title> \
      <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
      <link rel=\"icon\" href=\"data:,\"> \
      <link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\"    integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\"> \
      <link rel=\"stylesheet\" type=\"text/css\" > \
   </head> \
   <body> \
      <h2>Fire detection system</h2> \
      <div class=\"content\"> \
         <div class=\"card-grid\"> \
            <div class=\"card\"> \
               <p><i class=\"fa-regular fa-pump fa-2x\" style=\"color:#c81919;\"></i>     <strong>Manual pump control</strong></p> \
               <p>Pump state: <strong> ON</strong></p> \
               <p>          <a href=\"/pump2on\"><button class=\"button\">ON</button></a>          <a href=\"/pump2off\"><button class=\"button button2\">OFF</button></a>        </p> \
               <p><i class=\"fa-regular fa-pump fa-2x\" style=\"color:#c81919;\"></i>     <strong>Manual alarm control</strong></p> \
               <p>Alarm state: <strong> ON</strong></p> \
               <p>          <a href=\"/alarm2on\"><button class=\"button\">ON</button></a>          <a href=\"/alarm2off\"><button class=\"button button2\">OFF</button></a>        </p> \
               <p> Temperature: 2137 </p> \
               <p> Smoke: XX </p> \
               <p> Fire: XX </p> \
            </div> \
         </div> \
      </div> \
   </body> \
</html>";

static const char *TAG = "espressif"; // TAG for debug
int led_state = 0;
static led_strip_handle_t led_strip;

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void connect_wifi(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
}

esp_err_t send_web_page(httpd_req_t *req)
{
    return httpd_resp_send(req, page_src, HTTPD_RESP_USE_STRLEN);
}

esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req);
}

esp_err_t pump_on_handler(httpd_req_t *req)
{
    led_strip_set_pixel(led_strip, 0, 0, 16, 0);
    led_strip_refresh(led_strip);
    led_state = 1;
    return send_web_page(req);
}

esp_err_t pump_off_handler(httpd_req_t *req)
{
    led_strip_set_pixel(led_strip, 0, 16, 0, 0);
    led_strip_refresh(led_strip);
    led_state = 0;
    return send_web_page(req);
}

esp_err_t alarm_on_handler(httpd_req_t *req)
{
    led_strip_set_pixel(led_strip, 0, 0, 16, 16);
    led_strip_refresh(led_strip);
    led_state = 1;
    return send_web_page(req);
}

esp_err_t alarm_off_handler(httpd_req_t *req)
{
    led_strip_set_pixel(led_strip, 0, 16, 16, 0);
    led_strip_refresh(led_strip);
    led_state = 0;
    return send_web_page(req);
}

httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

httpd_uri_t uri_pump_on = {
    .uri = "/pump2on",
    .method = HTTP_GET,
    .handler = pump_on_handler,
    .user_ctx = NULL};

httpd_uri_t uri_pump_off = {
    .uri = "/pump2off",
    .method = HTTP_GET,
    .handler = pump_off_handler,
    .user_ctx = NULL};

httpd_uri_t uri_alarm_on = {
    .uri = "/alarm2on",
    .method = HTTP_GET,
    .handler = alarm_on_handler,
    .user_ctx = NULL};

httpd_uri_t uri_alarm_off = {
    .uri = "/alarm2off",
    .method = HTTP_GET,
    .handler = alarm_off_handler,
    .user_ctx = NULL};

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_pump_on);
        httpd_register_uri_handler(server, &uri_pump_off);
        httpd_register_uri_handler(server, &uri_alarm_on);
        httpd_register_uri_handler(server, &uri_alarm_off);
    }

    return server;
}


static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    connect_wifi();

    // GPIO initialization
    configure_led();

    led_state = 0;
    ESP_LOGI(TAG, "LED Control Web Server is running ... ...\n");
    setup_server();
}