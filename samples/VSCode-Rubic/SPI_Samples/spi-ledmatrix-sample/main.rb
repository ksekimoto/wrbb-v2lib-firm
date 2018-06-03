#GR-CITRUS

#
# SPI LED Matrix sample
# 
# Target Product 
# http://www.aitendo.com/product/11714 8x8マトリックスモジュールキット（1桁） [K-LED8X8M7219]
# https://www.banggood.com/MAX7219-Dot-Matrix-MCU-LED-Display-Control-Module-Kit-For-Arduino-p-915478.html?cur_warehouse=CN
#

# SPI constant
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

# SPI LED Matrix using SPILM
# DIN 11 pin
# CS 10 pin
# CLK 13 pin
@REG_NO_OP = 0x00
@REG_DIGIT0 = 0x01
@REG_DIGIT1 = 0x02
@REG_DIGIT2 = 0x03
@REG_DIGIT3 = 0x04
@REG_DIGIT4 = 0x05
@REG_DIGIT5 = 0x06
@REG_DIGIT6 = 0x07
@REG_DIGIT7 = 0x08
@REG_DECODE_MODE = 0x09
@REG_INTENSITY  = 0x0a
@REG_SCAN_LIMIT = 0x0b
@REG_SHUTDOWN = 0x0c
@REG_DISPLAY_TEST = 0x0f
@CS = @CS0
@STR1 = "ｍＲｕｂｙ試験"

@usb = Serial.new 0
# デバッグ用のpメソッド
def p obj
    @usb.print obj.to_s
end
def pl obj
    @usb.println obj.to_s
end

def writeSPILM pinCS, reg, val
    s = "  "
    s[0] = reg.chr
    s[1] = val.chr
    digitalWrite(pinCS, LOW)
    SPILM.transfers(s, 2)
    digitalWrite(pinCS, HIGH)
end

def displayArraySPILM pinCS, ary
    ary.each_with_index do |x, i|
        #pl "#{i+1}: #{x}"
        writeSPILM pinCS, i+1, x
    end
end

def displayUnicodeSPILM pinCS, index
    a = "        "
    a = FONT.data(index)
    i = 0
    8.times do
        #pl "#{i+1}: #{a[i]}"
        writeSPILM pinCS, i+1, a[i]
        i = i+1
    end
end

def testmodeSPILM pinCS
    5.times do
        writeSPILM pinCS, @REG_DISPLAY_TEST, 0x01
        pl "Test Mode"
        delay(300)
        writeSPILM pinCS, @REG_DISPLAY_TEST, 0x00
        pl "Normal Mode"
        delay(300)
    end
end

def initSPILM pinCS
    writeSPILM pinCS, @REG_SHUTDOWN, 0x01
    pl "Normal Mode"
    writeSPILM pinCS, @REG_DECODE_MODE, 0x00
    pl "Decode Mode"
    writeSPILM pinCS, @REG_SCAN_LIMIT, 0x07
    pl "Scan limit = 0x07"
end

def displayUnicodeStrSPILM pinCS, str
    #pl "str size = " + str.length.to_s
    unicode = FONT.cnvUtf8ToUnicode(str, str.length)
    i = 0
    size = unicode.size / 2
    size.times do
        u = FONT.getUnicodeAtIndex(unicode, i)
        pl u.to_s(16) 
        displayUnicodeSPILM pinCS, u
        i = i + 1
        delay(500)
    end
end

FONT = Font.new(2)

pinCS = @CS0
SPILM = Spi.new()
pinMode(pinCS, OUTPUT)
digitalWrite(pinCS, HIGH)

digitalWrite(pinCS, LOW)
SPILM.beginTransaction(100000, @MSBFIRST, @SPI_MODE0)

initSPILM pinCS
displayArraySPILM pinCS, [255, 129, 189, 165, 165, 189, 129, 255]
testmodeSPILM pinCS

str = @STR1
displayUnicodeStrSPILM pinCS, str

SPILM.endTransaction()
digitalWrite(pinCS, HIGH)
