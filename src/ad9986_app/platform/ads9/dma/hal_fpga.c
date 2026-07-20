/*!
 * @brief     HAL FPGA Source File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_HAL_FPGA__
 * @{
 */




/*============= I N C L U D E S ============*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>



/*============= D E F I N E S ==============*/
//#define FPGA_BASEADDR                      0x40000000
//#define AXIFPGADEV                         "/dev/uio3"
#define AXIFPGA_SIZE                       0x10000


int g_fd;
int g_baseaddr;
int g_size;
volatile void* g_membase;




/*============= C O D E ====================*/
unsigned int Hal_FpgaInit(unsigned int baseaddr, unsigned int size)
{
    int err = -1;

    g_baseaddr = g_size = 0;
    g_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (g_fd > 0) {
        g_membase = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, baseaddr);
        if (g_membase != (void*)-1) {
            g_baseaddr = baseaddr;
            g_size = size;
            err = 0;
        }
        else {
            printf("/dev/mem memory map error.\r\n");
        }
    }
    else {
        printf("Open /dev/mem device error.\r\n");
    }



#if 0
    if (access(AXIFPGADEV, F_OK) == 0) {
        g_fd = open(AXIFPGADEV, O_RDWR | O_SYNC);
        if (g_fd > 0) {
            g_membase = mmap(0, AXIFPGA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_fd, 0);
            if (g_membase != (void*)-1) {
                err = 0;
            }
            else {
                printf("/dev/mem memory map error.\r\n");
            }
        }
        else {
            printf("Open FPGA UIO device error.\r\n");
        }
    }
    else {
        printf("The UIO device doesn't exist.\r\n");
    }
#endif

    return err;
}


unsigned int Hal_FpgaAddr(unsigned int reg)
{
    return ((unsigned int)g_membase + (reg << 2));
}


unsigned int Hal_FpgaIn32(unsigned int reg)
{
    unsigned int vl;

    vl = *((volatile unsigned int*)((unsigned int)g_membase + (reg << 2)));
    //printf("The offset is 0x%x, and value is 0x%x.\r\n", (reg << 2), vl);

    return vl;
}


unsigned int Hal_FpgaOut32(unsigned int reg, unsigned int Value)
{
    //printf("offset is 0x%x, data is 0x%x.\r\n", (reg << 2), Value);
    *((volatile unsigned int*)((unsigned int)g_membase + (reg << 2))) = Value;

    return 0;
}

unsigned int Hal_FpgaDeinit(void)
{
    if ((g_baseaddr != 0) && (g_size != 0)) {
        munmap((void*)g_membase, g_size);
        g_baseaddr = g_size = 0;
    }
    close(g_fd);
    g_membase = NULL;

#if 0
    if (g_membase != NULL) {
        munmap((void*)g_membase, AXIFPGA_SIZE);
        g_membase = NULL;
    }
    close(g_fd);
#endif


    return 0;
}

/*! @} */
