#ifndef LCD_h
#define LCD_h
#include "Arduino.h"
#include "SPI.h"


typedef char *dtext_t[2];

class LCD {
 public:
    LCD(int pin_reset, int pin_register_select_, int pin_chip_select);
    void init();
    // Write commands for various data types:
    void write(String data, byte addr); // For the 1st line, 1st character, addr = 0x00. For the 2nd line, 1st character, addr = 0x40.
    void write(const char text[], byte addr);
    void write(dtext_t text);
    void write(char data);
    void write_cmd(byte cmd);
    //void write(String data, int position);
    // Clear various parts of the screen:
    void clear();
    void clear_1st_line();
    void clear_2nd_line();
    // Macros for pretty UI effects:
    void flash_string(String text, byte position);


 private:
    int pin_reset_, pin_chip_select_, pin_register_select_;
    SPISettings SPI_settings_lcd_;
};

#endif
