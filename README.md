# ArduinoBoardPackage
This is the current active version that I am modifing.  Copy and paste to your Arduino15/packages directory

Change1 - Updated SPI library to use transactions and the SPISettings.

Change2 - Since hardware SPI does not seem to work added software SPI support using Bill Greiman's DigitalIO library.  SD card support was added by adding the Flutter board to the Bill Greiman's SDFat library. Both libraries have been added to the package libraries section.  I have also include a SPI library for the MPU9250 IMU.

Change3 - added an excel spreadsheet of the actual pin mapping of the flutter board.  These are the pin numbers you use when referencing from sketches.

Change4 - in my fork of the FlutterWireless library i added two examples. The first for transmitting BME280 sensor readings and the second an example to log data from a MPU9250 using spi for both the sd card and a BME280 on the i2c bus.

Change5 - using the TinyPacks data serialization library to format data for transmitting and receiving which i also included in the libraries folder so you don't have to search for it.
