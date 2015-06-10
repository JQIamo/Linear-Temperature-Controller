#include "LCD.h"

LCD::LCD(int pin_reset, int pin_register_select, int pin_chip_select) {
    pin_reset_ = pin_reset;
    pin_register_select_ = pin_register_select;
    pin_chip_select_ = pin_chip_select;
    SPISettings SPI_settings_lcd_(5000000, MSBFIRST, SPI_MODE0);

    pinMode(pin_reset_, OUTPUT);
    pinMode(pin_register_select_, OUTPUT);
    pinMode(pin_chip_select_, OUTPUT);

    digitalWrite(pin_reset_, HIGH);
    digitalWrite(pin_register_select_, HIGH);
    digitalWrite(pin_chip_select_, HIGH);

}

void LCD::init(){
    digitalWrite(pin_reset_, LOW);
    delay(2);
    LCD::write_cmd(0x30); // wakeup
    delay(2);
    digitalWrite(pin_reset_, HIGH);
    LCD::write_cmd(0x30); // wakeup
    LCD::write_cmd(0x30); // wakeup

    LCD::write_cmd(0x39); // function set
    LCD::write_cmd(0x14);   // internal osc frequency
    LCD::write_cmd(0x56);   // power control
    LCD::write_cmd(0x6D);   // follower control

    LCD::write_cmd(0x70);   // contrast
    LCD::write_cmd(0x0C);   // display on
    LCD::write_cmd(0x06);   // entry mode
    LCD::write_cmd(0x01);   // clear

    delay(10);
}

void LCD::write(char data){
    digitalWrite(pin_chip_select_, LOW);
    digitalWrite(pin_register_select_, HIGH);
    delay(2);
    SPI.transfer(data);
    delay(2);
    digitalWrite(pin_chip_select_, HIGH);
}

void LCD::write(const char text[], byte addr) {
    addr += 0x80;   // additional bit set in DB7;
    LCD::write_cmd(addr);
    for(int i = 0; text[i] != '\0' && i < 16; i++){
        LCD::write(text[i]);
    }
}

void LCD::write(dtext_t text){
    LCD::write(text[0], 0x00);
    LCD::write(text[1], 0x40);
}

void LCD::write_cmd(byte data){
    digitalWrite(pin_chip_select_, LOW);
    digitalWrite(pin_register_select_, LOW);
    SPI.beginTransaction(SPI_settings_lcd_);
    delay(2);
    SPI.transfer(data);
    delay(2);
    SPI.endTransaction();
    digitalWrite(pin_chip_select_, HIGH);
}

void LCD::clear(){
    LCD::write_cmd(0x01);
}

void LCD::write(String data, byte addr) {
    LCD::write(data.c_str(), addr);
}

void LCD::clear_1st_line(){
    LCD::write("                ", 0x00);
}

void LCD::clear_2nd_line(){
    LCD::write("                ", 0x40);
}
