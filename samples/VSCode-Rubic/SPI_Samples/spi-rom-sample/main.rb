#GR-CITRUS

@LSBFIRST = 0
@MSBFIRST = 1
@SPI_MODE0 = 0x00
@SPI_MODE1 = 0x04
@SPI_MODE2 = 0x08
@SPI_MODE3 = 0x0c
@CS0 = 10
@CS1 = 2
@CS2 = 3
@CS3 = 4
@M25P32_CS = @CS0
@STATUS_REGISTER_READ = 0x05
@READ_MANUFACTURER_AND_DEVICE_ID = 0x9f
@SPI_DUMMY = 0xff

@usb = Serial.new 0
# デバッグ用のpメソッド
def p obj
    @usb.print obj.to_s
end
def pl obj
    @usb.println obj.to_s
end

def SpiRomReadManufactureID pinCS
    M25P32 = Spi.new()
    pl "Spi.new"
    pinMode(pinCS, OUTPUT)
    digitalWrite(pinCS, HIGH)
    
    digitalWrite(pinCS, LOW)
    M25P32.beginTransaction(250000, @MSBFIRST, @SPI_MODE0)
    pl "Spi.beginTransaction"
    val0 = M25P32.transfer(@READ_MANUFACTURER_AND_DEVICE_ID)
    val1 = M25P32.transfer(@SPI_DUMMY)
    val2 = M25P32.transfer(@SPI_DUMMY)
    val3 = M25P32.transfer(@SPI_DUMMY)
    pl "Manufacture ID = " + val1.to_s + ":" + val2.to_s + ":" + val3.to_s
    M25P32.endTransaction()
    digitalWrite(pinCS, HIGH)
end

SpiRomReadManufactureID @M25P32_CS





