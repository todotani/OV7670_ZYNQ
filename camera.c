/*
 * InitCamera.c
 *
 *  Created on: 2016/09/22
 *      Author: Kenshi Kamiya
 */
#include "xparameters.h"
#include "sleep.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "camera.h"

#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID

XIicPs Iic;

int InitCamera()
{
	int Status, result;
	XIicPs_Config *I2C_Config;	/**< configuration information for the device */

	I2C_Config = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if(I2C_Config == NULL){
		xil_printf("Error: XIicPs_LookupConfig()\n");
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, I2C_Config, I2C_Config->BaseAddress);
	if(Status != XST_SUCCESS){
		xil_printf("Error: XIicPs_CfgInitialize()\n");
		return XST_FAILURE;
	}

	Status = XIicPs_SelfTest(&Iic);
	if(Status != XST_SUCCESS){
		xil_printf("Error: XIicPs_SelfTest()\n");
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	xil_printf("I2C configuration done.\n");

	xil_printf("Soft Rest OV7670.\n");
	result = WriteReg(REG_COM7, COM7_RESET);
	if(result != XST_SUCCESS){
		xil_printf("Error: OV767 RESET\n");
		return XST_FAILURE;
	}
	usleep(300*1000);

	return XST_SUCCESS;
}


int WriteReg(u8 reg, u8 value)
{
	int Status;
	u8 buff[2];

	buff[0] = reg;
	buff[1] = value;

	Status = XIicPs_MasterSendPolled(&Iic, buff, 2, OV7670ADDR);

	if(Status != XST_SUCCESS){
		xil_printf("WriteReg:I2C Write Fail\n");
		return XST_FAILURE;
	}
	// Wait until bus is idle to start another transfer.
	while(XIicPs_BusIsBusy(&Iic)){
		/* NOP */
	}

	usleep(30*1000);	// wait 30ms

	return XST_SUCCESS;
}


int ReadReg(u8 reg)
{
	u8 buff[2];

	buff[0] = reg;
	XIicPs_MasterSendPolled(&Iic, buff, 1, OV7670ADDR);
	while(XIicPs_BusIsBusy(&Iic)){
		/* NOP */
	}

	XIicPs_MasterRecvPolled(&Iic, buff, 1, OV7670ADDR);
	while(XIicPs_BusIsBusy(&Iic)){
		/* NOP */
	}

	return buff[0];
}


void DumpReg(void)
{
	int i;

    xil_printf("AD : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F") ;
    for (i=0; i < OV7670_REGMAX; i++) {
        int data ;
        data = ReadReg(i) ; // READ REG
        if ((i & 0x0F) == 0) {
            xil_printf("\n%02X : ", i) ;
        }
        xil_printf("%02X ",data) ;
    }
    xil_printf("\n") ;

}


void InitDefaultReg(void) {
    // Gamma curve values
    WriteReg(0x7a, 0x20);
    WriteReg(0x7b, 0x10);
    WriteReg(0x7c, 0x1e);
    WriteReg(0x7d, 0x35);
    WriteReg(0x7e, 0x5a);
    WriteReg(0x7f, 0x69);
    WriteReg(0x80, 0x76);
    WriteReg(0x81, 0x80);
    WriteReg(0x82, 0x88);
    WriteReg(0x83, 0x8f);
    WriteReg(0x84, 0x96);
    WriteReg(0x85, 0xa3);
    WriteReg(0x86, 0xaf);
    WriteReg(0x87, 0xc4);
    WriteReg(0x88, 0xd7);
    WriteReg(0x89, 0xe8);

    // AGC and AEC parameters.  Note we start by disabling those features,
    //then turn them only after tweaking the values.
    WriteReg(REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT);
    WriteReg(REG_GAIN, 0);
    WriteReg(REG_AECH, 0);
    WriteReg(REG_COM4, 0x40);
    // magic reserved bit
    WriteReg(REG_COM9, 0x18);	// ACG Ceiling 4x
    // 4x gain + magic rsvd bit
    WriteReg(REG_BD50MAX, 0x05);
    WriteReg(REG_BD60MAX, 0x07);
    WriteReg(REG_AEW, 0x95);
    WriteReg(REG_AEB, 0x33);
    WriteReg(REG_VPT, 0xe3);
    WriteReg(REG_HAECC1, 0x78);
    WriteReg(REG_HAECC2, 0x68);
    WriteReg(0xa1, 0x03);
    // magic
    WriteReg(REG_HAECC3, 0xd8);
    WriteReg(REG_HAECC4, 0xd8);
    WriteReg(REG_HAECC5, 0xf0);
    WriteReg(REG_HAECC6, 0x90);
    WriteReg(REG_HAECC7, 0x94);
    WriteReg(REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC);

    // Almost all of these are magic "reserved" values.
    WriteReg(REG_COM5, 0x61);
    WriteReg(REG_COM6, 0x4b);
    WriteReg(0x16, 0x02);
    WriteReg(REG_MVFP, 0x37);		// VFlip on, Mirror Image
    WriteReg(0x21, 0x02);
    WriteReg(0x22, 0x91);
    WriteReg(0x29, 0x07);
    WriteReg(0x33, 0x0b);
    WriteReg(0x35, 0x0b);
    WriteReg(0x37, 0x1d);
    WriteReg(0x38, 0x71);
    WriteReg(0x39, 0x2a);
    WriteReg(REG_COM12, 0x78);
    WriteReg(0x4d, 0x40);
    WriteReg(0x4e, 0x20);
    WriteReg(REG_GFIX, 0);
    WriteReg(0x6b, 0x0a);
    WriteReg(0x74, 0x10);
    WriteReg(0x8d, 0x4f);
    WriteReg(0x8e, 0);
    WriteReg(0x8f, 0);
    WriteReg(0x90, 0);
    WriteReg(0x91, 0);
    WriteReg(0x96, 0);
    WriteReg(0x9a, 0);
    WriteReg(0xb0, 0x84);
    WriteReg(0xb1, 0x0c);
    WriteReg(0xb2, 0x0e);
    WriteReg(0xb3, 0x82);
    WriteReg(0xb8, 0x0a);

    // More reserved magic, some of which tweaks white balance
    WriteReg(0x43, 0x0a);
    WriteReg(0x44, 0xf0);
    WriteReg(0x45, 0x34);
    WriteReg(0x46, 0x58);
    WriteReg(0x47, 0x28);
    WriteReg(0x48, 0x3a);
    WriteReg(0x59, 0x88);
    WriteReg(0x5a, 0x88);
    WriteReg(0x5b, 0x44);
    WriteReg(0x5c, 0x67);
    WriteReg(0x5d, 0x49);
    WriteReg(0x5e, 0x0e);
    WriteReg(0x6c, 0x0a);
    WriteReg(0x6d, 0x55);
    WriteReg(0x6e, 0x11);
    WriteReg(0x6f, 0x9f);
    // "9e for advance AWB"
    WriteReg(0x6a, 0x40);
    WriteReg(REG_BLUE, 0x40);
    WriteReg(REG_RED, 0x60);
    WriteReg(REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC|COM8_AWB);

    // Matrix coefficients
    WriteReg(0x4f, 0x80);
    WriteReg(0x50, 0x80);
    WriteReg(0x51, 0);
    WriteReg(0x52, 0x22);
    WriteReg(0x53, 0x5e);
    WriteReg(0x54, 0x80);
    WriteReg(0x58, 0x9e);

    WriteReg(REG_COM16, COM16_AWBGAIN);
    WriteReg(REG_EDGE, 0);
    WriteReg(0x75, 0x05);
    WriteReg(0x76, 0xe1);
    WriteReg(0x4c, 0);
    WriteReg(0x77, 0x01);
    WriteReg(0x4b, 0x09);
    WriteReg(0xc9, 0x60);
    WriteReg(REG_COM16, 0x38);
    WriteReg(0x56, 0x40);

    WriteReg(0x34, 0x11);
    WriteReg(REG_COM11, COM11_EXP|COM11_HZAUTO_ON);
    WriteReg(0xa4, 0x88);
    WriteReg(0x96, 0);
    WriteReg(0x97, 0x30);
    WriteReg(0x98, 0x20);
    WriteReg(0x99, 0x30);
    WriteReg(0x9a, 0x84);
    WriteReg(0x9b, 0x29);
    WriteReg(0x9c, 0x03);
    WriteReg(0x9d, 0x4c);
    WriteReg(0x9e, 0x3f);
    WriteReg(0x78, 0x04);

    // Extra-weird stuff.  Some sort of multiplexor register
    WriteReg(0x79, 0x01);
    WriteReg(0xc8, 0xf0);
    WriteReg(0x79, 0x0f);
    WriteReg(0xc8, 0x00);
    WriteReg(0x79, 0x10);
    WriteReg(0xc8, 0x7e);
    WriteReg(0x79, 0x0a);
    WriteReg(0xc8, 0x80);
    WriteReg(0x79, 0x0b);
    WriteReg(0xc8, 0x01);
    WriteReg(0x79, 0x0c);
    WriteReg(0xc8, 0x0f);
    WriteReg(0x79, 0x0d);
    WriteReg(0xc8, 0x20);
    WriteReg(0x79, 0x09);
    WriteReg(0xc8, 0x80);
    WriteReg(0x79, 0x02);
    WriteReg(0xc8, 0xc0);
    WriteReg(0x79, 0x03);
    WriteReg(0xc8, 0x40);
    WriteReg(0x79, 0x05);
    WriteReg(0xc8, 0x30);
    WriteReg(0x79, 0x26);
}


void InitRGB565(void){
    int reg_com7 = ReadReg(REG_COM7);

    WriteReg(REG_COM7, reg_com7|COM7_RGB);
    WriteReg(REG_RGB444, RGB444_DISABLE);
    WriteReg(REG_COM15, COM15_R00FF|COM15_RGB565);
    WriteReg(REG_TSLB, 0x04);	   // TSLB Set UV ordering,  do not auto-reset window
    WriteReg(REG_COM1, 0x00);	   // CCIR 656 disable
    WriteReg(REG_COM9, 0x38);      // 16x gain ceiling; 0x8 is reserved bit
    WriteReg(0x4f, 0xb3);          // "matrix coefficient 1"
    WriteReg(0x50, 0xb3);          // "matrix coefficient 2"
    WriteReg(0x51, 0x00);          // vb
    WriteReg(0x52, 0x3d);          // "matrix coefficient 4"
    WriteReg(0x53, 0xa7);          // "matrix coefficient 5"
    WriteReg(0x54, 0xe4);          // "matrix coefficient 6"
    WriteReg(REG_COM13, COM13_GAMMA|COM13_UVSAT);
}


void InitVGA(void) {
    // VGA
    int reg_com7 = ReadReg(REG_COM7);
    WriteReg(REG_COM7,reg_com7|COM7_VGA);
    WriteReg(REG_HSTART,HSTART_VGA);
    WriteReg(REG_HSTOP,HSTOP_VGA);
    WriteReg(REG_HREF,HREF_VGA);
    WriteReg(REG_VSTART,VSTART_VGA);
    WriteReg(REG_VSTOP,VSTOP_VGA);
    WriteReg(REG_VREF,VREF_VGA);
    WriteReg(REG_COM3, COM3_VGA);
    WriteReg(REG_COM14, COM14_VGA);
    WriteReg(REG_SCALING_XSC, SCALING_XSC_VGA);
    WriteReg(REG_SCALING_YSC, SCALING_YSC_VGA);
    WriteReg(REG_SCALING_DCWCTR, SCALING_DCWCTR_VGA);
    WriteReg(REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_VGA);
    WriteReg(REG_SCALING_PCLK_DELAY, SCALING_PCLK_DELAY_VGA);
}


void InitQVGA(void) {
    // QVGA mode
    int reg_com7 = ReadReg(REG_COM7);
    WriteReg(REG_COM7,reg_com7|COM7_QVGA);
    WriteReg(REG_HSTART,HSTART_QVGA);
    WriteReg(REG_HSTOP,HSTOP_QVGA);
    WriteReg(REG_HREF,HREF_QVGA);
    WriteReg(REG_VSTART,VSTART_QVGA);
    WriteReg(REG_VSTOP,VSTOP_QVGA);
    WriteReg(REG_VREF,VREF_QVGA);
    WriteReg(REG_COM3, COM3_QVGA);
    WriteReg(REG_COM14, COM14_QVGA);		// Divided by 2
    WriteReg(REG_SCALING_XSC, SCALING_XSC_QVGA);
    WriteReg(REG_SCALING_YSC, SCALING_YSC_QVGA);
    WriteReg(REG_SCALING_DCWCTR, SCALING_DCWCTR_QVGA);
    WriteReg(REG_SCALING_PCLK_DIV, SCALING_PCLK_DIV_QVGA);	// Divided by 2
    WriteReg(REG_SCALING_PCLK_DELAY, SCALING_PCLK_DELAY_QVGA);
}


void ShowQVGA_ColorBar(void) {
	// QVGA
	WriteReg(REG_SCALING_XSC, 0x80);
	WriteReg(REG_SCALING_YSC, 0x80);
}
