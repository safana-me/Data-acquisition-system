
#include "header.h"
#include <lpc21xx.h>

// Function Prototypes
void uart0_init(void);
void lcd_init(void);
void spi0_init(void);
void i2c_init(void);
void adc_init(void);
float read_temperature(void);
float read_ldr(void);
void set_rtc_time(void);
void read_rtc_time(unsigned char* h, unsigned char* m, unsigned char* d, unsigned char* mo, unsigned char* y);
void display_time(unsigned char h, unsigned char m, unsigned char d, unsigned char mo, unsigned char y);
void send_time_via_uart(unsigned char h, unsigned char m, unsigned char d, unsigned char mo, unsigned char y);

int main(void) {
    float temperature, light;
    unsigned char h, m, d, mo, y;

    uart0_init();
    lcd_init();
    spi0_init();
    i2c_init();
    adc_init();

    while (1) {
        // Read and process temperature
        temperature = read_temperature();
        uart0_tx_string("Temperature: ");
        uart0_tx_float(temperature);
        uart0_tx_string("\r\n");
        
        lcd_cmd(0x80);
        lcd_string("Temp:");
        lcd_data(((int)temperature / 10) + '0');
        lcd_data(((int)temperature % 10) + '0');

        // Read and process LDR value
        light = read_ldr();
        uart0_tx_string("Light: ");
        uart0_tx_float(light);
        uart0_tx_string("\r\n");
        
        lcd_string(" LIGHT:");
        lcd_data(((int)light / 10) + '0');
        lcd_data(((int)light % 10) + '0');
        lcd_data('%');

        delay_ms(50);

        // Set and read RTC time
        set_rtc_time();  // Set RTC time (once at the beginning or as required)
        read_rtc_time(&h, &m, &d, &mo, &y);
        display_time(h, m, d, mo, y);
        send_time_via_uart(h, m, d, mo, y);
        
        delay_ms(1000);
    }
}

// Function to read and convert temperature
float read_temperature(void) {
    uint16_t temp = adc_read(1);  // Read raw temperature data
    float vout = (temp * 3.3) / 1023;
    return (vout - 0.5) / 0.01;  // Convert to temperature in °C
}

// Function to read and convert LDR value
float read_ldr(void) {
    uint16_t temp1 = mcp3204_adc_read(2);  // Read LDR value from external ADC
    return 100.0f - ((temp1 * 100.0f) / 4095.0f);  // Convert to light percentage
}

// Function to set RTC time
void set_rtc_time(void) {
    i2c_byte_write_frame(0xD0, 0x01, 0x45);  // Set minutes
    i2c_byte_write_frame(0xD0, 0x02, 0x05);  // Set hours
    i2c_byte_write_frame(0xD0, 0x04, 0x23);  // Set date
    i2c_byte_write_frame(0xD0, 0x05, 0x07);  // Set month
    i2c_byte_write_frame(0xD0, 0x06, 0x24);  // Set year
}

// Function to read RTC time
void read_rtc_time(unsigned char* h, unsigned char* m, unsigned char* d, unsigned char* mo, unsigned char* y) {
    *h = i2c_byte_read_frame(0xD0, 0x02);  // Read hours
    *m = i2c_byte_read_frame(0xD0, 0x01);  // Read minutes
    *d = i2c_byte_read_frame(0xD0, 0x04);  // Read date
    *mo = i2c_byte_read_frame(0xD0, 0x05); // Read month
    *y = i2c_byte_read_frame(0xD0, 0x06);  // Read year
}

// Function to display RTC time on LCD
void display_time(unsigned char h, unsigned char m, unsigned char d, unsigned char mo, unsigned char y) {
    lcd_cmd(0xC0);
    
    if (h > 0x12 || (h == 0x12 && m > 0x00)) {  // 24 hr to 12 hr format
        h -= 0x12;
        lcd_data((h / 0x10) + '0');
        lcd_data((h % 0x10) + '0');
        lcd_data(':');
        lcd_data((m / 0x10) + '0');
        lcd_data((m % 0x10) + '0');
        lcd_string(" PM");
    } else {
        lcd_data((h / 0x10) + '0');
        lcd_data((h % 0x10) + '0');
        lcd_data(':');
        lcd_data((m / 0x10) + '0');
        lcd_data((m % 0x10) + '0');
        lcd_string(" AM");
    }

    lcd_data(' ');
    lcd_data((d / 0x10) + '0');
    lcd_data((d % 0x10) + '0');
    lcd_data('/');
    lcd_data((mo / 0x10) + '0');
    lcd_data((mo % 0x10) + '0');
    lcd_data('/');
    lcd_data((y / 0x10) + '0');
    lcd_data((y % 0x10) + '0'); 
}

// Function to send RTC time via UART
void send_time_via_uart(unsigned char h, unsigned char m, unsigned char d, unsigned char mo, unsigned char y) {
    if (h > 0x12 || (h == 0x12 && m > 0x00)) {  // 24 hr to 12 hr format
        h -= 0x12;
        uart0_tx((h / 0x10) + '0');
        uart0_tx((h % 0x10) + '0');
        uart0_tx(':');
        uart0_tx((m / 0x10) + '0');
        uart0_tx((m % 0x10) + '0');
        uart0_tx_string(" PM");
    } else {
        uart0_tx((h / 0x10) + '0');
        uart0_tx((h % 0x10) + '0');
        uart0_tx(':');
        uart0_tx((m / 0x10) + '0');
        uart0_tx((m % 0x10) + '0');
        uart0_tx_string(" AM");
    }

    uart0_tx(' ');
    uart0_tx((d / 0x10) + '0');
    uart0_tx((d % 0x10) + '0');
    uart0_tx('/');
    uart0_tx((mo / 0x10) + '0');
    uart0_tx((mo % 0x10) + '0');
    uart0_tx('/');
    uart0_tx((y / 0x10) + '0');
    uart0_tx((y % 0x10) + '0'); 
    uart0_tx_string("\r\n");
}

