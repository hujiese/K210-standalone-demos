/**
* @par  Copyright (C): 2016-2022, Shenzhen Yahboom Tech
* @file         main.c
* @author       Gengyue
* @version      V1.0
* @date         2020.05.27
* @brief        TF卡读写文件例程
* @details      
* @par History  见如下说明
*                 
* version:	V1.0: 写入数据到TF中，然后再读出来。
*                 烧录固件后，需要重新给开发板上电，否则会出现内存卡初始化失败。
*/
#include <stdio.h>
#include "sysctl.h"
#include "dmac.h"
#include "fpioa.h"
#include "sdcard.h"
#include "ff.h"
#include "i2s.h"
#include "plic.h"
#include "uarths.h"
#include "bsp.h"
#include "pin_config.h"


static int check_sdcard(void);
static int check_fat32(void);
FRESULT sd_write_file(TCHAR *path);
FRESULT sd_read_file(TCHAR *path);


/**
* Function       hardware_init
* @author        Gengyue
* @date          2020.05.27
* @brief         硬件初始化，绑定GPIO口
* @param[in]     void
* @param[out]    void
* @retval        void
* @par History   无
*/
void hardware_init(void)
{
    /*
    ** io26--miso--d1
    ** io27--clk---sclk
    ** io28--mosi--d0
    ** io29--cs----cs
    */
    fpioa_set_function(PIN_TF_MISO, FUNC_TF_SPI_MISO);
    fpioa_set_function(PIN_TF_CLK,  FUNC_TF_SPI_CLK);
    fpioa_set_function(PIN_TF_MOSI, FUNC_TF_SPI_MOSI);
    fpioa_set_function(PIN_TF_CS,   FUNC_TF_SPI_CS);
}

/**
* Function       main
* @author        Gengyue
* @date          2020.05.27
* @brief         主函数，程序的入口
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
int main(void)
{
    // 硬件引脚初始化
    hardware_init();

    /* 设置系统时钟频率 */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 300000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    if (check_sdcard())
    {
        printf("SD card err\n");
        return -1;
    }

    if (check_fat32())
    {
        printf("FAT32 err\n");
        return -1;
    }

    sleep(1);
    if (sd_write_file(_T("test.txt")))
    {
        printf("SD write err\n");
        return -1;
    }

    if (sd_read_file(_T("test.txt")))
    {
        printf("SD read err\n");
        return -1;
    }
    
    while (1);
    return 0;
}

/**
* Function       check_sdcard
* @author        Gengyue
* @date          2020.05.27
* @brief         检测TF是否正常
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
static int check_sdcard(void)
{
    uint8_t status;

    printf("/******************check_sdcard*****************/\n");
    status = sd_init();
    printf("sd init :%d\n", status);
    if (status != 0)
    {
        return status;
    }

    printf("CardCapacity:%.1fG \n", (double)cardinfo.CardCapacity / 1024 / 1024 / 1024);
    printf("CardBlockSize:%d\n", cardinfo.CardBlockSize);
    return 0;
}

/**
* Function       check_fat32
* @author        Gengyue
* @date          2020.05.27
* @brief         检测TF的格式是否FAT32
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
static int check_fat32(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    printf("/********************check_fat32*******************/\n");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    printf("mount sdcard:%d\n", status);
    if (status != FR_OK)
        return status;

    printf("printf filename\n");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while (status == FR_OK && fno.fname[0])
    {
        if (fno.fattrib & AM_DIR)
            printf("dir:%s\n", fno.fname);
        else
            printf("file:%s\n", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

/**
* Function       sd_write_file
* @author        Gengyue
* @date          2020.05.27
* @brief         写入文件到TF卡
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
FRESULT sd_write_file(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    printf("/*******************sd_write_file*******************/\n");
    uint32_t v_ret_len = 0;

    /* 打开文件，如果文件不存在，则新建 */
    if ((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
    {
        printf("open file %s err[%d]\n", path, ret);
        return ret;
    }
    else
    {
        printf("Open %s ok\n", path);
    }

    /* 要写入的数据 */
    uint8_t data[] = {'H','i',',','D','a','t','a',' ','W','r','i','t','e',' ','O','k','!'};

    /* 写入数据 */
    ret = f_write(&file, data, sizeof(data), &v_ret_len);
    if (ret != FR_OK)
    {
        printf("Write %s err[%d]\n", path, ret);
    }
    else
    {
        printf("Write %d bytes to %s ok\n", v_ret_len, path);
    }
    /* 关闭文件 */
    f_close(&file);
    return ret;
}

/**
* Function       sd_read_file
* @author        Gengyue
* @date          2020.05.27
* @brief         从TF卡读取文件
* @param[in]     void
* @param[out]    void
* @retval        0
* @par History   无
*/
FRESULT sd_read_file(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    printf("/*******************sd_read_file*******************/\n");
    uint32_t v_ret_len = 0;

    /* 检测文件状态 */
    FILINFO v_fileinfo;
    if ((ret = f_stat(path, &v_fileinfo)) == FR_OK)
    {
        printf("%s length is %lld\n", path, v_fileinfo.fsize);
    }
    else
    {
        printf("%s fstat err [%d]\n", path, ret);
        return ret;
    }

    /* 只读方式打开文件 */
    if ((ret = f_open(&file, path, FA_READ)) == FR_OK)
    {
        char v_buf[64] = {0};
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        if (ret != FR_OK)
        {
            printf("Read %s err[%d]\n", path, ret);
        }
        else
        {
            printf("Read :> %s \n", v_buf);
            printf("total %d bytes lenth\n", v_ret_len);
        }
        /* 关闭文件 */
        f_close(&file);
    }
    return ret;
}
