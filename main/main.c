/*

   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "sdkconfig.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include <string.h>

#include "ttgo.h"


#define UART_RX 27
#define UART_TX 26

extern uint8_t bgRed, bgGreen,  bgBlue;

struct AirData
{
    unsigned pm1;
    unsigned pm2_5;
    unsigned pm10;
    unsigned count_3;
    unsigned count_5;
    unsigned count1;
    unsigned count2_5;
    unsigned count5;
    unsigned count10;
    int64_t sampleTime;
} airData;

int displayMode = 0;
 


//The driver samples the SDA (input data) at rising edge of SCL,
//but shifts SDA (output data) at the falling edge of SCL

//After the read status command has been sent, the SDA line must be set to tri-state no later than at the
//falling edge of SCL of the last bit.


unsigned calcAqi(unsigned pm25)
{

    int interval_y0;
    int interval_y1;
    int interval_x0;
    int interval_x1;

    if (pm25 <= 12)
    {
        interval_y0 = 0;
        interval_y1 = 50;
        interval_x0 = 0;
        interval_x1 = 12;
    }
    else if (pm25 <= 35)
    {
        interval_y0 = 51;
        interval_y1 = 100;
        interval_x0 = 13;
        interval_x1 = 35;
    }
    else if (pm25 <= 55)
    {
        interval_y0 = 101;
        interval_y1 = 150;
        interval_x0 = 35;
        interval_x1 = 55;
    }
    else if (pm25 <= 150)
    {
        interval_y0 = 151;
        interval_y1 = 200;
        interval_x0 = 55;
        interval_x1 = 150;
    }
    else if (pm25 <= 250)
    {
        interval_y0 = 201;
        interval_y1 = 300;
        interval_x0 = 150;
        interval_x1 = 250;
    }
    else if (pm25 <= 350)
    {
        interval_y0 = 301;
        interval_y1 = 400;
        interval_x0 = 250;
        interval_x1 = 350;
    }
    else if (pm25 <= 500)
    {
        interval_y0 = 401;
        interval_y1 = 500;
        interval_x0 = 350;
        interval_x1 = 500;
    }
    else{
        return pm25;
    }
    int retval = interval_y0 + ((pm25 - interval_x0) * (interval_y1 - interval_y0) / (interval_x1 - interval_x0));
    return retval;
}


void test1()
{

    //TTGO Logo is top
    char s[30];
    int count = 0;

    while (1)
    {

        if (displayMode == 0) {
            bgRed = 0x40;
            bgGreen = 0x50;
            bgBlue  =  0xe0;
            clearScreen(bgRed, bgGreen,bgBlue);
        }

        while (1)
        {

            if (displayMode == 0)
            {
            int x = 10;
            int y = 10; 
            count++;
            

            
            snprintf(s, 30, "AQI:  %d   ", calcAqi(airData.pm2_5) );
            displayStr(s, x, y,0xf0, 0xf0, 0xf0);
            y = y + 32;

            snprintf(s, 30, "pm2.5:  %d   ", airData.pm2_5);
            displayStr(s, x, y,0xf0, 0xf0, 0xf0);
            y = y + 32;
            snprintf(s, 30, "0.3 qty:  %d   ", airData.count_3);
            displayStr(s, x, y,0xf0, 0xf0, 0xf0);

            fillBox(210, 15,  14, 14, 0, 0x0f0>>(count%2), 0);        
            }
            else
            {

                uint8_t random;
                random = (uint8_t)esp_random();
                bgRed = random;
                bgGreen = random;
                bgBlue  = random;
                clearScreen(random, random, random >> 2);
                vTaskDelay(50 / portTICK_PERIOD_MS);

                //wrCmmd(ST7789_MADCTL);
                //wrData(0b01101000);

                
                int x = 20;
                int y = 20;

                snprintf(s, 30, "This ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
                snprintf(s, 30, "is a ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
                snprintf(s, 30, "Test ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);   
            //count++;
            fillBox(210, 15,  14, 14, 0, 0x0c0>>(count%2), 0);        
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

static const int RX_BUF_SIZE = 1024;

void init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char *logName, const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, 32, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            //ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            //ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);

            if ((data[0] == 'B') &&
                (data[1] == 'M') &&
                (rxBytes == 32) &&
                (data[2] == (uint8_t)0) &&
                (data[3] == (uint8_t)28))
            {

                airData.pm1 = (((unsigned)data[4]) << 8) + (unsigned)data[5];
                airData.pm2_5 = (((unsigned)data[6]) << 8) + (unsigned)data[7];
                airData.pm10 = (((unsigned)data[8]) << 8) + (unsigned)data[9];
                airData.count_3 = (((unsigned)data[16]) << 8) + (unsigned)data[17];

                //airData.count_3 =
                airData.sampleTime = esp_timer_get_time();
            }

            //printf();
           // ESP_LOGI(RX_TASK_TAG, "pm2.5 = %d .3um Count = %d ts=%lld", airData.pm2_5,
           //          airData.count_3, airData.sampleTime);
        }

        //else printf("no data\n");
    }
    free(data);
}

void app_main(void)
{

    initTTGO();
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);

    test1();
}
