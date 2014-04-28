#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"
#include "info.h"
#include "renderer.h"

static void show_help(void);
static void colors_from_string(char *str, Color *c);
static void parse_stroke(char *str, Color *c, double *size);

static void show_help(void)
{
    printf("font2png [OPTIONS] -o file.png\n"
            "--help                     Print this help\n"
            "-w n                       Width of n pixels\n"
            "-h n                       Height of n pixels\n"
            "-f font                    Font to render\n"
            "-o filename                Output file (required)\n"
            "-a (center|left|right)     horizontal alignment\n"
            "-v (center|top|bottom)     vertical alignment\n"
            "-c XXXXXXX                 Color in hexadecimal\n"
            "-s XXXXXXX:n               Stroke with size n and color XXXXXXX\n"
            "-p n                       Pixels that the font is reduced\n"
          );
}
static void colors_from_string(char *str, Color *c)
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


    c->red = (double) xr/0xFF;
    c->green = (double) xg/0xFF;
    c->blue = (double) xb/0xFF;
}

static void parse_stroke(char *str, Color *c, double *size)
{
    char *hexcode;

    hexcode = malloc(7*sizeof(char));

    /* The first 6 digits of str are the hexadecimal number of
     * the color. */
    hexcode = strncpy(hexcode, str, 6);
    colors_from_string(hexcode, c);

    /* And the rest is the size of the stroke */
    *size = atof(&(str[7]));
}


int main(int argc, char *argv[])
{
    /* Check if we need to process the image or not */
    int process = 1;
    int opt = 0, result = 0;
    char *text = NULL;
    char *buffer = calloc(256, sizeof(char));
    size_t text_len = 0;
    ssize_t count = 0;
    opterr= 0;
    struct options_t options = {
        .text       = NULL,
        .font       = "Arial",
        .filename   = NULL,
        .width      = 0,
        .height     = 0,
        .text_color = {
            .red    = 0.0f,
            .green  = 0.0f,
            .blue   = 0.0f
        },
        .stroke_color = {
            .red    = 0.0f,
            .green  = 0.0f,
            .blue   = 0.0f
        },
        .stroke_size = 0.0,
        .fpa        = 0,
        .align      = LEFT_ALIGN,
        .valign     = CENTER_ALIGN
    };

    if(argc > 1 && (strcmp(argv[1], "--help") == 0))
    {
        show_help();
        return 0;
    }

    while((opt = getopt(argc, argv, "xf:h:w:o:c:a:p:s:v:")) != -1)
    {
        switch(opt)
        {
            case 'x':
                process = 0;
                break;
            case 'p':
                options.fpa = atoi(optarg);
                if(options.fpa > 100 || options.fpa < 0)
                {
                    fprintf(stderr, "padding must be inside [0,100). Passed %d\n", options.fpa);
                    return -6;
                }
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
                                    "with 'left', 'right' or 'center'");
                    options.align = NONE_ALIGN;
                }
                break;
            case 'v':
                /* The default align is center */
                if(strcmp(optarg, "top") == 0)
                    options.valign = TOP_ALIGN;
                else if(strcmp(optarg, "bottom") == 0)
                    options.valign = BOTTOM_ALIGN;
                else
                    options.valign = CENTER_ALIGN;
                break;
            case 'c':
                if(strlen(optarg) != 6)
                {
                    fprintf(stderr, "You need to especify a hexa number with 6 digits\n");
                    return -4;
                }
                colors_from_string(optarg, &(options.text_color));
                break;
            case 's':
                if(strlen(optarg) < 5)
                {
                    fprintf(stderr, "There's some error with the stroke syntax\n");
                    return -5;
                }
                parse_stroke(optarg, &(options.stroke_color), &(options.stroke_size));
                if(options.stroke_size == 0.0)
                {
                    fprintf(stderr, "Failed to parse the stroke size\n");
                    return -6;
                }
                break;
            default:
                break;
        }
    }

    if(options.width <= 0 || options.height <= 0)
    {
        fprintf(stderr, "You're forgoting a size parameter!");
        return -3;
    }
    if(options.filename == NULL && process)
    {
        fprintf(stderr, "You're forgoting the filename parameter!");
        return -3;
    }

    /* Get the text from stdin */
    while((count = read(STDIN_FILENO, buffer, 256)) > 0)
    {
        text = realloc(text, text_len + count + 1);
        memset(text+text_len, 0, count +1);
        text = strncat(text, buffer, count);
        text_len += count;
    }

    if(count == -1)
    {
        fprintf(stderr, "Failed to read()");
        return -7;
    }

    options.text = text;

    if(process)
        result = make_png(options);
    else
        print_font_data(options);

    free(text);
    free(buffer);

    /* Exits with a status of cairo defined
     * in http://cairographics.org/manual/cairo-PNG-Support.html#cairo-surface-write-to-png
     */
    if(result <= 0)
        return result;

    printf("%d\n", result);

    return 0;
}
