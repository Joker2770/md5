/*************************************************************************
    > File Name: main.c
    > Author: Jintao Yang
    > Mail: 18608842770@163.com 
    > Created Time: 2022年04月24日 星期日 14时39分30秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "md5.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUF_LEN (1024 * 1024 * 1024)

int calc_md5_s(const char *src, char *dest)
{
    int read_len = 0;
    int i = 0;
    char temp[8] = {0};
    unsigned char decrypt[16] = {0};

    MD5_CTX md5c;

    MD5Init(&md5c); //初始化
    read_len = strlen(src);
    MD5Update(&md5c, (unsigned char *)src, read_len);

    MD5Final(&md5c, decrypt);

    for (i = 0; i < 16; i++)
    {
        sprintf(temp, "%02x", decrypt[i]);
        if (NULL != dest)
        {
            strcat((char *)dest, temp);
        }
    }
    //printf("md5:%s\n", dest);

    return read_len;
}

int calc_md5_f(const char *filename, size_t f_size, char *dest)
{
    int i = 0;
    int filelen = 0;
    int read_len;
    char temp[8] = {0};
    char *buf = NULL;
    buf = (char*)malloc(sizeof(char) * f_size);
    unsigned char decrypt[16] = {0};
    MD5_CTX md5;
    char fw_path[128];

    int fdf;

    fdf = open(filename, O_RDWR);
    if (fdf < 0)
    {
        printf("%s not exist\n", filename);
        return -1;
    }

    MD5Init(&md5);
    while (1)
    {
        read_len = read(fdf, buf, sizeof(buf));
        if (read_len < 0)
        {
            close(fdf);
            if (NULL != buf)
            {
                free(buf);
                buf = NULL;
            }
            return -1;
        }
        if (read_len == 0)
        {
            break;
        }
        filelen += read_len;
        MD5Update(&md5, (unsigned char *)buf, read_len);
    }

    MD5Final(&md5, decrypt);

    for (i = 0; i < 16; i++)
    {
        sprintf(temp, "%02x", decrypt[i]);
        if (NULL != dest)
        {
            strcat((char *)dest, temp);
        }
    }

    //printf("md5:%s len=%d\n", dest, filelen);
    close(fdf);
    if (NULL != buf)
    {
        free(buf);
        buf = NULL;
    }

    return filelen;
}

int main(int argc, char *argv[])
{
    if (argc == 3)
    {
        int iLen = 0;
        char szDest[32 + 1] = "";
        memset(szDest, 0, sizeof(szDest));
        if (0 == strcmp(argv[1], "-s"))
        {
            iLen = calc_md5_s(argv[2], szDest);
            if (strlen(szDest) > 0 && iLen > 0)
                printf("md5: %s\n", szDest);
        }
        else if (0 == strcmp(argv[1], "-f"))
        {
            iLen = calc_md5_f(argv[2], MAX_BUF_LEN, szDest);
            if (strlen(szDest) > 0 && iLen > 0)
                printf("md5: %s\n", szDest);
        }
        else
        {
            printf(
                "%s [option] [string | path]\n"
                "all options: \n"
                "    -s    mode for string calculation. \n"
                "    -f    mode for file calculation. \n",
                argv[0]);
        }
    }
    else
    {
        printf(
            "%s [option] [string | path]\n"
            "all options: \n"
            "    -s    mode for string calculation. \n"
            "    -f    mode for file calculation. \n",
            argv[0]);
    }

    return 0;
}

