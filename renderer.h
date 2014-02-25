#ifndef RENDERER_H
#define RENDERER_H

typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} TextDirection;

typedef enum {
    NONE_ALIGN = 0,
    RIGHT_ALIGN,
    LEFT_ALIGN,
    CENTER_ALIGN,
    JUSTIFIED,
} TextAlign;

typedef struct _color {
    double red;
    double green;
    double blue;
} Color;


struct options_t{
    char *text;
    char *font;

    int width;
    int height;

    TextAlign align;
    TextDirection direction;
    int fpa;                /* Font Pixel Adjust */
    Color text_color;

    /* Stroke */
    Color stroke_color;
    double stroke_size;

    const char *filename;
};


int make_png(struct options_t options);

#endif
