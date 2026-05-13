#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ILI9341_TFTWIDTH	320
#define ILI9341_TFTHEIGHT	240

void lcd_set_pins(uint16_t dc, uint16_t cs, int16_t rst, uint16_t sck, uint16_t tx);

void lcd_init(void);

void lcd_set_rotation(uint8_t m);

void lcd_fill_screen(uint16_t color);

void lcd_draw_pixel(int16_t x, int16_t y, uint16_t color);

void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

void lcd_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color);

void lcd_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color);

void lcd_invert_display(bool invert);

uint16_t lcd_color565(uint8_t r, uint8_t g, uint8_t b);