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
extern uint8_t fillboxBgRed, fillboxBgGreen, fillboxBgBlue;


void demo1()
{

    //TTGO Logo is top
    char s[30];
    bgRed = 0x30;
    bgGreen = 0x40;
    bgBlue = 0xd0;
    //clearScreen(bgRed, bgGreen, bgBlue);
    //vTaskDelay(50 / portTICK_PERIOD_MS);

    //fillBox(0, 0, 40, 40, 0xff,0x0,0xff);
    //fillBox(0, 0, 40, 40, 0xff,0x0,0xff);

    int count = 0;
    unsigned x = 10;
    unsigned y = 10;

    vTaskDelay(50 / portTICK_PERIOD_MS);



     clearScreen(bgRed, bgGreen, bgBlue);

    snprintf(s, 30, "This ");
    x = displayStr(s, x, y, 0xff, 0x00, 0x00, 32);

   
    x=10;
    x = displayStr(s, x, y, 0xff, 0x00, 0x00, 32);
   

    //snprintf(s, 30, "This ");
    //x = displayStr(s, x, y, 0xff, 0x00, 0x00, 32);



    snprintf(s, 30, "is ");
    x = displayStr(s, x, y, 0x00, 0x00, 0x00, 32);

    snprintf(s, 30, "a ");
    x = displayStr(s, x, y, 0xff, 0xff, 0x00, 32);

    snprintf(s, 30, "Test ");
    x = displayStr(s, x, y, 0xff, 0x00, 0xff, 32);

    x = 0;
    y = y + 32;
    //snprintf(s, 30, "Count:");
    //displayStr(s, x, y, 0xff, 0xff, 0xff, 32);

    uint8_t data[240];
    uint64_t lastValue = 0;
    uint64_t currentValue = 0;
    uint headPtr = 0;
    for (int i= 0; i<240; i++) {
        
        data[i] = 0; //(uint8_t) ( currentValue % (uint64_t)20 ) ;//  i%20;
    }

    
    //fillBox(x, y, 10, 10, 0xff,0xff,0xff);
    //fillBox(x, y+10, 10, 10, 0x00,0xff,0x00); 

   // y = y + 32;
    

    int i;
    for (i = 0; i < 8; i++)
    {
        fillBox(190+20*(i/4), 15 + 20 * i - 80*(i/4), 16, 16, 0, 0x0f0 - 0x20 * i, 0);
    }

    //fillBox(0, 47, 240, 29, 0x0,0x0,0x0);
    //fillBox(0, 0, 40, 40, 0x50,0x0,0x0);

    //fillBox(0,0, 40, 40, 0x50,0x0,0x0);)
   
fillboxBgRed    = 0x10;
fillboxBgGreen  = 0x10;
fillboxBgBlue   = 0x10;

    fillBox(0, HEIGHTY-45, 240, 40, fillboxBgRed, fillboxBgGreen, fillboxBgBlue);

    while (1)
    {
        int h = 30;
        snprintf(s, 30, "%d", count);
        displayStr(s, 10, y, 0xff, 0xff, 0xff, 64);
        lastValue = currentValue;
        currentValue = lastValue + lastValue * 2161 + esp_timer_get_time() * 2267;
        //data[headPtr%240] = (uint8_t) ( currentValue % (uint64_t)20 );
        data[headPtr%240] = (uint8_t) ( count % h );
        fillBox2(0,HEIGHTY-40, 240, 30, 0xff,0xff,0x0,data, headPtr);
        headPtr = (headPtr + 1 ) %240;
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
        count++;

    }
}

void app_main(void)
{

    initTTGO();

    demo1();
}
