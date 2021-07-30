avr-gcc -mmcu=attiny85 attiny85_test.c;
avr-objcopy -O ihex  a.out a.hex;
avrdude -C /etc/avrdude.conf -p t85 -c stk500v1 -P /dev/ttyUSB0 -b 19200 -U flash:w:a.hex:i
