/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include "string.h"

#include "fonts.h"

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
uint8_t bgRed, bgGreen,  bgBlue;

//The driver samples the SDA (input data) at rising edge of SCL,
//but shifts SDA (output data) at the falling edge of SCL

//After the read status command has been sent, the SDA line must be set to tri-state no later than at the
//falling edge of SCL of the last bit.

uint8_t previousPxRed;
uint8_t previousPxGreen;
uint8_t previousPxBlue;
uint8_t pxCount = 0;


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


void printValues(int header)
{
    if (header)
        printf("RST CS_ DC_ CLK SDA\n");
    else
        printf("%d   %d   %d   %d   %d\n", gpio_get_level(RST_GPIO), gpio_get_level(CS_GPIO), gpio_get_level(DC_GPIO), gpio_get_level(CLK_GPIO), gpio_get_level(SDA_GPIO));
}

void rst_(int v) { gpio_set_level(RST_GPIO, v); }
void cs_(int v) { gpio_set_level(CS_GPIO, v); }
void dc_(int v) { gpio_set_level(DC_GPIO, v); }
void clk(int v) { gpio_set_level(CLK_GPIO, v); }
void sda(int v) { gpio_set_level(SDA_GPIO, v); }

void rdCmdByte(unsigned b)
{
    printf("read addr=%x\n", b);
    printValues(1);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(0);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = b;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        printValues(0);
        clk(1);

        if (i == 7)
        {
            gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT);
        }
        printValues(0);
        clk(0);
    }
    dc_(1);

    for (i = 0; i < 8; i++)
    {
        printValues(0);
        clk(1);
        printValues(0);
        clk(0);
    }

    cs_(1);
    printValues(0);
}

void rdData()
{
    printf("read serial data\n");
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT);
    // dc_(1);
    // cs_(0);
    int i;
    int v = 0;

    for (i = 0; i < 8; i++)
    {
        v = v | (gpio_get_level(SDA_GPIO) << (7 - i));
        printValues(0);
        clk(1);
        printValues(0);
        clk(0);
    }

    printf("v=%x\n", v);
    //cs_(1);
    //printValues(0);
}

void rdCmdWord16(unsigned b)
{
    printf("read addr=%x\n", b);
    printValues(1);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(0);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = b;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        printValues(0);
        clk(1);

        if (i == 7)
        {
            gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT);
        }
        printValues(0);
        clk(0);
    }
    dc_(1);
    //  clk(1); //dummy read
    //  clk(0);
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    rdData();
    /*
    for (i=0; i<16;i++) {
        printValues(0);
        clk(1);
        printValues(0);
        clk(0);
    }
    */

    cs_(1);
    printValues(0);
}

void wrCmmd(int cmd)
{
    //printf("write cmd=%x\n", cmd);
    //printValues(1);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(0);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = cmd;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        //printValues(0);
        clk(1);
        //printValues(0);
        clk(0);
    }
    cs_(1);
    //printValues(0);
}

void wrData(int data)
{
    //printf("write data=%x\n", data);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(1);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = data;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        //printValues(0);
        clk(1);
        //printValues(0);
        clk(0);
    }

    cs_(1);
    //printValues(0);
}

void wrData2(int data)
{
    //printf("write data=%x\n", data);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(1);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = data;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        //   printValues(0);
        clk(1);
        //   printValues(0);
        clk(0);
    }

    ///cs_(1);
    //(0);
}

void wrByte(int addr, int data)
{
    printf("write addr=%x, data =%x\n", addr, data);
    //printValues(1);
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    cs_(0);
    dc_(0);
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned a = addr;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        //printValues(0);
        clk(1);
        //printValues(0);
        clk(0);
    }
    dc_(1);
    for (i = 0; i < 8; i++)
    {
        unsigned a = data;
        a = (a << i) & 0x0ff;
        a = a / 0x80;
        sda(a);
        //printValues(0);
        clk(1);
        //printValues(0);
        clk(0);
    }

    cs_(1);
    //printValues(0);
}

void delay(int i)
{
    return;
}

void sendPx(uint8_t pxRed, uint8_t pxGreen, uint8_t pxBlue)
{
    if (pxCount >= 1)
    {
        pxCount = 0;
        wrData2((previousPxRed << 4) | ((previousPxGreen & 0x0f0) >> 4));
        wrData2((previousPxBlue << 4) | ((pxRed & 0x0f0) >> 4));
        wrData2((pxGreen << 4) | ((pxBlue & 0x0f0) >> 4));
    }
    else
    {
        pxCount++;
        previousPxRed = pxRed;
        previousPxGreen = pxGreen;
        previousPxBlue = pxBlue;
    }
}

unsigned printChar(unsigned char c, unsigned x, unsigned y, uint8_t pxRed, uint8_t pxGreen, uint8_t pxBlue)
{

    unsigned h = 26;
    unsigned w;

    w = (unsigned)widtbl_f32[c - (unsigned char)32];
    const unsigned char *font = chrtbl_f32[c - (unsigned char)32];

    wrCmmd(ST7789_CASET); // Column address set - x axix
    wrData(0x00);
    wrData(40 + x);
    wrData(0x00);
    wrData(40 + x + w - 1);

    wrCmmd(ST7789_RASET); // Row address set
    wrData(0x00);
    wrData(53 + y);
    wrData(0x00);
    wrData(53 + y + h - 1); 

    wrCmmd(ST7789_RAMWR);

    int i, j;
    int count = 0;
    int len;
    int val;
    int pc = 0;

    //printf("width:%d\n", w);

    for (i = 0; pc < w * h; i++)
    {
        len = (font[i] & 0x7f) + 1;
        val = font[i] >> 7;
        count = count + len;
        //      printf("%d %d %d\n", val, len, count);
        for (j = 0; j < len; j++)
        {
            pc++;
            if (val)
                sendPx(pxRed, pxGreen, pxBlue);
            else
                sendPx(bgRed, bgGreen, bgBlue);
            //vTaskDelay(50 / portTICK_PERIOD_MS);
            //printf("%d", val);
            //if (pc % w == 0) printf("\n");
        }
    }

    cs_(1);

    return w;
}


void fillBox(unsigned x, unsigned y, unsigned w, unsigned h, uint8_t pxRed, uint8_t pxGreen, uint8_t pxBlue)
{
    wrCmmd(ST7789_CASET); // Column address set - x axix
    wrData(0x00);
    wrData(40 + x);
    wrData(0x00);
    wrData(40 + x + w - 1);

    wrCmmd(ST7789_RASET); // Row address set
    wrData(0x00);
    wrData(53 + y);
    wrData(0x00);
    wrData(53 + y + h - 1); 

    wrCmmd(ST7789_RAMWR);
    int i;
    for (i = 0; i < w*h; i++)
    {
        sendPx(pxRed, pxGreen, pxBlue);
    }
    cs_(1);
}


void clearScreen(uint8_t pxRed, uint8_t pxGreen, uint8_t pxBlue)
{
    wrCmmd(ST7789_CASET); // Column address set - Y axis
    wrData(0x00);
    wrData(40); // First Column is 40
    wrData(0x01);
    wrData(23); //  Last Colum is 40 + 239

    wrCmmd(ST7789_RASET); // Row address set
    wrData(0x00);
    wrData(53);
    wrData(0x00);
    wrData(256);

    wrCmmd(ST7789_RAMWR);
    int i;
    for (i = 0; i < 135 * 240; i++)
    {
        sendPx(pxRed, pxGreen, pxBlue);
    }
    cs_(1);
}

unsigned displayStr(char *str, unsigned x, unsigned y, uint8_t pxRed, uint8_t pxGreen, uint8_t pxBlue)
{

    unsigned len = strlen(str);
    int i;

    for (i = 0; i < len; i++)
    {
        x = x + printChar(str[i], x, y, pxRed, pxGreen, pxBlue);
    }

    return x;
}

void test1()
{
    printf("starting: test1\n");
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(CLK_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(DC_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(RST_GPIO, GPIO_MODE_INPUT_OUTPUT);

    gpio_reset_pin(BL_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BL_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(BL_GPIO, 1); // Turn on the back light

    printValues(1);
    printValues(0);

    rst_(0);
    cs_(1);
    dc_(0);
    clk(0);

    printValues(0);
    rst_(1);
    printValues(0);
    rdCmdByte(0xDA);
    rdCmdByte(0xDB);

    //wrByte(0x53, 0xff);
    wrCmmd(0x53);
    wrData(0xff);
    rdCmdByte(0x54);

    //wrByte()
    wrCmmd(ST7789_SLPOUT); // Sleep out
    delay(120);

    wrCmmd(ST7789_NORON); // Normal display mode on

    //------------------------------display and color format setting--------------------------------//
    wrCmmd(ST7789_MADCTL);
    wrData(0x8);

    // JLX240 display datasheet
    //wrCmmd(0xB6);
    //wrData(0x0A);
    //wrData(0x82);

    wrCmmd(ST7789_COLMOD);
    wrData(0x55);
    delay(10);

    //--------------------------------ST7789V Frame rate setting----------------------------------//
    wrCmmd(ST7789_PORCTRL);
    wrData(0x0c);
    wrData(0x0c);
    wrData(0x00);
    wrData(0x33);
    wrData(0x33);

    wrCmmd(ST7789_GCTRL); // Voltages: VGH / VGL
    wrData(0x35);

    //---------------------------------ST7789V Power setting--------------------------------------//
    wrCmmd(ST7789_VCOMS);
    wrData(0x28); // JLX240 display datasheet

    wrCmmd(ST7789_LCMCTRL);
    wrData(0x0C);

    wrCmmd(ST7789_VDVVRHEN);
    wrData(0x01);
    wrData(0xFF);

    wrCmmd(ST7789_VRHS); // voltage VRHS
    wrData(0x10);

    wrCmmd(ST7789_VDVSET);
    wrData(0x20);

    wrCmmd(ST7789_FRCTR2);
    wrData(0x0f);

    wrCmmd(ST7789_PWCTRL1);
    wrData(0xa4);
    wrData(0xa1);

    //--------------------------------ST7789V gamma setting---------------------------------------//
    wrCmmd(ST7789_PVGAMCTRL);
    wrData(0xd0);
    wrData(0x00);
    wrData(0x02);
    wrData(0x07);
    wrData(0x0a);
    wrData(0x28);
    wrData(0x32);
    wrData(0x44);
    wrData(0x42);
    wrData(0x06);
    wrData(0x0e);
    wrData(0x12);
    wrData(0x14);
    wrData(0x17);

    wrCmmd(ST7789_GAMSET);
    wrData(0x04);

    wrCmmd(ST7789_INVON);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //end_tft_write();
    //delay(120);
    //begin_tft_write();

    wrCmmd(ST7789_DISPON); //Display on
    delay(120);

    wrCmmd(ST7789_RAMCTRL);
    wrData(0x00);
    wrData(0xf0);

    //rdCmdWord16(0x0B);

    wrCmmd(ST7789_COLMOD);
    wrData(0x03);

    wrCmmd(ST7789_MADCTL); // Rotate the screen
    wrData(0b01101000);    // Rotate the screen

    /*
How to set ID
wrCmmd(ST7789_IDSET);
wrData(0x12);
wrData(0x34);
wrData(0x56);
*/

    /*
How to read id, NOTE rdCmdWord16 need to add dummy clock cycle
rdCmdWord16(ST7789_RDDID);
*/

    /* read from data ram. note need to read back in 18bit color... I think
  rdCmdWord16(ST7789_RAMRD);
  //wrCmmd(ST7789_RAMRD);
  //rdData();
  //rdData();
  //rdData();
*/

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

            fillBox(200, 10,  10, 10, 0, 0xff*(count%2), 0);

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

                snprintf(s, 30, "Rick ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
                snprintf(s, 30, "Loves ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
                snprintf(s, 30, "Haleh ");
                x = displayStr(s, x, y, 0xff,0xff,0xff);
                y = y + 32;
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);   
            count++;
            fillBox(200, 10,  10, 10, 0, 0xff*(count%2), 0);        
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

    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    //xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);

    test1();
}
