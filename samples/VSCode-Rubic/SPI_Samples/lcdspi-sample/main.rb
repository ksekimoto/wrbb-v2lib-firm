#!mruby
#
# How to use LcdSpi Class
# LcdSpi.New (lcd_id, spi_type, cs, clk, dout, reset, rs, din)
#   lcd_id
#   0: Nokia 6100 (PCF8833)
#   1: Nokia 6100 (S1D15G10)
#   spi_type
#   0: software
#   1: hardware
# LcdSpi.clear()
# LcdSpi.set_font(font_id)
#   font_id
#   0: MISAKIFONT4X8
#   1: MISAKIFONT6X12
# LcdSpi.putxy(x, y, c)
# LcdSpi.putc(c)
# LcdSpi.puts(string)
#
@usb = Serial.new 0
def p obj
    @usb.print obj.to_s
end
def pl obj
    @usb.println obj.to_s
end

if (!System.use?("SD")) then
    puts "SD Card can't use."
    System.exit 
end
puts "SD Ready"

# LCD Nokia 6100
#LcdSpi = LcdSpi.new(0, 0, 10, 13, 11, 6, -1, -1) 
# LCD M022C9340SPI (ILI9340)
# http://www.aitendo.com/product/11963
LcdSpi = LcdSpi.new(2, 1, 10, 13, 11, 6, 5, 12) 
pl "LcdSpi.new()"
LcdSpi.set_font(3)
pl "LcdSpi.set_font()"
LcdSpi.puts("GR-CITRUS SPILCD\r\n")
pl "LcdSpi.puts()"
#LcdSpi.pututf8("This is test\r\n")
LcdSpi.pututf8("mrubyの日本語表示テスト\r\n")
pl "LcdSpi.pututf8()"

#FileName = "/sif1201.bmp"
#FileName = "/citrus24.bmp"
#FileName = "/citrus24.jpg"
FileName = "/mif1203.jpg"
#FileName = "/PIC00.JPG"
if SD.exists(FileName) == 1 then
    pl FileName + " exists"
else
    pl FileName + " doesn't exist"
end

remain = Util.free_size()
pl remain
#LcdSpi.dispBmpSD(0, 0, FileName);
LcdSpi.dispJpegSD(10, 30, FileName);

remain = Util.free_size()
pl remain

pl "End"
