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
    unsigned x = 10;
    unsigned y = 20;

    vTaskDelay(50 / portTICK_PERIOD_MS);

    snprintf(s, 30, "This ");
    x = displayStr(s, x, y, 0xff, 0x00, 0x00, 32);

    snprintf(s, 30, "is ");
    x = displayStr(s, x, y, 0x00, 0x00, 0x00, 32);

    snprintf(s, 30, "a ");
    x = displayStr(s, x, y, 0xff, 0xff, 0x00, 32);

    snprintf(s, 30, "Test ");
    x = displayStr(s, x, y, 0xff, 0x00, 0xff, 32);

    x = 10;
    y = y + 32;
    snprintf(s, 30, "Count:");
    displayStr(s, x, y, 0xff, 0xff, 0xff, 32);

    y = y + 32;
    

    int i;
    for (i = 0; i < 8; i++)
    {
        fillBox(190+20*(i/4), 15 + 20 * i - 80*(i/4), 16, 16, 0, 0x0f0 - 0x20 * i, 0);
    }

    while (1)
    {
        snprintf(s, 30, "%d", count);
        displayStr(s, x, y, 0xff, 0xff, 0xff, 64);

        vTaskDelay(50 / portTICK_PERIOD_MS);
        count++;

    }
}

void app_main(void)
{

    initTTGO();

    demo1();
}
