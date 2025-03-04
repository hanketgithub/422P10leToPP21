//
//  main.c
//  422P10ToPP21
//
//  Created by Hank Lee on 2025-03-05.
//  Copyright (c) 2024 Hank Lee. All rights reserved.
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "pack.h"


#define MAX_WIDTH   7680
#define MAX_HEIGHT  4320


static uint8_t img[MAX_WIDTH * MAX_HEIGHT * 4];


int main(int argc, char *argv[])
{
    quatre_pixel q_pix;
    uint64_t *y;
    uint32_t *u;
    uint32_t *v;
    int ifd, ofd;

    int width;
    int height;
    int wxh;
    
    int i, j;
    int count = 0;
    ssize_t rd_sz;
    
    char *cp;
    char output[256] = { 0 };
    
    if (argc < 4)
    {
        fprintf(stderr, "usage: %s [input file] [width] [height]\n", argv[0]);
        
        return -1;
    }


    cp = NULL;


    // get input file name from comand line
    ifd = open(argv[1], O_RDONLY);
    if (ifd < 0)
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    // specify output file name
    cp = strrchr(argv[1], '.');
    strncpy(output, argv[1], cp - argv[1]);
    strcat(output, "_pp21");
    strcat(output, cp);

    ofd = open
            (
             output,
             O_WRONLY | O_CREAT | O_TRUNC,
             S_IRUSR
            );

    width   = atoi(argv[2]);
    height  = atoi(argv[3]);

    wxh = width * height;


    while (1)
    {
        rd_sz = read(ifd, img, wxh * 4);
        
        if (rd_sz == wxh * 4)
        {
            y = (uint64_t *) img;
            u = (uint32_t *) ((uint8_t *) img + width * height * 2);
            v = (uint32_t *) ((uint8_t *) img + width * height * 2 + width * height);
    
            // Y
            quatre_pixel *p = (quatre_pixel *) y;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width / 4; j++) // 4 horizantal pixel at a time
                {
                    memset(&q_pix, 0, sizeof(q_pix));

                    pack_y(&q_pix, (uint8_t *) y);

                    memcpy(p, &q_pix, sizeof(quatre_pixel));

                    y++;
                    p++;
                }
            }
            write(ofd, img, width * height / 4 * sizeof(quatre_pixel));
            
            // U, V
            quatre_pixel *q = p;
            for (i = 0; i < height; i++)
            {
                for (j = 0; j < width / 4; j++) // 2 horizontal pixel at a time
                {
                    memset(&q_pix, 0, sizeof(q_pix));
                    
                    pack_uv
                    (
                        &q_pix,
                        (uint8_t *) u,
                        (uint8_t *) v
                    );

                    memcpy(q, &q_pix, sizeof(quatre_pixel));

                    u++;
                    v++;
                    q++;
                }
            }
            write(ofd, p, width * height / 4 * sizeof(quatre_pixel));
            count++;
        }
        else
        {
            break;
        }
        fputc('.', stderr);
        fflush(stderr);
    }
    
    close(ifd);
    close(ofd);
    
    fprintf(stderr, "Done\n");
    fprintf(stderr, "Output file: %s\n", output);
    
    return 0;
}
