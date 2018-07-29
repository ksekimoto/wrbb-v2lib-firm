#!mruby

@Usb = Serial.new(0, 115200)
@Se1 = Serial.new(1, 115200)
def p obj
    @Usb.print obj.to_s
end
def pl obj
    @Usb.println obj.to_s
end

pl "Serial Camera"
@sc = SerialCamera.new(1, 115200)
pl "Serial Camera initialize"
@sc.precapture(1)
@sc.capture
@sc.save("SCMR32.JPG")
pl "Serial Camera end"
