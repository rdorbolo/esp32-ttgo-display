

#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "fonts.h"
#include "ttgo.h"

static const char *TAG = "TTGO";

uint8_t bgRed, bgGreen,  bgBlue;

uint8_t previousPxRed;
uint8_t previousPxGreen;
uint8_t previousPxBlue;
uint8_t pxCount = 0;


void rst_(int v) { gpio_set_level(RST_GPIO, v); }
void cs_(int v) { gpio_set_level(CS_GPIO, v); }
void dc_(int v) { gpio_set_level(DC_GPIO, v); }
void clk(int v) { gpio_set_level(CLK_GPIO, v); }
void sda(int v) { gpio_set_level(SDA_GPIO, v); }

//The driver samples the SDA (input data) at rising edge of SCL,
//but shifts SDA (output data) at the falling edge of SCL

//After the read status command has been sent, the SDA line must be set to tri-state
//no later than at the
//falling edge of SCL of the last bit.



/*
Print out the gpio pin values used for debugging
*/
void printValues(int header)
{
    if (header)
        printf("RST CS_ DC_ CLK SDA\n");
    else
        printf("%d   %d   %d   %d   %d\n", gpio_get_level(RST_GPIO), gpio_get_level(CS_GPIO), gpio_get_level(DC_GPIO), gpio_get_level(CLK_GPIO), gpio_get_level(SDA_GPIO));
}


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
    //  clk(1); //dummy read needed by some commands
    //  clk(0);

    // Read a bunch of bytes 
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
    dc_(1);
    cs_(0);
    
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
        wrData2((previousPxRed  & 0xf0) | ((previousPxGreen & 0x0f0) >> 4));
        wrData2((previousPxBlue & 0xf0) | ((pxRed & 0x0f0) >> 4));
        wrData2((pxGreen        & 0xf0) | ((pxBlue & 0x0f0) >> 4));
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


    const unsigned x1 = 40 + x;
    const unsigned x2 = 40 + x + w - 1;
    wrCmmd(ST7789_CASET); // Column address set - x axix
    wrData(x1>>8);
    wrData(x1);
    wrData(x2>>8);
    wrData(x2);

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
    const unsigned x1 = 40 + x;
    const unsigned x2 = 40 + x + w - 1;
    wrCmmd(ST7789_CASET); // Column address set - x axix
    wrData(x1>>8);
    wrData(x1);
    wrData(x2>>8);
    wrData(x2);

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

/*
Draw string on display
*/
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


/*Need to run this command first*/
void initTTGO() {
    ESP_LOGI(TAG, "Initialising the TTGO Display");
    gpio_set_direction(SDA_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(CLK_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(DC_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(CS_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_direction(RST_GPIO, GPIO_MODE_INPUT_OUTPUT);

    gpio_reset_pin(BL_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BL_GPIO, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(BL_GPIO, 1); // Turn on the back light

    //printValues(1);
    //printValues(0);

    rst_(0);
    cs_(1);
    dc_(1);
    clk(0);

    
    rst_(1);
    


    //rdCmdByte(0xDA);
    //rdCmdByte(0xDB);

    // There is some type of bug in these two line
    // I think for the first write did not work
    // wrByte is needed here
    // wrCmmd/wrData seem to mess things up
    wrByte(0x53, 0xff); // 
    //wrCmmd(0x53);
    //wrData(0xff);

    //wrCmmd(0x53);
    //wrData(0xff);

    //rdCmdByte(0x54);

    wrCmmd(ST7789_SLPOUT); // Sleep out
    

    //wrCmmd(ST7789_NORON); // Normal display mode on

    //------------------------------display and color format setting--------------------------------//
    //wrCmmd(ST7789_MADCTL);
    //wrData(0x8);

    
    //--------------------------------ST7789V Frame rate setting----------------------------------//
    /*
    wrCmmd(ST7789_PORCTRL);
    wrData(0x0c);
    wrData(0x0c);
    wrData(0x00);
    wrData(0x33);
    wrData(0x33);
    */

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
    //wrData(0xa1);
    wrData(0x81);

    //--------------------------------ST7789V gamma setting---------------------------------------//
   /*
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
*/
    // Set Gamma curve to G2.5
    wrCmmd(ST7789_GAMSET);
    wrData(0x04);

    wrCmmd(ST7789_INVON);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //end_tft_write();
    //delay(120);
    //begin_tft_write();

    wrCmmd(ST7789_DISPON); //Display on

    wrCmmd(ST7789_RAMCTRL);
    wrData(0x00);
    wrData(0xf0);

    //rdCmdWord16(0x0B);

    // 12-bit color/pixel enabled
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

}