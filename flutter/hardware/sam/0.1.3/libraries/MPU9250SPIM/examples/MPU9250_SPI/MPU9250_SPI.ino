//==============================================================================================//
//==============================================================================================//
//					                                        MPU9250 via SPI //
//==============================================================================================//
//==============================================================================================//
#define VERSION      1.0 Beta // Version number


//==============================================================================================//
//				                                                       Includes	//
//==============================================================================================//
#include <SPI.h>
#include <Streaming.h>
#include <mpu9250_spi.h>

//==============================================================================================//
//										Pin Definitions	//
//==============================================================================================//
const int nCS_GPS = 0;
const int Force_ADC = 1;
const int nCS_MPU = 2; // moved to GPIO 2, MOSI needs to be on GPIO 5
const int nCS_FLASH = 6;

//==============================================================================================//
//==============================================================================================//
//										Global Scope	//
//==============================================================================================//
//==============================================================================================//

//==============================================================================================//
//									 Object Initializations //
//==============================================================================================//
mpu9250_spi imu(nCS_MPU);



//==============================================================================================//
//									  IO Stream Definitions //
//==============================================================================================//
#define Debug Serial


//==============================================================================================//
//==============================================================================================//
//										Program Startup	//
//==============================================================================================//
//==============================================================================================//
void setup()
{
  Debug.begin(57600); // set at 57.6k for now for Debug com monitor, will change back later

  delay(100);

  Debug <<  F("Booting .....") << endl;

  SPI.begin(); // default SPI is 4MHz
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  pinMode(nCS_MPU, OUTPUT);
  digitalWrite(nCS_MPU, HIGH);

  if(imu.init(1,BITS_DLPF_CFG_188HZ))
  {  
    Debug <<  F("Couldn't initialize MPU9250 via SPI!") << endl;
  }
  Debug <<  F("WHOAMI = ") << imu.whoami() << endl;  //output the I2C address to know if SPI is working, it should be 104
  delay(1);    
  Debug <<  F("Gyro_scale = ") << imu.set_gyro_scale(BITS_FS_2000DPS) << endl;	//Set full scale range for gyros
  delay(1);  
  Debug <<  F("Acc_scale = ") << imu.set_acc_scale(BITS_FS_16G) << endl;			//Set full scale range for accs
  delay(1);
  Debug <<  F("AK8963 WHIAM = ") << imu.AK8963_whoami() << endl;
  delay(1);  
  imu.AK8963_calib_Magnetometer();
}

//==============================================================================================//
//==============================================================================================//
//										   Program Loop //
//==============================================================================================//
//==============================================================================================//
void loop()
{
  imu.read_all();
  Debug << F("Gyro : ");
  Debug << imu.gyroscope_data[0] << F(",");
  Debug << imu.gyroscope_data[1] << F(",");
  Debug << imu.gyroscope_data[2] << F(",");
  Debug << F(" Acc : ");
  Debug << imu.accelerometer_data[0] << F(",");
  Debug << imu.accelerometer_data[1] << F(",");
  Debug << imu.accelerometer_data[2] << F(",");
  Debug << F(" Mag : ");
  Debug << imu.Magnetometer[0] << F(",");
  Debug << imu.Magnetometer[1] << F(",");
  Debug << imu.Magnetometer[2] << endl;
  delay(30);
}


