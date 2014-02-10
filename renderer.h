#ifndef RENDERER_H
#define RENDERER_H

typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} TextDirection;

typedef enum {
    RIGHT_ALIGN,
    LEFT_ALIGN,
    CENTER_ALIGN,
    JUSTIFIED,
} TextAlign;

struct options_t{
    char *text;
    char *font;

    int width;
    int height;

    TextAlign align;
    TextDirection direction;
    struct {
        double red;
        double green;
        double blue;
    } color;

    const char *filename;
};


int make_png(struct options_t options);

#endif
