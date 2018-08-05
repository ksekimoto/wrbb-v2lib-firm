#TARGET_BOARD = GRCITRUS
TARGET_BOARD = GRCITRUS_JTAG
#TARGET_BOARD = GRSAKURA_128
#TARGET_BOARD = GRSAKURA_128_JTAG
#TARGET_BOARD = GRSAKURA_256

INC_SPI = 1
INC_FONT = 1
INC_LCDSPI = 1
INC_JPEG = 1
INC_SERIALCAMERA = 1
#INC_TWITTER = 1
INC_UTIL = 1

SRCFILES = ./gr_sketch.cpp 
SRCFILES += ./gr_common/core/HardwareSerial.cpp 
SRCFILES += ./gr_common/core/main.cpp 
SRCFILES += ./gr_common/core/new.cpp 
SRCFILES += ./gr_common/core/Print.cpp 
SRCFILES += ./gr_common/core/Stream.cpp 
SRCFILES += ./gr_common/core/Tone.cpp 
SRCFILES += ./gr_common/core/usbdescriptors.c 
SRCFILES += ./gr_common/core/usb_cdc.c 
SRCFILES += ./gr_common/core/usb_core.c 
SRCFILES += ./gr_common/core/usb_hal.c 
SRCFILES += ./gr_common/core/utilities.cpp 
SRCFILES += ./gr_common/core/WInterrupts.c 
SRCFILES += ./gr_common/core/wiring.c 
SRCFILES += ./gr_common/core/wiring_analog.c 
SRCFILES += ./gr_common/core/wiring_digital.c 
SRCFILES += ./gr_common/core/wiring_pulse.c 
SRCFILES += ./gr_common/core/wiring_shift.c 
SRCFILES += ./gr_common/core/WMath.cpp 
SRCFILES += ./gr_common/core/WString.cpp 
SRCFILES += ./gr_common/core/avr/avrlib.c 
SRCFILES += ./gr_common/lib/DSP/DSP.cpp 
SRCFILES += ./gr_common/lib/EEPROM/EEPROM.cpp 
SRCFILES += ./gr_common/lib/EEPROM/utility/r_flash_api_rx600.c 
SRCFILES += ./gr_common/lib/Firmata/Firmata.cpp 
SRCFILES += ./gr_common/lib/LiquidCrystal/LiquidCrystal.cpp 
SRCFILES += ./gr_common/lib/MsTimer2/MsTimer2.cpp 
SRCFILES += ./gr_common/lib/RTC/RTC.cpp 
SRCFILES += ./gr_common/lib/RTC/utility/RX63_RTC.cpp 
SRCFILES += ./gr_common/lib/SD/File.cpp 
SRCFILES += ./gr_common/lib/SD/SD.cpp 
SRCFILES += ./gr_common/lib/SD/utility/Sd2Card.cpp 
SRCFILES += ./gr_common/lib/SD/utility/SdFile.cpp 
SRCFILES += ./gr_common/lib/SD/utility/SdVolume.cpp 
SRCFILES += ./gr_common/lib/Servo/Servo.cpp 
SRCFILES += ./gr_common/lib/SoftwareSerial/SoftwareSerial.cpp 
SRCFILES += ./gr_common/lib/SPI/SPI.cpp 
SRCFILES += ./gr_common/lib/Stepper/Stepper.cpp 
SRCFILES += ./gr_common/lib/Wire/Wire.cpp 
SRCFILES += ./gr_common/lib/Wire/utility/I2cMaster.cpp 
SRCFILES += ./gr_common/lib/Wire/utility/twi_rx.c 
SRCFILES += ./gr_common/rx63n/exception_handler.cpp 
SRCFILES += ./gr_common/rx63n/hardware_setup.cpp 
SRCFILES += ./gr_common/rx63n/interrupt_handlers.c 
SRCFILES += ./gr_common/rx63n/reboot.c 
SRCFILES += ./gr_common/rx63n/reset_program.asm 
SRCFILES += ./gr_common/rx63n/util.c 
SRCFILES += ./gr_common/rx63n/vector_table.c
SRCFILES += ./gr_common/rx63n/sbrk.c
SRCFILES += ./wrbb_eepfile/eepfile.cpp 
SRCFILES += ./wrbb_eepfile/eeploader.cpp
SRCFILES += ./wrbb_mruby/sExec.cpp
ifdef INC_FONT
SRCFILES += ./wrbb_mruby/sFont.cpp 
endif
SRCFILES += ./wrbb_mruby/sI2c.cpp 
ifdef INC_FONT
SRCFILES += ./wrbb_mruby/sJpeg.cpp
SRCFILES += ./wrbb_mruby/picojpeg.c
endif
SRCFILES += ./wrbb_mruby/sKernel.cpp  
ifdef INC_LCDSPI
SRCFILES += ./wrbb_mruby/sLcdSpi.cpp 
endif
SRCFILES += ./wrbb_mruby/sMem.cpp 
SRCFILES += ./wrbb_mruby/sRtc.cpp 
SRCFILES += ./wrbb_mruby/sSdCard.cpp 
SRCFILES += ./wrbb_mruby/sSerial.cpp 
ifdef INC_SERIALCAMERA
SRCFILES += ./wrbb_mruby/sSerialCamera.cpp 
endif
ifdef INC_SPI
SRCFILES += ./wrbb_mruby/sSpi.cpp 
endif
SRCFILES += ./wrbb_mruby/sSys.cpp
ifdef INC_TWITTER 
SRCFILES += ./wrbb_mruby/sTwitter.cpp 
endif
ifdef INC_UTIL
SRCFILES += ./wrbb_mruby/sUtil.cpp 
endif
SRCFILES += ./wrbb_mruby/sWiFi.cpp 
ifdef INC_MP3
SRCFILES += ./wrbb_mruby/sMp3.cpp 
endif
SRCFILES += ./wrbb_mruby/sGlobal.cpp
ifdef INC_MP3
SRCFILES += ./WavMp3p/WavMp3p.cpp
SRCFILES += ./WavMp3p/libmad-0.15.1b/bit.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/decoder.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/fixed.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/frame.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/huffman.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/layer12.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/layer3.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/minimad.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/stream.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/synth.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/timer.c 
SRCFILES += ./WavMp3p/libmad-0.15.1b/version.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_audio.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_ctrl.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_dma.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_gpio.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_icu.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_init.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_play.cpp 
SRCFILES += ./WavMp3p/utility/wavmp3p_play_mp3.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_play_wav.c 
SRCFILES += ./WavMp3p/utility/wavmp3p_pwm.c
endif

# OBJFILES

OBJFILES = ./gr_sketch.o 
OBJFILES += ./gr_common/core/HardwareSerial.o 
OBJFILES += ./gr_common/core/main.o 
OBJFILES += ./gr_common/core/new.o 
OBJFILES += ./gr_common/core/Print.o 
OBJFILES += ./gr_common/core/Stream.o 
OBJFILES += ./gr_common/core/Tone.o 
OBJFILES += ./gr_common/core/utilities.o 
OBJFILES += ./gr_common/core/WMath.o 
OBJFILES += ./gr_common/core/WString.o 
OBJFILES += ./gr_common/lib/DSP/DSP.o 
OBJFILES += ./gr_common/lib/EEPROM/EEPROM.o 
OBJFILES += ./gr_common/lib/Firmata/Firmata.o 
OBJFILES += ./gr_common/lib/LiquidCrystal/LiquidCrystal.o 
OBJFILES += ./gr_common/lib/MsTimer2/MsTimer2.o 
OBJFILES += ./gr_common/lib/RTC/RTC.o 
OBJFILES += ./gr_common/lib/RTC/utility/RX63_RTC.o 
OBJFILES += ./gr_common/lib/SD/File.o 
OBJFILES += ./gr_common/lib/SD/SD.o 
OBJFILES += ./gr_common/lib/SD/utility/Sd2Card.o 
OBJFILES += ./gr_common/lib/SD/utility/SdFile.o 
OBJFILES += ./gr_common/lib/SD/utility/SdVolume.o 
OBJFILES += ./gr_common/lib/Servo/Servo.o 
OBJFILES += ./gr_common/lib/SoftwareSerial/SoftwareSerial.o 
OBJFILES += ./gr_common/lib/SPI/SPI.o 
OBJFILES += ./gr_common/lib/Stepper/Stepper.o 
OBJFILES += ./gr_common/lib/Wire/Wire.o 
OBJFILES += ./gr_common/lib/Wire/utility/I2cMaster.o 
OBJFILES += ./gr_common/rx63n/exception_handler.o 
OBJFILES += ./gr_common/rx63n/hardware_setup.o
OBJFILES += ./wrbb_eepfile/eepfile.o
OBJFILES += ./wrbb_eepfile/eeploader.o
OBJFILES += ./wrbb_mruby/sExec.o
ifdef INC_FONT
OBJFILES += ./wrbb_mruby/sFont.o
endif
OBJFILES += ./wrbb_mruby/sGlobal.o
OBJFILES += ./wrbb_mruby/sI2c.o
ifdef INC_JPEG
OBJFILES += ./wrbb_mruby/sJpeg.o
OBJFILES += ./wrbb_mruby/picojpeg.o
endif
OBJFILES += ./wrbb_mruby/sKernel.o
ifdef INC_LCDSPI
OBJFILES += ./wrbb_mruby/sLcdSpi.o
endif
OBJFILES += ./wrbb_mruby/sMem.o
OBJFILES += ./wrbb_mruby/sRtc.o
OBJFILES += ./wrbb_mruby/sSdCard.o
OBJFILES += ./wrbb_mruby/sSerial.o
ifdef INC_SERIALCAMERA
OBJFILES += ./wrbb_mruby/sSerialCamera.o 
endif
OBJFILES += ./wrbb_mruby/sServo.o
ifdef INC_SPI
OBJFILES += ./wrbb_mruby/sSpi.o
endif
OBJFILES += ./wrbb_mruby/sSys.o
ifdef INC_TWITTER 
OBJFILES += ./wrbb_mruby/sTwitter.o
endif
ifdef INC_UTIL 
OBJFILES += ./wrbb_mruby/sUtil.o
endif
OBJFILES += ./wrbb_mruby/sWiFi.o
ifdef INC_MP3
OBJFILES += ./wrbb_mruby/sMp3.o
OBJFILES += ./WavMp3p/WavMp3p.o
OBJFILES += ./WavMp3p/utility/wavmp3p_play.o
endif
OBJFILES += ./wrbb_mruby/sDCMotor.o
OBJFILES += ./gr_common/core/usbdescriptors.o
OBJFILES += ./gr_common/core/usb_cdc.o
OBJFILES += ./gr_common/core/usb_core.o
OBJFILES += ./gr_common/core/usb_hal.o
OBJFILES += ./gr_common/core/WInterrupts.o
OBJFILES += ./gr_common/core/wiring.o
OBJFILES += ./gr_common/core/wiring_analog.o
OBJFILES += ./gr_common/core/wiring_digital.o
OBJFILES += ./gr_common/core/wiring_pulse.o
OBJFILES += ./gr_common/core/wiring_shift.o
OBJFILES += ./gr_common/core/avr/avrlib.o
OBJFILES += ./gr_common/lib/EEPROM/utility/r_flash_api_rx600.o
OBJFILES += ./gr_common/lib/Wire/utility/twi_rx.o
OBJFILES += ./gr_common/rx63n/interrupt_handlers.o
OBJFILES += ./gr_common/rx63n/reboot.o
OBJFILES += ./gr_common/rx63n/util.o
OBJFILES += ./gr_common/rx63n/vector_table.o
OBJFILES += ./gr_common/rx63n/sbrk.o
ifdef INC_MP3
OBJFILES += ./WavMp3p/libmad-0.15.1b/bit.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/decoder.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/fixed.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/frame.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/huffman.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/layer12.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/layer3.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/minimad.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/stream.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/synth.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/timer.o
OBJFILES += ./WavMp3p/libmad-0.15.1b/version.o
OBJFILES += ./WavMp3p/utility/wavmp3p_audio.o
OBJFILES += ./WavMp3p/utility/wavmp3p_ctrl.o
OBJFILES += ./WavMp3p/utility/wavmp3p_dma.o
OBJFILES += ./WavMp3p/utility/wavmp3p_gpio.o
OBJFILES += ./WavMp3p/utility/wavmp3p_icu.o
OBJFILES += ./WavMp3p/utility/wavmp3p_init.o
OBJFILES += ./WavMp3p/utility/wavmp3p_play_mp3.o
OBJFILES += ./WavMp3p/utility/wavmp3p_play_wav.o
OBJFILES += ./WavMp3p/utility/wavmp3p_pwm.o
endif
OBJFILES += ./gr_common/rx63n/reset_program.o

LIBFILES = ./gr_common/lib/DSP/utility/libGNU_RX_DSP_Little.a
LIBFILES += ./wrbb_mruby/libmruby.a

CCINC = -I./gr_build
CCINC += -I./gr_common 
CCINC += -I./gr_common/core 
CCINC += -I./gr_common/core/avr 
CCINC += -I./gr_common/lib 
CCINC += -I./gr_common/lib/DSP 
CCINC += -I./gr_common/lib/DSP/utility 
CCINC += -I./gr_common/lib/EEPROM 
CCINC += -I./gr_common/lib/EEPROM/utility 
CCINC += -I./gr_common/lib/Firmata 
CCINC += -I./gr_common/lib/LiquidCrystal 
CCINC += -I./gr_common/lib/MsTimer2 
CCINC += -I./gr_common/lib/RTC 
CCINC += -I./gr_common/lib/RTC/utility 
CCINC += -I./gr_common/lib/SD 
CCINC += -I./gr_common/lib/SD/utility 
CCINC += -I./gr_common/lib/Servo 
CCINC += -I./gr_common/lib/SoftwareSerial 
CCINC += -I./gr_common/lib/SPI 
CCINC += -I./gr_common/lib/Stepper 
CCINC += -I./gr_common/lib/Wire 
CCINC += -I./gr_common/lib/Wire/utility 
CCINC += -I./gr_common/rx63n 
CCINC += -I./USB_Driver
CCINC += -I./wrbb_eepfile 
CCINC += -I./wrbb_mruby 
CCINC += -I./wrbb_mruby/include 
CCINC += -I./wrbb_mruby/include/mruby
ifdef INC_MP3
CCINC += -I./WavMp3p 
CCINC += -I./WavMp3p/libmad-0.15.1b 
CCINC += -I./WavMp3p/libmad-0.15.1b/msvc++ 
CCINC += -I./WavMp3p/utility
endif

HEADERFILES = ./wrbb.h 
HEADERFILES += ./gr_common/core/Arduino.h 
HEADERFILES += ./gr_common/core/binary.h
HEADERFILES += ./gr_common/core/HardwareSerial.h 
HEADERFILES += ./gr_common/core/HardwareSerial_private.h
HEADERFILES += ./gr_common/core/new.h 
HEADERFILES += ./gr_common/core/pins_arduino.h 
HEADERFILES += ./gr_common/core/Print.h 
HEADERFILES += ./gr_common/core/Printable.h 
HEADERFILES += ./gr_common/core/Stream.h 
HEADERFILES += ./gr_common/core/Types.h 
HEADERFILES += ./gr_common/core/usbdescriptors.h 
HEADERFILES += ./gr_common/core/usb_cdc.h 
HEADERFILES += ./gr_common/core/usb_common.h 
HEADERFILES += ./gr_common/core/usb_core.h 
HEADERFILES += ./gr_common/core/usb_hal.h 
HEADERFILES += ./gr_common/core/utilities.h 
HEADERFILES += ./gr_common/core/WCharacter.h 
HEADERFILES += ./gr_common/core/wiring_private.h 
HEADERFILES += ./gr_common/core/WString.h 
HEADERFILES += ./gr_common/core/avr/avrlib.h 
HEADERFILES += ./gr_common/core/avr/pgmspace.h 
HEADERFILES += ./gr_common/lib/DSP/DSP.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_complex.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_filters.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_matrix.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_statistical.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_transform.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_typedefs.h 
HEADERFILES += ./gr_common/lib/DSP/utility/r_dsp_types.h 
HEADERFILES += ./gr_common/lib/EEPROM/EEPROM.h 
HEADERFILES += ./gr_common/lib/EEPROM/utility/r_flash_api_rx600.h 
HEADERFILES += ./gr_common/lib/Firmata/Boards.h 
HEADERFILES += ./gr_common/lib/Firmata/Firmata.h 
HEADERFILES += ./gr_common/lib/LiquidCrystal/LiquidCrystal.h 
HEADERFILES += ./gr_common/lib/MsTimer2/MsTimer2.h 
HEADERFILES += ./gr_common/lib/RTC/RTC.h 
HEADERFILES += ./gr_common/lib/RTC/utility/RX63_RTC.h 
HEADERFILES += ./gr_common/lib/SD/SD.h 
HEADERFILES += ./gr_common/lib/SD/utility/FatStructs.h 
HEADERFILES += ./gr_common/lib/SD/utility/Sd2Card.h 
HEADERFILES += ./gr_common/lib/SD/utility/Sd2PinMap.h 
HEADERFILES += ./gr_common/lib/SD/utility/SdFat.h 
HEADERFILES += ./gr_common/lib/SD/utility/SdFatmainpage.h 
HEADERFILES += ./gr_common/lib/SD/utility/SdFatUtil.h 
HEADERFILES += ./gr_common/lib/SD/utility/SdInfo.h 
HEADERFILES += ./gr_common/lib/Servo/Servo.h 
HEADERFILES += ./gr_common/lib/SoftwareSerial/SoftwareSerial.h 
HEADERFILES += ./gr_common/lib/SPI/SPI.h 
HEADERFILES += ./gr_common/lib/Stepper/Stepper.h 
HEADERFILES += ./gr_common/lib/Wire/Wire.h 
HEADERFILES += ./gr_common/lib/Wire/utility/I2cMaster.h 
HEADERFILES += ./gr_common/lib/Wire/utility/twi_rx.h 
HEADERFILES += ./gr_common/rx63n/interrupt_handlers.h 
HEADERFILES += ./gr_common/rx63n/iodefine.h 
HEADERFILES += ./gr_common/rx63n/iodefine_gcc63n.h 
HEADERFILES += ./gr_common/rx63n/reboot.h 
HEADERFILES += ./gr_common/rx63n/rx63n_stdio.h 
HEADERFILES += ./gr_common/rx63n/specific_instructions.h 
HEADERFILES += ./gr_common/rx63n/typedefine.h 
HEADERFILES += ./gr_common/rx63n/user_interrupt.h 
HEADERFILES += ./gr_common/rx63n/util.h
HEADERFILES += ./wrbb_eepfile/eepfile.h 
HEADERFILES += ./wrbb_eepfile/eeploader.h 
HEADERFILES += ./wrbb_mruby/sExec.h 
ifdef INC_FONT
HEADERFILES += ./wrbb_mruby/sFont.h 
endif
HEADERFILES += ./wrbb_mruby/sI2c.h 
ifdef INC_JPEG
HEADERFILES += ./wrbb_mruby/sJpeg.h
HEADERFILES += ./wrbb_mruby/picojpeg.h
endif
HEADERFILES += ./wrbb_mruby/sKernel.h 
ifdef INC_LCDSPI
HEADERFILES += ./wrbb_mruby/sLcdSpi.h 
endif
HEADERFILES += ./wrbb_mruby/sMem.h 
HEADERFILES += ./wrbb_mruby/sRtc.h 
HEADERFILES += ./wrbb_mruby/sSdCard.h 
HEADERFILES += ./wrbb_mruby/sSerial.h 
ifdef INC_SERIALCAMERA
HEADERFILES += ./wrbb_mruby/sSerialCamera.h
endif
HEADERFILES += ./wrbb_mruby/sServo.h 
ifdef INC_SPI
HEADERFILES += ./wrbb_mruby/sSpi.h 
endif
HEADERFILES += ./wrbb_mruby/sSys.h
ifdef INC_TWITTER 
HEADERFILES += ./wrbb_mruby/sTwitter.h 
endif
ifdef INC_UTIL
HEADERFILES += ./wrbb_mruby/sUtil.h 
endif
HEADERFILES += ./wrbb_mruby/sWiFi.h 
ifdef INC_MP3
HEADERFILES += ./wrbb_mruby/sMp3.h 
endif
HEADERFILES += ./wrbb_mruby/sGlobal.h
HEADERFILES += ./wrbb_mruby/include/mrbconf.h 
HEADERFILES += ./wrbb_mruby/include/mruby.h 
HEADERFILES += ./wrbb_mruby/include/mruby/array.h 
HEADERFILES += ./wrbb_mruby/include/mruby/boxing_nan.h 
HEADERFILES += ./wrbb_mruby/include/mruby/boxing_no.h 
HEADERFILES += ./wrbb_mruby/include/mruby/boxing_word.h 
HEADERFILES += ./wrbb_mruby/include/mruby/class.h 
HEADERFILES += ./wrbb_mruby/include/mruby/common.h 
HEADERFILES += ./wrbb_mruby/include/mruby/compile.h 
HEADERFILES += ./wrbb_mruby/include/mruby/data.h 
HEADERFILES += ./wrbb_mruby/include/mruby/debug.h 
HEADERFILES += ./wrbb_mruby/include/mruby/dump.h 
HEADERFILES += ./wrbb_mruby/include/mruby/error.h 
HEADERFILES += ./wrbb_mruby/include/mruby/gc.h 
HEADERFILES += ./wrbb_mruby/include/mruby/hash.h 
HEADERFILES += ./wrbb_mruby/include/mruby/irep.h 
HEADERFILES += ./wrbb_mruby/include/mruby/khash.h 
HEADERFILES += ./wrbb_mruby/include/mruby/numeric.h 
HEADERFILES += ./wrbb_mruby/include/mruby/object.h 
HEADERFILES += ./wrbb_mruby/include/mruby/opcode.h 
HEADERFILES += ./wrbb_mruby/include/mruby/proc.h 
HEADERFILES += ./wrbb_mruby/include/mruby/range.h 
HEADERFILES += ./wrbb_mruby/include/mruby/re.h 
HEADERFILES += ./wrbb_mruby/include/mruby/string.h 
HEADERFILES += ./wrbb_mruby/include/mruby/throw.h 
HEADERFILES += ./wrbb_mruby/include/mruby/value.h 
HEADERFILES += ./wrbb_mruby/include/mruby/variable.h 
HEADERFILES += ./wrbb_mruby/include/mruby/version.h
ifdef MR_MP3
HEADERFILES += ./WavMp3p/WavMp3p.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/bit.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/config.h.in 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/decoder.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/fixed.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/frame.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/global.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/huffman.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/layer12.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/layer3.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/mad.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/stream.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/synth.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/timer.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/version.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/msvc++/config.h 
HEADERFILES += ./WavMp3p/libmad-0.15.1b/msvc++/mad.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_audio.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_ctrl.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_dma.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_gpio.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_icu.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_init.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_play.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_play_mp3.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_play_wav.h 
HEADERFILES += ./WavMp3p/utility/wavmp3p_pwm.h
endif

ifeq ($(OS),Windows_NT)
	# GNU_PATH := c:/Renesas/GNURXv14.03-ELF/rx-elf/rx-elf/
	GNU_PATH := D:/dev/ide4gr-1.03/hardware/tools/gcc-rx/rx-elf/rx-elf/
else
	OSNAME=${shell uname -s}
	ifeq ($(OSNAME),Darwin)
		GNU_PATH := /Applications/IDE4GR.app/Contents/Java/hardware/tools/gcc-rx/rx-elf/rx-elf/
	else
		GNU_PATH := /usr/share/gnurx_v14.03_elf-1/
	endif
endif
GCC_VERSION := 4.8-GNURX_v14.03
CCINC := $(filter-out -I./wrbb_mruby/include/mruby, $(CCINC)) #string.hの重複を避けるため
CFLAGS := -Wall -ffunction-sections -fno-function-cse -fsigned-char -fdata-sections -I. -I"$(GNU_PATH)rx-elf\include" -I"$(GNU_PATH)lib\gcc\rx-elf\$(GCC_VERSION)\include" -mno-balign -DARDUINO=100 -DCPPAPP -D__RX_LITTLE_ENDIAN__=1 -O2 -g2 -g -flto -mlittle-endian-data -mcpu=rx600 -m64bit-doubles \
          -DMRB_USE_FLOAT -DMRB_FUNCALL_ARGC_MAX=6 -DMRB_HEAP_PAGE_SIZE=24 -DMRB_USE_IV_SEGLIST -DMRB_IVHASH_INIT_SIZE=3 -DKHASH_DEFAULT_SIZE=2 -DPOOL_PAGE_SIZE=256 -DMRB_BYTECODE_DECODE_OPTION # mruby/build_config.rbを参照のこと

ifeq ($(TARGET_BOARD),GRCITRUS)
CFLAGS += -DGRCITRUS
endif
ifeq ($(TARGET_BOARD),GRCITRUS_JTAG)
CFLAGS += -DGRCITRUS
endif
ifeq ($(TARGET_BOARD),GRSAKURA_128)
CFLAGS += -DGRSAKURA
endif
ifeq ($(TARGET_BOARD),GRSAKURA_128_JTAG)
CFLAGS += -DGRSAKURA
endif
ifeq ($(TARGET_BOARD),GRSAKURA_256)
CFLAGS += -DGRSAKURA
endif
ifdef INC_FONT
CFLAGS += -DMR_FONT
endif
ifdef INC_SPI
CFLAGS += -DMR_SPI
endif
ifdef INC_LCDSPI
CFLAGS += -DMR_LCDSPI
endif
ifdef INC_JPEG
CFLAGS += -DMR_JPEG
endif
ifdef INC_SERIALCAMERA
CFLAGS += -DMR_SERIALCAMERA
endif
ifdef INC_TWITTER
CFLAGS += -DMR_TWITTER
endif
ifdef INC_UTIL
CFLAGS += -DMR_UTIL
endif
AFLAGS := -Wall -ffunction-sections -fno-function-cse -fsigned-char -fdata-sections -I. -I"$(GNU_PATH)rx-elf\include" -I"$(GNU_PATH)lib\gcc\rx-elf\$(GCC_VERSION)\include" -mno-balign -DARDUINO=100 -DCPPAPP -D__RX_LITTLE_ENDIAN__=1 -O2 -g2 -g -flto -mlittle-endian-data -mcpu=rx600 -m64bit-doubles
AFLAGS += -DGRCITRUS
SFLAGS :=--gdwarf2
CC  = $(GNU_PATH)bin/rx-elf-gcc
CPP = $(GNU_PATH)bin/rx-elf-gcc
AS  = $(GNU_PATH)bin/rx-elf-as
LNK = $(GNU_PATH)bin/rx-elf-ld
AR  = $(GNU_PATH)bin/rx-elf-ar rcs
CNVS = $(GNU_PATH)bin/rx-elf-objcopy -O srec -I elf32-rx-be-ns
CNVB = $(GNU_PATH)bin/rx-elf-objcopy -O binary
DMP = $(GNU_PATH)bin/rx-elf-objdump
OBJS = $(OBJFILES) $(LIBFILES)
AOBJS = $(filter-out ./gr_sketch.o, $(OBJFILES))

ifeq ($(TARGET_BOARD),GRSAKURA_128)
TARGET = sakura_sketch_128K
LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_BDDFP.gsi" -no-keep-memory -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group
endif

ifeq ($(TARGET_BOARD),GRSAKURA_128_JTAG)
TARGET = sakura_sketch_128K
LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_BDDFP_noloader.gsi" -no-keep-memory -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group
endif

ifeq ($(TARGET_BOARD),GRCITRUS)
TARGET = citrus_sketch
LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_FDDFP.gsi" -no-keep-memory -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group
endif

ifeq ($(TARGET_BOARD),GRCITRUS_JTAG)
TARGET = citrus_sketch
LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_FDDFP_noloader.gsi" -no-keep-memory -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group
endif

ifeq ($(TARGET_BOARD),GRSAKURA_256)
TARGET = sakura2_sketch_256K
LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_YDDFP.gsi" -no-keep-memory -S -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group
endif

#TARGET = wrbb_sketch_128K
#LFLAGS = -Map ./gr_build/$(TARGET).map -e_PowerON_Reset -T"./gr_common/linker_arduino_BDDFP.gsi" -no-keep-memory -S -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" -L"$(GNU_PATH)rx-elf/lib/64-bit-double" -L"$(GNU_PATH)lib/gcc/rx-elf/$(GCC_VERSION)/64-bit-double" --no-flag-mismatch-warnings --start-group -lstdc++ -lnosys -lsim -lm -lc -lgcc --end-group

MAKEFILE = makefile

make = make --no-print-directory

all:	rom

rom:	$(OBJS) $(MAKEFILE)
	$(LNK) $(OBJS) $(LFLAGS) -o ./gr_build/$(TARGET).x
	$(CNVB) ./gr_build/$(TARGET).x  $(TARGET).bin
	$(CNVS) ./gr_build/$(TARGET).x  ./gr_build/$(TARGET).mot

%.o: %.s
	$(AS) $(SFLAGS) $(CCINC) $< -o $@

%.o: %.S
	$(CC) $(AFLAGS) $(CCINC) -c -x assembler-with-cpp $< -o $@

%.o: %.asm
	$(CC) $(AFLAGS) $(CCINC) -c -x assembler-with-cpp $< -o $@

%.o: %.c $(HEADERFILES)
	$(CC) $(CFLAGS) $(CCINC) -c -x c $< -o $@

%.o: %.cpp $(HEADERFILES)
	$(CPP) $(CFLAGS) $(CCINC) -c -x c++ $< -o $@

clean:
	rm -f $(OBJFILES)
	rm -f ./gr_build/$(TARGET).x
	rm -f ./gr_build/$(TARGET).mot
	rm -f ./gr_build/$(TARGET).map
#	rm -f $(TARGET).bin

bin: core
	$(CNVB) $(TARGET).x  $(TARGET).bin

core: lib
	$(LNK) gr_sketch.o ./gr_common/rx63n/vector_table.o ./gr_common/rx63n/interrupt_handlers.o core.a $(LFLAGS) -o $(TARGET).x
	$(CNVS) $(TARGET).x  $(TARGET).mot

lib: $(AOBJS) $(MAKEFILE)
	$(AR) core.a $(AOBJS)

clrsrc:
	rm -f $(filter-out ./gr_sketch.cpp, $(SRCFILES))
