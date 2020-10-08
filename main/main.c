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
//#include "driver/uart.h"
#include "esp_timer.h"
//#include <string.h>

#include "ttgo.h"

extern uint8_t bgRed, bgGreen, bgBlue;

void demo1()
{

    //TTGO Logo is top
    char s[30];
    bgRed = 0x40;
    bgGreen = 0x50;
    bgBlue = 0xd0;
    clearScreen(bgRed, bgGreen, bgBlue);
    vTaskDelay(50 / portTICK_PERIOD_MS);

    int count = 0;
    while (1)
    {

        unsigned x = 10;
        unsigned y = 20;
        count++;

        vTaskDelay(50 / portTICK_PERIOD_MS);

        snprintf(s, 30, "This ");
        x = displayStr(s, x, y, 0xff, 0x00, 0x00);

        snprintf(s, 30, "is ");
        x = displayStr(s, x, y, 0x00, 0x00, 0x00);

        snprintf(s, 30, "a ");
        x = displayStr(s, x, y, 0xff, 0xff, 0x00);

        snprintf(s, 30, "Test ");
        x = displayStr(s, x, y, 0xff, 0x00, 0xff);

        x = 10;
        y = y + 32; 
        snprintf(s,30, "Count: %d", count);
        displayStr(s, x, y, 0xff, 0xff, 0xff);

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        count++;
        fillBox(210, 15, 14, 14, 0, 0x0c0 >> (count % 2), 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}



void app_main(void)
{

    initTTGO();

    demo1();
}
