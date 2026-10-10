/*
 * Lab 6 -- MQTT over TLS through emulated Ethernet (STARTER)
 *
 * Ugradbeni racunalni sustavi -- Lab 6: MQTT/TLS telemetry
 *
 * What is GIVEN here:
 *   - Network bring-up: openeth + DHCP via example_connect()
 *     (blocks until an IPv4 address is acquired; under QEMU that
 *     is 10.0.2.15, gateway 10.0.2.2).
 *   - Sensor bring-up: bme280_init() + a read loop producing T/P/H
 *     samples from the shared bme280_sim (same synthetic source as
 *     Lab 3).
 *
 * What YOU must add (see the TODOs):
 *   1. TLS config: point esp-mqtt at mqtts://10.0.2.2:8883 and
 *      give it the CA certificate to verify the broker. The CA
 *      cert is embedded from main/certs/ca.crt (EMBED_TXTFILES in
 *      main/CMakeLists.txt) -- generate it first with
 *      broker/gen_certs.sh. Use the symbols declared below.
 *      (CONFIG_LAB6_PLAINTEXT, default off, is available if you
 *       first want to bring MQTT up without TLS against
 *       mqtt://10.0.2.2:1883.)
 *   2. Topic design: publish to "vuka/sensors/telemetry".
 *   3. Publish loop: every 2 s format one sample as JSON and
 *      publish it. Print "LAB6 OK connected=1" once the broker
 *      connects and "LAB6 OK published=<n>" per successful publish
 *      (these are what the smoke test greps for).
 *
 * Reference: ESP-IDF esp-mqtt (esp_mqtt_client_*),
 * examples/protocols/mqtt/ssl.
 */
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "mqtt_client.h"

#include "bme280.h"

static const char *TAG = "lab6";

/* CA certificate embedded from certs/ca.crt (EMBED_TXTFILES in
 * CMakeLists). Pass ca_crt_start to
 * cfg.broker.verification.certificate for TLS. */
extern const uint8_t ca_crt_start[] asm("_binary_ca_crt_start");
extern const uint8_t ca_crt_end[]   asm("_binary_ca_crt_end");

#define TELEMETRY_TOPIC     "vuka/sensors/telemetry"
#define PUBLISH_PERIOD_MS   2000

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* GIVEN: network bring-up (openeth + DHCP). Blocks until an IP
     * is acquired. */
    ESP_LOGI(TAG, "Ugradbeni racunalni sustavi -- "
                  "Lab 6 (starter): bringing up network");
    ESP_ERROR_CHECK(example_connect());

    /* GIVEN: sensor bring-up. */
    ESP_ERROR_CHECK(bme280_init());

    /* TODO 1: create and start an esp-mqtt client, configured for
     *   TLS. Fill in an esp_mqtt_client_config_t: point
     *   cfg.broker.address.uri at the TLS broker
     *   (mqtts://10.0.2.2:8883), and give it the embedded CA
     *   certificate for server verification via the two
     *   cfg.broker.verification.certificate / .certificate_len
     *   fields -- ca_crt_start/ca_crt_end above are exactly what
     *   those two fields need (mind the pointer cast and the length
     *   arithmetic). Then call esp_mqtt_client_init(&cfg), register
     *   an event handler, and start the client with
     *   esp_mqtt_client_start().
     *   (CONFIG_LAB6_PLAINTEXT, default off, swaps in
     *    mqtt://10.0.2.2:1883 with no TLS fields at all, if you
     *    first want MQTT working without TLS before tackling the
     *    certificate wiring.)
     *
     * TODO 2: in the event handler, on MQTT_EVENT_CONNECTED, log
     *   "LAB6 OK connected=1" and signal the publish loop (e.g. an
     *   EventGroup).
     */

    int published = 0;
    for (;;) {
        float temp_c = 0.0f, press_hpa = 0.0f, hum_pct = 0.0f;
        esp_err_t err = bme280_read(&temp_c, &press_hpa, &hum_pct);
        if (err == ESP_OK) {
            /* Local readout so the starter builds and boots
             * as-is. */
            ESP_LOGI(TAG,
                     "sample T=%.1fC P=%.1fhPa H=%.1f%% "
                     "(not yet published)",
                     temp_c, press_hpa, hum_pct);

            /* TODO 3: format this sample as JSON and publish it
             *   to TELEMETRY_TOPIC with
             *   esp_mqtt_client_publish(..., qos=1). On a
             *   successful publish, increment `published` and
             *   log "LAB6 OK published=%d". */
            (void)published;
            (void)TELEMETRY_TOPIC;
        } else {
            ESP_LOGE(TAG, "bme280_read failed: %s",
                     esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(PUBLISH_PERIOD_MS));
    }
}
