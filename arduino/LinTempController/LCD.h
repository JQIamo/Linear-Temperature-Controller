#include "Arduino.h"
#include "SPI.h"

typedef char *dtext_t[2];

class LCD {
 public:
    LCD(int pin_reset, int pin_register_select_, int pin_chip_select);
    void init();
    void write(String data, byte addr);
    void write(const char text[], byte addr);
    void write(dtext_t text);
    void write(char data);
    void write_cmd(byte cmd);
    void clear();
    void clear_1st_line();
    void clear_2nd_line();

 private:
    int pin_reset_, pin_chip_select_, pin_register_select_;
    SPISettings SPI_settings_lcd_;
};



