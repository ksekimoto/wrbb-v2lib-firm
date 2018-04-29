#!mruby
#
# How to use LcdSpi Class
# LcdSpi.New lcd_id
#   lcd_id
#   0: Nokia 6100 (PCF8833)
#   1: Nokia 6100 (S1D15G10)
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

LcdSpi = LcdSpi.new 0 
pl "LcdSpi.new()"
LcdSpi.set_font(1)
pl "LcdSpi.set_font()"
LcdSpi.puts("This is test\r\n")
LcdSpi.puts("GR-CITRUS LCD\n")
pl "LcdSpi.puts()"
