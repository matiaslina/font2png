#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "renderer.h"

static void show_help(void);
static void colors_from_string(char *str, double *r, double *g, double *b);

static void show_help(void)
{
    printf("font2png [OPTIONS] -o file.png\n"
            "--help                     Print this help\n"
            "-w n                       Width of n pixels\n"
            "-h n                       Height of n pixels\n"
            "-t str                     Input text\n"
            "-f font                    Font to render\n"
            "-o filename                Output file (required)\n"
            "-a (center|left|right)     alignment\n"
            "-c XXXXXXX                 Color in hexadecimal\n"
            "-p n                       Pixels that the font is reduced\n"
          );
}
static void colors_from_string(char *str, double *r, double *g, double *b)
{
    char sr[3];
    char sg[3];
    char sb[3];
    unsigned int xr,xg,xb;
    
    sr[0] = str[0];
    sr[1] = str[1];
    sr[2] = '\0';
    sg[0] = str[2];
    sg[1] = str[3];
    sg[2] = '\0';
    sb[0] = str[4];
    sb[1] = str[5];
    sb[2] = '\0';

    sscanf(sr, "%x", &xr);
    sscanf(sg, "%x", &xg);
    sscanf(sb, "%x", &xb);

    *r =(double) xr/0xFF;
    *g =(double) xg/0xFF;
    *b =(double) xb/0xFF;
}

int main(int argc, char *argv[])
{
    int opt;

    opterr= 0;

    struct options_t options = {
        .text       = NULL,
        .font       = "Arial",
        .filename   = NULL,
        .width      = 0,
        .height     = 0,
        .color      = {
            .red    = 0.0f,
            .green  = 0.0f,
            .blue   = 0.0f
        },
        .fpa        = 0,
        .align      = LEFT_ALIGN,
    };

    if(argc > 1 && (strcmp(argv[1], "--help") == 0))
    {
        show_help();
        return 0;
    }

    while((opt = getopt(argc, argv, "f:h:w:o:c:a:t:p:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                options.fpa = atoi(optarg);
                break;
            case 'f':
                options.font = optarg;
                break;
            case 'h':
                options.height = atoi(optarg);
                break;
            case 'w':
                options.width = atoi(optarg);
                break;
            case 'o':
                options.filename = (const char *) optarg;
                break;
            case 'd':
                if(strcmp(optarg, "left") == 0)
                    options.direction = LEFT_TO_RIGHT;
                else if(strcmp(optarg, "right") == 0)
                    options.direction = RIGHT_TO_LEFT;
                else
                {
                    fprintf(stderr, "Unknown option for -d. You should try"
                                    "with 'left' or 'right'"
                           );
                    return -1;
                }
                break;
            case 'a':
                if(strcmp(optarg, "right") == 0)
                    options.align = RIGHT_ALIGN;

                else if(strcmp(optarg, "left") == 0)
                    options.align = LEFT_ALIGN;
                else if(strcmp(optarg, "center") == 0)
                    options.align = CENTER_ALIGN;
                else
                {
                    fprintf(stderr, "Unknown option for -a. You should try"
                                    "with 'left', 'right' or 'center'"
                           );
                    return -2;
                }
                break;
            case 't':
                options.text = optarg;
                break;
            case 'c':
                if(strlen(optarg) != 6)
                {
                    fprintf(stderr, "You need to especify a hexa number with 6 digits\n");
                    return -4;
                }
                colors_from_string(optarg,
                                   &(options.color.red),
                                   &(options.color.green),
                                   &(options.color.blue));
            default:
                break;
        }
    }

    if(options.width <= 0 || options.height <= 0 ||
       options.filename == NULL || options.text == NULL)
    {
        fprintf(stderr, "You're forgoting a parameter!");
        return -3;
    }

    make_png(options);
    
    return 0;
}
