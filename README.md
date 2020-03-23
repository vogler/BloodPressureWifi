# BloodPressureWifi

Plan: Add a Wemos D1 mini to my Beurer blood pressure monitor to publish results via MQTT.

- Photos: https://photos.app.goo.gl/GeWPxujQbdsRS84m7
- Helpful tutorial: https://www.edusteinhorst.com/hacking-a-blood-pressure-monitor/
- Nothing usable on GitHub: https://github.com/topics/blood-pressure
- Needed hardware:
  - USB Logic Analyzer 24M 8CH ([ordered](https://trade.aliexpress.com/order_detail.htm?spm=a2g0s.9042311.0.0.27424c4d8So3IX&orderId=100481955072588) on 31.03.2019 for 5.25$): [AliExpress](https://www.aliexpress.com/item/32953889214.html?spm=a2g0s.9042311.0.0.27424c4d1U9tuk) https://sigrok.org/wiki/VKTECH_saleae_clone
  - Wemos D1 mini
- Logic Analyzer Software:
  - https://sigrok.org/wiki/PulseView
    - Install [WinUSB driver with Zadig](https://sigrok.org/wiki/Fx2lafw) and then select driver [fx2lafw](https://sigrok.org/wiki/Fx2lafw) in PulseView ([screenshot](PulseView-driver.png)).
  - https://www.saleae.com
  - https://www.ikalogic.com/scanastudio/
- Analyze HEX: https://hexed.it
- Application notes on oscillation signal: http://ww1.microchip.com/downloads/en/AppNotes/00001556B.pdf http://www.8051projects.net/files/public/1235208942_12665_FT14949_an1571.pdf
- Chips in my Beurer blood pressure monitor:
  - CS-20A pressure sensor: https://brmlab.cz/user/jenda/cs-20a
  - CMOS Serial EEPROM: [S-93C66BD](S-93C66BD.pdf): http://www.farnell.com/datasheets/46810.pdf (pins: page 2, lower half)
    - 256-word × 16-bit
    - Soldered cables to pins and connected to logic analyzer: CS:yellow:D0, SK:orange:D1, DI:red:D2, DO:brown:D3, GND:black
    - [SPI decoder settings](PulseView-SPI-channels.png), [first three blocks](PulseView-SPI-data.png)
    - Can also use the [Microwire decoder with a stacked protocol decoder for 93xx EEPROM](PulseView-Microwirte-channels.png) with 8 bit address size and 16 bit word size, but for writes it complains with 'Not enough word bits'
    - Data is only written to EEPROM once the device is turned off after measuring.
- Measured a couple of times to figure out what the data means. A memory slot is 4x 16 bit words of data:
~~~
Block 1: DI: 100 11xxxxxx = Write enable
Block 2: DI: 110... = Read address 0 -> DO: 0x101 = 5 (start of next free memory slot)
Block 3: Write address 5, bin: 0000100000000100, hex: 08 04, ints: 8 (month) 4 (?, constant)
Block 4: Write address 6, bin: 0000101000010101, hex: 0A 15, ints: 10 (hour) 21 (day)
Block 5: Write address 7, bin: 0111010100100000, hex: 75 20, ints: 117 (high BP - 20) 32 (minutes)
Block 6: Write address 8, bin: 0011110001010111, hex: 3C 57, ints: 60 (HR) 87 (low BP)
Block 7: Write address 0, bin: 0000001000000010, hex: 02 02, ints: 2 2 (number of occupied memory slots)
Block 8: DI: 100 00xxxxxx = Write disable
~~~
