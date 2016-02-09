#include<xinu.h>

shellcmd xsh_reset(int nargs, char *args[])
    {
        volatile uint32 *prm_rstctrl1 = (volatile uint32 *) 0x44E00F00;
        *prm_rstctrl1=0x01;
        }
