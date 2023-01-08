/*************************************************************************
    > File Name: main.c
    > Author: Jintao Yang
    > Mail: 18608842770@163.com 
    > Created Time: 2022年04月24日 星期日 14时39分30秒
 ************************************************************************/

/*
    A simple tool for calculating MD5. 
    Copyright (C) 2022-2023  joker2770

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif // __linux
#ifdef _MSC_VER
#include <windows.h>
#endif //_MSC_VER

#define MAX_BUF_LEN (1024 * 1024)

size_t f_size(FILE *fp)
{
    size_t n;
    fpos_t fpos;

    if (fp == NULL)
        return -1;

    fgetpos(fp, &fpos);
    fseek(fp, 0, SEEK_END);
    n = ftell(fp);
    fsetpos(fp, &fpos);

    return n;
}

int calc_md5_s(const char *src, char *dest)
{
    int read_len = 0;
    int i = 0;
    char temp[8] = {0};
    unsigned char decrypt[16] = {0};
    MD5_CTX md5c;

    read_len = strlen(src);

    MD5Init(&md5c);
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

int calc_md5_f(const char *filename, size_t bf_size, char *dest)
{
    int i = 0;
    size_t filelen = 0;
    size_t read_len = 0;
    char temp[8] = {0};
    unsigned char *buf = NULL;
    buf = (unsigned char*)malloc(sizeof(unsigned char) * bf_size);
    unsigned char decrypt[16] = {0};
    MD5_CTX md5;
    size_t lfsize = 0;
    FILE *fdf = NULL;
    FILE *fp = NULL;

    fdf = fopen(filename, "rb");
    if (NULL == fdf)
    {
        printf("%s not exist\n", filename);
        if (NULL != buf)
        {
            free(buf);
            buf = NULL;
        }
        return -1;
    }

    fp = fopen(filename, "rb");
    lfsize = f_size(fp);
    if (lfsize <= 0)
    {
        printf("Failed to count file size!\n");
        if (NULL != buf)
        {
            free(buf);
            buf = NULL;
        }
        return -2;
    }
    else
        printf("file size: %zu bytes\n", lfsize);

    if (NULL != fp)
        fclose(fp);

    MD5Init(&md5);
    while (1)
    {
        memset(buf, 0, sizeof(unsigned char) * bf_size);
        read_len = fread(buf, sizeof(unsigned char), sizeof(unsigned char) * bf_size, fdf);
        if (read_len < 0)
        {
            fclose(fdf);
            if (NULL != buf)
            {
                free(buf);
                buf = NULL;
            }
            return -1;
        }
        if (read_len == 0)
            break;

        filelen += read_len;
        MD5Update(&md5, (unsigned char *)buf, read_len);

        printf("[TIME USED = %6.2f MINUTES] [%zu%%]\r", (double)clock() / CLOCKS_PER_SEC / 60, filelen * 100 / lfsize);
        fflush(stdout);
    }
    printf("\n");

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
    fclose(fdf);
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
            else if (strlen(szDest) <= 0)
                printf("internal error!\n");
        }
        else if (0 == strcmp(argv[1], "-f"))
        {
            iLen = calc_md5_f(argv[2], MAX_BUF_LEN, szDest);
            if (strlen(szDest) > 0 && iLen > 0)
                printf("md5: %s\n", szDest);
            else if (strlen(szDest) <= 0)
                printf("internal error!\n");
        }
        else
        {
            printf(
                "%s [option] [string|path]\n"
                "all options: \n"
                "    -s    mode for string calculation. \n"
                "    -f    mode for file calculation. \n",
                argv[0]);
        }
    }
    else
    {
        printf(
            "A simple tool for calculating MD5. \n"
            "Copyright (C) 2022-2023  joker2770 \n"
            "\n"
            "This program is free software; you can redistribute it and/or modify \n"
            "it under the terms of the GNU General Public License as published by \n"
            "the Free Software Foundation; either version 2 of the License, or \n"
            "(at your option) any later version. \n"
            "\n"
            "This program is distributed in the hope that it will be useful, \n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of \n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n"
            "GNU General Public License for more details. \n"
            "\n"
            "You should have received a copy of the GNU General Public License along \n"
            "with this program; if not, write to the Free Software Foundation, Inc., \n"
            "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. \n"
            "\n"
            "%s [option] [string|path]\n"
            "all options: \n"
            "    -s    mode for string calculation. \n"
            "    -f    mode for file calculation. \n",
            argv[0]);
    }

    return 0;
}

