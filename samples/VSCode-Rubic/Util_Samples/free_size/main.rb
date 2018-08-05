#!mruby

@Usb = Serial.new(0, 115200)
def p obj
    @Usb.print obj.to_s
end
def pl obj
    @Usb.println obj.to_s
end

pl "Check Heap Size"

remain = Util.free_size()
pl remain.to_s
