
#include "platform.h"
#include "xil_printf.h"
#include "xiicps.h"
#include "camera.h"

int main()
{
	int result;

    init_platform();

    xil_printf("Start Initialize Camera Module\n");
    result = InitCamera();
    if (result != XST_SUCCESS) {
    	xil_printf("Camera Module Initialization failed\n");
    	cleanup_platform();
    	return (-1);
    }
    xil_printf("Reset Camera Module\n");

    xil_printf("Camera Module default register dump\n");
    DumpReg();

    xil_printf("Set RGB 565 mode\n");
    InitRGB565();

    xil_printf("Set QVGA mode\n");
    InitQVGA();
    xil_printf("Load default Camera registers\n");
    InitDefaultReg();

    //xil_printf("Camera Test\n");
    //ShowQVGA_ColorBar();

    xil_printf("\nCamera Module register dump after initialization\n");
    DumpReg();

    while(1) {
    	// dummy loop
    }

    cleanup_platform();
    return 0;
}
