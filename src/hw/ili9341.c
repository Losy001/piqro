#include "ili9341.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

#define TFT_NOP			0x00
#define TFT_SWRST		0x01
#define TFT_SLPIN		0x10
#define TFT_SLPOUT		0x11
#define TFT_INVOFF		0x20
#define TFT_INVON		0x21
#define TFT_DISPOFF		0x28
#define TFT_DISPON		0x29
#define TFT_CASET		0x2A
#define TFT_PASET		0x2B
#define TFT_RAMWR		0x2C
#define TFT_MADCTL		0x36
#define TFT_PIXFMT		0x3A

#define TFT_MAD_MY		0x80
#define TFT_MAD_MX		0x40
#define TFT_MAD_MV		0x20
#define TFT_MAD_ML		0x10
#define TFT_MAD_BGR		0x08
#define TFT_MAD_MH		0x04
#define TFT_MAD_RGB		0x00

static spi_inst_t *spi = spi1;
static uint16_t dc_pin;
static uint16_t cs_pin;
static int16_t rst_pin;
static uint16_t _width = ILI9341_TFTWIDTH;
static uint16_t _height = ILI9341_TFTHEIGHT;
static uint8_t _rotation = 0;

static inline void dc_command(void)
{
	gpio_put(dc_pin, 0);
}

static inline void dc_data(void)
{
	gpio_put(dc_pin, 1);
}

static inline void cs_low(void)
{
	gpio_put(cs_pin, 0);
}

static inline void cs_high(void)
{
	gpio_put(cs_pin, 1);
}

static void write_command(uint8_t cmd)
{
	dc_command();
	cs_low();
	spi_write_blocking(spi, &cmd, 1);
	cs_high();
}

static void write_data(uint8_t data)
{
	dc_data();
	cs_low();
	spi_write_blocking(spi, &data, 1);
	cs_high();
}

static void write_data16(uint16_t data)
{
	uint8_t buf[2] = {data >> 8, data & 0xFF};
	dc_data();
	cs_low();
	spi_write_blocking(spi, buf, 2);
	cs_high();
}

static void set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

	write_command(TFT_CASET);
	dc_data();
	cs_low();
	uint8_t buf[4];
	buf[0] = xa >> 24;
	buf[1] = xa >> 16;
	buf[2] = xa >> 8;
	buf[3] = xa;
	spi_write_blocking(spi, buf, 4);
	cs_high();

	write_command(TFT_PASET);
	dc_data();
	cs_low();
	buf[0] = ya >> 24;
	buf[1] = ya >> 16;
	buf[2] = ya >> 8;
	buf[3] = ya;
	spi_write_blocking(spi, buf, 4);
	cs_high();

	write_command(TFT_RAMWR);
}

void lcd_set_pins(uint16_t dc, uint16_t cs, int16_t rst, uint16_t sck, uint16_t tx)
{
	dc_pin = dc;
	cs_pin = cs;
	rst_pin = rst;

	gpio_init(cs_pin);
	gpio_set_dir(cs_pin, GPIO_OUT);
	gpio_put(cs_pin, 1);

	gpio_init(dc_pin);
	gpio_set_dir(dc_pin, GPIO_OUT);
	gpio_put(dc_pin, 1);

	if (rst_pin >= 0)
	{
		gpio_init(rst_pin);
		gpio_set_dir(rst_pin, GPIO_OUT);
		gpio_put(rst_pin, 1);
	}

	spi_init(spi, 40000000);
	spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
	
	gpio_set_function(tx, GPIO_FUNC_SPI);
	gpio_set_function(sck, GPIO_FUNC_SPI);
}

void lcd_init(void)
{
	if (rst_pin >= 0)
	{
		gpio_put(rst_pin, 1);
		sleep_ms(5);
		gpio_put(rst_pin, 0);
		sleep_ms(20);
		gpio_put(rst_pin, 1);
		sleep_ms(150);
	}

	write_command(0xEF);
	write_data(0x03);
	write_data(0x80);
	write_data(0x02);

	write_command(0xCF);
	write_data(0x00);
	write_data(0XC1);
	write_data(0X30);

	write_command(0xED);
	write_data(0x64);
	write_data(0x03);
	write_data(0X12);
	write_data(0X81);

	write_command(0xE8);
	write_data(0x85);
	write_data(0x00);
	write_data(0x78);

	write_command(0xCB);
	write_data(0x39);
	write_data(0x2C);
	write_data(0x00);
	write_data(0x34);
	write_data(0x02);

	write_command(0xF7);
	write_data(0x20);

	write_command(0xEA);
	write_data(0x00);
	write_data(0x00);

	write_command(0xC0);
	write_data(0x23);

	write_command(0xC1);
	write_data(0x10);

	write_command(0xC5);
	write_data(0x3e);
	write_data(0x28);

	write_command(0xC7);
	write_data(0x86);

	write_command(0x36);
	write_data(0x48);

	write_command(0x3A);
	write_data(0x55);

	write_command(0xB1);
	write_data(0x00);
	write_data(0x18);

	write_command(0xB6);
	write_data(0x08);
	write_data(0x82);
	write_data(0x27);

	write_command(0xF2);
	write_data(0x00);

	write_command(0x26);
	write_data(0x01);

	write_command(0xE0);
	write_data(0x0F);
	write_data(0x31);
	write_data(0x2B);
	write_data(0x0C);
	write_data(0x0E);
	write_data(0x08);
	write_data(0x4E);
	write_data(0xF1);
	write_data(0x37);
	write_data(0x07);
	write_data(0x10);
	write_data(0x03);
	write_data(0x0E);
	write_data(0x09);
	write_data(0x00);

	write_command(0XE1);
	write_data(0x00);
	write_data(0x0E);
	write_data(0x14);
	write_data(0x03);
	write_data(0x11);
	write_data(0x07);
	write_data(0x31);
	write_data(0xC1);
	write_data(0x48);
	write_data(0x08);
	write_data(0x0F);
	write_data(0x0C);
	write_data(0x31);
	write_data(0x36);
	write_data(0x0F);

	write_command(0x11);
	sleep_ms(120);
	
	write_command(0x29);
	sleep_ms(20);
}

void lcd_set_rotation(uint8_t m)
{
	_rotation = m % 4;
	write_command(TFT_MADCTL);
	
	switch (_rotation)
	{
		case 0:
		{
			write_data(TFT_MAD_MX | TFT_MAD_BGR);
			_width = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
		} break;
		
		case 1:
		{
			write_data(TFT_MAD_MV | TFT_MAD_BGR);
			_width = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
		}	break;
		
		case 2:
		{
			write_data(TFT_MAD_MY | TFT_MAD_BGR);
			_width = ILI9341_TFTWIDTH;
			_height = ILI9341_TFTHEIGHT;
		} break;
		
		case 3:
		{
			write_data(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_BGR);
			_width = ILI9341_TFTHEIGHT;
			_height = ILI9341_TFTWIDTH;
		} break;
	}
}

void lcd_fill_screen(uint16_t color)
{
	lcd_fill_rect(0, 0, _width, _height, color);
}

void lcd_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
	{
		return;
	}

	set_addr_window(x, y, 1, 1);
	
	uint8_t buf[2] = {color >> 8, color & 0xFF};
	dc_data();
	cs_low();
	spi_write_blocking(spi, buf, 2);
	cs_high();
}

void lcd_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	if ((x >= _width) || (y >= _height) || w <= 0 || h <= 0)
	{
		return;
	}
	
	if ((x + w - 1) >= _width)
	{
		w = _width - x;
	}
	if ((y + h - 1) >= _height)
	{
		h = _height - y;
	}

	set_addr_window(x, y, w, h);

	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;

	dc_data();
	cs_low();
	
	uint32_t num = (uint32_t)w * (uint32_t)h;
	
	for (uint32_t i = 0; i < num; i++)
	{
		spi_write_blocking(spi, &hi, 1);
		spi_write_blocking(spi, &lo, 1);
	}
	
	cs_high();
}

void lcd_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	if ((x >= _width) || (y >= _height) || h <= 0)
	{
		return;
	}
	
	if ((y + h - 1) >= _height)
	{
		h = _height - y;
	}

	set_addr_window(x, y, 1, h);

	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;

	dc_data();
	cs_low();
	
	for (int16_t i = 0; i < h; i++)
	{
		spi_write_blocking(spi, &hi, 1);
		spi_write_blocking(spi, &lo, 1);
	}
	
	cs_high();
}

void lcd_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	if ((x >= _width) || (y >= _height) || w <= 0)
	{
		return;
	}
	
	if ((x + w - 1) >= _width)
	{
		w = _width - x;
	}

	set_addr_window(x, y, w, 1);

	uint8_t hi = color >> 8;
	uint8_t lo = color & 0xFF;

	dc_data();
	cs_low();
	
	for (int16_t i = 0; i < w; i++)
	{
		spi_write_blocking(spi, &hi, 1);
		spi_write_blocking(spi, &lo, 1);
	}
	
	cs_high();
}

void lcd_invert_display(bool invert)
{
	write_command(invert ? TFT_INVON : TFT_INVOFF);
}

uint16_t lcd_color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}