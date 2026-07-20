/*!
 * @brief     Hal CDMA Source File
 *
 * @copyright copyright(c) 2018 analog devices, inc. all rights reserved.
 *            This software is proprietary to Analog Devices, Inc. and its
 *            licensor. By using this software you agree to the terms of the
 *            associated analog devices software license agreement.
 */

/*!
 * @addtogroup __ADI_HAL_CDMA__
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
int g_fd;
int g_baseaddr;
int g_size;
volatile void* g_membase;


/*============= C O D E ====================*/
unsigned int Hal_CdmaInit(unsigned int baseaddr, unsigned int size)
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


    return err;
}


unsigned int Hal_CdmaIn32(unsigned int offset)
{
    unsigned int vl;

    vl = *((volatile unsigned int*)((unsigned int)g_membase + offset));
    //printf("The Address is 0x%x, and value is 0x%x.\r\n", (unsigned int)g_membase + offset, vl);

    return vl;
}


unsigned int Hal_CdmaOut32(unsigned int offset, unsigned int Value)
{
    *((volatile unsigned int*)((unsigned int)g_membase + offset)) = Value;

    return 0;
}

unsigned int Hal_CdmaDeinit(void)
{
    if ((g_baseaddr != 0) && (g_size != 0)) {
        munmap((void*)g_membase, g_size);
        g_baseaddr = g_size = 0;
    }
    close(g_fd);
    g_membase = NULL;


    return 0;
}

/*! @} */
