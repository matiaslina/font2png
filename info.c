#include <pango/pangocairo.h>
#include <stdio.h>

#include "info.h"
#include "renderer.h"

struct metrics_t {
    guint height;
    guint width;
    guint font;
};

/* Macros */
static char *printable_align(TextAlign align);
struct metrics_t get_metrics(struct options_t options);

unsigned int
get_font_size(struct options_t options) {
    struct metrics_t m = get_metrics(options);
    return m.font;
}

struct metrics_t
get_metrics(struct options_t options)
{
    int result = 1;
    unsigned int size = 1;
    int h = 0, w = 0;
    const gchar *last_line_str;
    cairo_t *cr = NULL;
    cairo_surface_t *surface = NULL;
    PangoLayout *layout = NULL;
    PangoFontDescription *desc = NULL;
    struct metrics_t metrics = { 0, 0, 0};

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            options.width,
            options.height);
    cr = cairo_create(surface);
    layout = pango_cairo_create_layout(cr);

    layout = config_layout(layout, options);
    desc = pango_font_description_from_string(options.font);

    last_line_str = get_last_line_text(layout);

    while(1)
    {
        update_layout_size(cr, layout, desc, size);
        pango_layout_get_pixel_size(layout, &w, &h);
        if(!wrap_is_well_formed(layout, last_line_str))
        {
            size = size/2;
            break;
        }

        if(h >= options.height || w >= options.width)
            break;
        result = size;
        size = size * 2;
    }
    /* Reached here, we have size = size*2 of what we want. */
    size = result;

    /* And now linealy. */
    while(1)
    {
        update_layout_size(cr, layout, desc, size);
        pango_layout_get_pixel_size(layout, &w, &h);

        if(!wrap_is_well_formed(layout, last_line_str))
        {
            size--;
            break;
        }

        if(h >= options.height || w >= options.width)
            break;
        result = size;
        size++;
    }

    pango_font_description_free(desc);

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return (metrics);
}

static char *
printable_align(TextAlign align)
{
    switch(align)
    {
        case NONE_ALIGN:
            return "none";
        case RIGHT_ALIGN:
            return "right";
        case LEFT_ALIGN:
            return "left";
        case CENTER_ALIGN:
            return "center";
        case TOP_ALIGN:
            return "top";
        case BOTTOM_ALIGN:
            return "bottom";
        case JUSTIFIED:
            return "justified";
    }
}

void
print_font_data(struct options_t options)
{
    struct metrics_t m = get_metrics(options);
    printf("%ux%u ", m.width, m.height); /* size in pixels */
    printf("%s ", options.font); /* font name */
    printf("%d ", m.font); /* font size */
    printf("%d ", options.fpa); /* padding */
    printf("%s ", printable_align(options.valign));/* vertical align */
    printf("%s ", printable_align(options.align));/* horizontal align */
    printf("\n");
}
