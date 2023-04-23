#include <esp_http_server.h>

void web_connect(void);
httpd_handle_t setup_server(httpd_uri_t* uris, size_t uris_count);