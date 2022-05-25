#ifndef OLED_HPP
#define OLED_HPP
extern "C"
{
#include "main.h"
}
class SSD1106Driver
{
public:

    
    SSD1106Driver(oledUesdPin);
    void OLED_Init(oledUesdPin);
    void OLED_Display_On(void);
    void OLED_Display_Off(void);

    void OLED_Clear(void);
    void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t t);
    void OLED_Fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);
    
    void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr);
    void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);   
    void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *p);
    //TODO HARDCODE接下来要去除以上三个函数，并且集合在OLED_SHOW中，顺便试下泛型编程
    void OLED_Show();
    void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no);
    void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);


private:
    struct oledUesdPin
    {
        SPI_HandleTypeDef * inSPI,
        GPIO_TypeDef* DC_GPIO_Port,
        uint16_t DC_Pin,
        /* data */
    };
    void OLED_Set_Pos(unsigned char x, unsigned char y);
}

#endif // !OLED_HPP
