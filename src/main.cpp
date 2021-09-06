#include "WiFi.h"

extern "C"{
#include <sh2lib.h>
}
 
extern const uint8_t server_root_cert_pem_start[] ;
extern const uint8_t server_root_cert_pem_end[]  ;

/* The HTTP/2 server to connect to */
#define HTTP2_SERVER_URI  "https://http2.golang.org"
/* A GET request that keeps streaming current time every second */
#define HTTP2_STREAMING_GET_PATH  "/clockstream"
/* A PUT request that echoes whatever we had sent to it */
#define HTTP2_PUT_PATH            "/ECHO"

const char* ssid = "LINA_NET";
const char* password =  "ventilador";
 
int handle_get_response(struct sh2lib_handle *handle, const char *data, size_t len, int flags)
{
    if (len > 0) {
        Serial.printf("%.*s\n", len, data);
    }
 
    if (flags == DATA_RECV_RST_STREAM) {
        Serial.println("STREAM CLOSED");
    }
    return 0;
}
 
static void http2_task(void *args)
{
    /* Set current time: proper system time is required for TLS based
     * certificate verification.
     */
    //set_time();

    /* HTTP2: one connection multiple requests. Do the TLS/TCP connection first */
    printf("Connecting to server\n");
    struct sh2lib_config_t cfg = {
        .uri = HTTP2_SERVER_URI,

    };
    struct sh2lib_handle hd;

    if (sh2lib_connect(&cfg, &hd) != 0) {
        printf("Failed to connect\n");
        vTaskDelete(NULL);
        return;
    }
    printf("Connection done\n");

    /* HTTP GET  */
    sh2lib_do_get(&hd, HTTP2_STREAMING_GET_PATH, handle_get_response);

    /* HTTP PUT  */
    //sh2lib_do_put(&hd, HTTP2_PUT_PATH, send_put_data, handle_echo_response);

    while (1) {
        /* Process HTTP2 send/receive */
        if (sh2lib_execute(&hd) < 0) {
            printf("Error in send/receive\n");
            break;
        }
        vTaskDelay(2);
    }

    sh2lib_free(&hd);
    vTaskDelete(NULL);
}
 
void setup() {
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Conectado");
  xTaskCreate(http2_task, "http2_task", (1024 * 32), NULL, 5, NULL);
 
}
 
void loop() {
  vTaskDelete(NULL);
}