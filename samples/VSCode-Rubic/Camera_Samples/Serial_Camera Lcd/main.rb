#!mruby

@Usb = Serial.new(0, 115200)
@Se1 = Serial.new(1, 115200)
def p obj
    @Usb.print obj.to_s
end
def pl obj
    @Usb.println obj.to_s
end

if (!System.use?("SD")) then
    puts "SD Card can't use."
    System.exit 
end
puts "SD Ready"

FileName = "/SCMR32.JPG"

pl "Serial Camera"
@sc = SerialCamera.new(1, 115200)
pl "Serial Camera initialize"
@sc.precapture(1)
@sc.capture
@sc.save(FileName)
pl "Serial Camera end"

# LCD Nokia 6100
#LcdSpi = LcdSpi.new(0, 0, 10, 13, 11, 6, -1, -1) 
# LCD M022C9340SPI (ILI9340)
# http://www.aitendo.com/product/11963
LcdSpi = LcdSpi.new(2, 1, 10, 13, 11, 6, 5, 12) 
pl "LcdSpi.new()"
LcdSpi.set_font(1)
pl "LcdSpi.set_font()"
LcdSpi.puts("This is test\r\n")
LcdSpi.puts("GR-CITRUS LCD\n")
pl "LcdSpi.puts()"

if SD.exists(FileName) == 1 then
    pl FileName + " exists"
else
    pl FileName + " doesn't exist"
end

#LcdSpi.dispBmpSD(0, 0, FileName);
LcdSpi.dispJpegSD(20, 20, FileName);

pl "End"
