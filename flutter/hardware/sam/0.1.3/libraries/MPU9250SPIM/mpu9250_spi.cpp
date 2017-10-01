//==============================================================================================//
//==============================================================================================//
//								MPU9250 Commands and Definitions //
//==============================================================================================//
//==============================================================================================//
#include <Arduino.h>
#include "mpu9250_spi.h"
#include "DigitalIO.h"

//==============================================================================================//
//									     MPU9250 Read-Write	//
//==============================================================================================// 
#include "DigitalIO.h"
const uint8_t SOFT_SPI_MISO_PIN = 8;
const uint8_t SOFT_SPI_MOSI_PIN = 9;
const uint8_t SOFT_SPI_SCK_PIN  = 10;
const uint8_t SPI_MODE = 0;
extern SoftSPI<SOFT_SPI_MISO_PIN, SOFT_SPI_MOSI_PIN, SOFT_SPI_SCK_PIN, SPI_MODE> spi;


mpu9250_spi::mpu9250_spi(int cs)
{
  _CS_pin = cs;
  pinMode(_CS_pin,OUTPUT);
  spi.begin();
}
 
unsigned int mpu9250_spi::WriteReg( uint8_t WriteAddr, uint8_t WriteData )
{
  unsigned int temp_val;
  select();
  spi.transfer(WriteAddr);
  temp_val=spi.transfer(WriteData);
  deselect();
  delayMicroseconds(50);
  return temp_val;
}

unsigned int  mpu9250_spi::ReadReg( uint8_t WriteAddr, uint8_t WriteData )
{
  return WriteReg(WriteAddr | READ_FLAG,WriteData);
}

void mpu9250_spi::ReadRegs( uint8_t ReadAddr, uint8_t *ReadBuf, unsigned int Bytes )
{
  unsigned int  i = 0;
 
  select();
  spi.transfer(ReadAddr | READ_FLAG);
  for(i=0; i<Bytes; i++) ReadBuf[i] = spi.transfer(0x00);
  deselect();
  delayMicroseconds(50);
}
 
//==============================================================================================//
//							               MPU-9250 Initialization	//
//==============================================================================================// 
/*-----------------------------------------------------------------------------------------------
                                         INITIALIZATION
usage: call this function at startup, giving the sample rate divider (raging from 0 to 255) and
low pass filter value; suitable values are:
BITS_DLPF_CFG_256HZ_NOLPF2
BITS_DLPF_CFG_188HZ
BITS_DLPF_CFG_98HZ
BITS_DLPF_CFG_42HZ
BITS_DLPF_CFG_20HZ
BITS_DLPF_CFG_10HZ 
BITS_DLPF_CFG_5HZ 
BITS_DLPF_CFG_2100HZ_NOLPF
returns 1 if an error occurred
-----------------------------------------------------------------------------------------------*/
#define MPU_InitRegNum 16
 
bool mpu9250_spi::init(int sample_rate_div,int low_pass_filter)
{
  uint8_t i = 0;
	// Setup Acc
	//----------------------------------------------------------------------------
	WriteReg(MPUREG_PWR_MGMT_1,	0x80);	// toggle reset device
	delay(1000);
	// reset FIFO :
	WriteReg(MPUREG_INT_ENABLE,	0x00);
	WriteReg(MPUREG_FIFO_EN,		0x00);
	WriteReg(MPUREG_USER_CTRL,		0x04);
	delay(100);
	
	WriteReg(MPUREG_USER_CTRL,		0x60);
	WriteReg(MPUREG_FIFO_EN,		0b01111010);
	WriteReg(MPUREG_INT_ENABLE,		0x01);

	// else use the internal oscillator, bits 2:0 = 001
	WriteReg(MPUREG_PWR_MGMT_2,	0x00);
	WriteReg(MPUREG_PWR_MGMT_1,	0x01);	// Turn on internal clock source
	// Set SampleRate
	uint8_t div=(1000/100)-1;
	WriteReg(MPUREG_SMPRT_DIV, div);
	//WriteReg(MPUREG_GYRO_LPF, MPUREG_GYRO_LPF_184);
	//WriteReg(MPUREG_ACCEL_LPF,	MPUREG_ACCEL_LPF_184);
	WriteReg(MPUREG_GYRO_CONFIG, BITS_FS_1000DPS);
	WriteReg(MPUREG_ACCEL_CONFIG, BITS_FS_2G);

	// Confiugure magneto :
	// Reset magnometer
	WriteReg(MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR );
	WriteReg(MPUREG_I2C_SLV0_REG, AK8963_CNTL2);
	WriteReg(MPUREG_I2C_SLV0_DO,	0x01);
	WriteReg(MPUREG_I2C_SLV0_CTRL,	0x81);
	delay(1500);

	// Set Mode and Sample Rate :
	WriteReg(MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR );
	WriteReg(MPUREG_I2C_SLV0_REG, AK8963_CNTL1);
	WriteReg(MPUREG_I2C_SLV0_DO,	0x12);
	WriteReg(MPUREG_I2C_SLV0_CTRL,	0x81);	// Enable i2c and write 1 bytes

	WriteReg(MPUREG_I2C_SLV1_ADDR, AK8963_I2C_ADDR  | READ_FLAG);
	// Magneto address
	WriteReg(MPUREG_I2C_SLV1_REG, AK8963_ST1);
	// Address of the first data register
	WriteReg(MPUREG_I2C_SLV1_CTRL,	0x88);	// Enable reading and read 8 bytes
	WriteReg(MPUREG_USER_CTRL,		0x0C);	// Reset FIFO and DMP

	// Configure FIFO to capture accelerometer and gyro data for bias calculation
	WriteReg(MPUREG_INT_ENABLE,	0x01);
	WriteReg(MPUREG_USER_CTRL,	0b01100000);  // Enable FIFO
	WriteReg(MPUREG_FIFO_EN,	0b01111010);
	// Enable gyro and accelerometer sensors for FIFO (max size 512 bytes in MPU-9150)

	set_acc_scale(2);
	set_gyro_scale(250);
    
	//AK8963_calib_Magnetometer();  //Can't load this function here , strange problem?
	return 0;
}

//==============================================================================================//
//								            Accelerometer Scale	//
//==============================================================================================// 
/*-----------------------------------------------------------------------------------------------
                                        ACCELEROMETER SCALE
usage: call this function at startup, after initialization, to set the right range for the
accelerometers. Suitable ranges are:
BITS_FS_2G
BITS_FS_4G
BITS_FS_8G
BITS_FS_16G
returns the range set (2, 4, 8, or 16)
-----------------------------------------------------------------------------------------------*/
unsigned int mpu9250_spi::set_acc_scale(int scale){
  unsigned int temp_scale;
  WriteReg(MPUREG_ACCEL_CONFIG, scale);
    
  switch (scale)
  {
    case BITS_FS_2G:
      acc_divider=16384;
    break;
    case BITS_FS_4G:
      acc_divider=8192;
    break;
    case BITS_FS_8G:
      acc_divider=4096;
    break;
    case BITS_FS_16G:
      acc_divider=2048;
    break;   
  }
 
  temp_scale=WriteReg(MPUREG_ACCEL_CONFIG|READ_FLAG, 0x00);
    
  switch (temp_scale)
  {
    case BITS_FS_2G:
      temp_scale=2;
    break;
    case BITS_FS_4G:
      temp_scale=4;
    break;
    case BITS_FS_8G:
      temp_scale=8;
    break;
    case BITS_FS_16G:
      temp_scale=16;
    break;   
  }
  return temp_scale;
}
 
//==============================================================================================//
//									        Gyroscope Scale	//
//==============================================================================================// 
/*-----------------------------------------------------------------------------------------------
                                        GYROSCOPE SCALE
usage: call this function at startup, after initialization, to set the right range for the
gyroscopes. Suitable ranges are:
BITS_FS_250DPS
BITS_FS_500DPS
BITS_FS_1000DPS
BITS_FS_2000DPS
returns the range set (250, 500, 1000 or 2000)
-----------------------------------------------------------------------------------------------*/
unsigned int mpu9250_spi::set_gyro_scale(int scale)
{
  unsigned int temp_scale;
  WriteReg(MPUREG_GYRO_CONFIG, scale);
  switch (scale)
  {
    case BITS_FS_250DPS:
      gyro_divider=131;
    break;
    case BITS_FS_500DPS:
      gyro_divider=65.5;
    break;
    case BITS_FS_1000DPS:
      gyro_divider=32.8;
    break;
    case BITS_FS_2000DPS:
      gyro_divider=16.4;
    break;   
  }
  
  temp_scale=WriteReg(MPUREG_GYRO_CONFIG|READ_FLAG, 0x00);
  
  switch (temp_scale)
  {
    case BITS_FS_250DPS:
      temp_scale=250;
    break;
    case BITS_FS_500DPS:
      temp_scale=500;
    break;
    case BITS_FS_1000DPS:
      temp_scale=1000;
    break;
    case BITS_FS_2000DPS:
      temp_scale=2000;
    break;   
  }
  return temp_scale;
}
 
//==============================================================================================//
//									              Who Am I?	//
//==============================================================================================//  
/*-----------------------------------------------------------------------------------------------
                                            WHO AM I?
usage: call this function to know if SPI is working correctly. It checks the I2C address of the
mpu9250 which should be 104 when in SPI mode.
returns the I2C address (104)
-----------------------------------------------------------------------------------------------*/
unsigned int mpu9250_spi::whoami()
{
  unsigned int response;
  response=WriteReg(MPUREG_WHOAMI|READ_FLAG, 0x00);
  return response;
}
 
//==============================================================================================//
//									        Read IMU Values	//
//==============================================================================================//  
/*-----------------------------------------------------------------------------------------------
                                       READ ACCELEROMETER
usage: call this function to read accelerometer data. Axis represents selected axis:
0 -> X axis
1 -> Y axis
2 -> Z axis
-----------------------------------------------------------------------------------------------*/
void mpu9250_spi::read_acc()
{
  uint8_t response[6];
  int16_t bit_data;
  float data;
  int i;
  ReadRegs(MPUREG_ACCEL_XOUT_H,response,6);
  for(i=0; i<3; i++) 
  {
    bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
    data=(float)bit_data;
    accelerometer_data[i]=data/acc_divider;
  }
}
 
/*-----------------------------------------------------------------------------------------------
                                         READ GYROSCOPE
usage: call this function to read gyroscope data. Axis represents selected axis:
0 -> X axis
1 -> Y axis
2 -> Z axis
-----------------------------------------------------------------------------------------------*/
void mpu9250_spi::read_rot()
{
  uint8_t response[6];
  int16_t bit_data;
  float data;
  int i;
  ReadRegs(MPUREG_GYRO_XOUT_H,response,6);
  for(i=0; i<3; i++) 
  {
    bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
    data=(float)bit_data;
    gyroscope_data[i]=data/gyro_divider;
  }
}
 
/*-----------------------------------------------------------------------------------------------
                                         READ TEMPERATURE
usage: call this function to read temperature data. 
returns the value in °C
-----------------------------------------------------------------------------------------------*/
void mpu9250_spi::read_temp()
{
  uint8_t response[2];
  int16_t bit_data;
  float data;
  ReadRegs(MPUREG_TEMP_OUT_H,response,2);
 
  bit_data=((int16_t)response[0]<<8)|response[1];
  data=(float)bit_data;
  Temperature=(data/340)+36.53;
  deselect();
}

//==============================================================================================//
//							                       Calibration Code	//
//==============================================================================================// 
/*-----------------------------------------------------------------------------------------------
                                READ ACCELEROMETER CALIBRATION
usage: call this function to read accelerometer data. Axis represents selected axis:
0 -> X axis
1 -> Y axis
2 -> Z axis
returns Factory Trim value
-----------------------------------------------------------------------------------------------*/
void mpu9250_spi::calib_acc()
{
  uint8_t response[4];
  int temp_scale;
  //READ CURRENT ACC SCALE
  temp_scale=WriteReg(MPUREG_ACCEL_CONFIG|READ_FLAG, 0x00);
  set_acc_scale(BITS_FS_8G);
  //ENABLE SELF TEST need modify
  //temp_scale=WriteReg(MPUREG_ACCEL_CONFIG, 0x80>>axis);
 
  ReadRegs(MPUREG_SELF_TEST_X,response,4);
  calib_data[0]=((response[0]&11100000)>>3)|((response[3]&00110000)>>4);
  calib_data[1]=((response[1]&11100000)>>3)|((response[3]&00001100)>>2);
  calib_data[2]=((response[2]&11100000)>>3)|((response[3]&00000011));
 
  set_acc_scale(temp_scale);
}

uint8_t mpu9250_spi::AK8963_whoami()
{
  uint8_t response;
  WriteReg(MPUREG_I2C_SLV0_ADDR, AK8963_I2C_ADDR|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
  delay(100);
  WriteReg(MPUREG_I2C_SLV0_REG, AK8963_WIA); //I2C slave 0 register address from where to begin data transfer
  delay(100);
  WriteReg(MPUREG_I2C_SLV0_CTRL, 0x81); //Read 1 byte from the magnetometer
 
  //WriteReg(MPUREG_I2C_SLV0_CTRL, 0x81);    //Enable I2C and set bytes
  delay(100);
  response=WriteReg(MPUREG_EXT_SENS_DATA_00|READ_FLAG, 0x00);    //Read I2C 
  //ReadRegs(MPUREG_EXT_SENS_DATA_00,response,1);
  //response=WriteReg(MPUREG_I2C_SLV0_DO, 0x00);    //Read I2C 
  delay(100);

  return response;
}

void mpu9250_spi::AK8963_calib_Magnetometer()
{
  uint8_t response[3];
  float data;
  int i;
 
  WriteReg(MPUREG_I2C_SLV0_ADDR,AK8963_I2C_ADDR|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
  WriteReg(MPUREG_I2C_SLV0_REG, AK8963_ASAX); //I2C slave 0 register address from where to begin data transfer
  WriteReg(MPUREG_I2C_SLV0_CTRL, 0x83); //Read 3 bytes from the magnetometer
 
  //WriteReg(MPUREG_I2C_SLV0_CTRL, 0x81);    //Enable I2C and set bytes
  delayMicroseconds(100);
  //response[0]=WriteReg(MPUREG_EXT_SENS_DATA_01|READ_FLAG, 0x00);    //Read I2C 
  ReadRegs(MPUREG_EXT_SENS_DATA_00,response,3);
    
  //response=WriteReg(MPUREG_I2C_SLV0_DO, 0x00);    //Read I2C 
  for(i=0; i<3; i++) 
  {
    data=response[i];
    Magnetometer_ASA[i]=((data-128)/256+1)*Magnetometer_Sensitivity_Scale_Factor;
  }
}

void mpu9250_spi::AK8963_read_Magnetometer()
{
   uint8_t response[7];
   int16_t bit_data;
   float data;
   int i;
 
   WriteReg(MPUREG_I2C_SLV0_ADDR,AK8963_I2C_ADDR|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
   WriteReg(MPUREG_I2C_SLV0_REG, AK8963_HXL); //I2C slave 0 register address from where to begin data transfer
   WriteReg(MPUREG_I2C_SLV0_CTRL, 0x87); //Read 6 bytes from the magnetometer
 
   delayMicroseconds(100);
   ReadRegs(MPUREG_EXT_SENS_DATA_00,response,7);
   //must start your read from AK8963A register 0x03 and read seven bytes so that upon read of ST2 register 0x09 the AK8963A will unlatch the data registers for the next measurement.
   for(i=0; i<3; i++) 
   {
      bit_data=((int16_t)response[i*2+1]<<8)|response[i*2];
      data=(float)bit_data;
      Magnetometer[i]=data*Magnetometer_ASA[i];
   }
}

void mpu9250_spi::read_all()
{
   uint8_t response[21];
   int16_t bit_data;
   float data;
   int i;
 
   //Send I2C command at first
   WriteReg(MPUREG_I2C_SLV0_ADDR,AK8963_I2C_ADDR|READ_FLAG); //Set the I2C slave addres of AK8963 and set for read.
   WriteReg(MPUREG_I2C_SLV0_REG, AK8963_HXL); //I2C slave 0 register address from where to begin data transfer
   WriteReg(MPUREG_I2C_SLV0_CTRL, 0x87); //Read 7 bytes from the magnetometer
   //must start your read from AK8963A register 0x03 and read seven bytes so that upon read of ST2 register 0x09 the AK8963A will unlatch the data registers for the next measurement.
 
   //wait(0.001);
   ReadRegs(MPUREG_ACCEL_XOUT_H,response,21);
   //Get accelerometer value
   for(i=0; i<3; i++) 
   {
      bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
      data=(float)bit_data;
      accelerometer_data[i]=data/acc_divider;
   }
   //Get temperature
      bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
      data=(float)bit_data;
      Temperature=((data-21)/333.87)+21;
    //Get gyroscope value
   for(i=4; i<7; i++) 
   {
      bit_data=((int16_t)response[i*2]<<8)|response[i*2+1];
      data=(float)bit_data;
      gyroscope_data[i-4]=data/gyro_divider;
   }
   //Get Magnetometer value
   for(i=7; i<10; i++) 
   {
      bit_data=((int16_t)response[i*2+1]<<8)|response[i*2];
      data=(float)bit_data;
      Magnetometer[i-7]=data*Magnetometer_ASA[i-7];
   }
}
 
/*-----------------------------------------------------------------------------------------------
                                SPI SELECT AND DESELECT
usage: enable and disable mpu9250 communication bus
-----------------------------------------------------------------------------------------------*/
void mpu9250_spi::select() 
{
   digitalWrite(_CS_pin,LOW);
}
void mpu9250_spi::deselect() 
{
   digitalWrite(_CS_pin,HIGH);
}
