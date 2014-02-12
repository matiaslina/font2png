#include <stdio.h>
#include <pango/pangocairo.h>

#include "renderer.h"

#define FONT        "Traditional Arabic"
#define SHORT_TEXT  "Hello world\0"
#define LONG_TEXT   "Hello my dear world, how are you? Hope that fine.\0"
#define ARABIC_TEXT "شيء يحشر السوبر المخادع"

/* Option helpers */
static PangoAlignment _alignment_from_opt(struct options_t options);

static PangoAlignment _alignment_from_opt(struct options_t options)
{
    /* By default return this.*/
    PangoAlignment result = PANGO_ALIGN_LEFT;

    switch(options.align)
    {
        case CENTER_ALIGN:
            result = PANGO_ALIGN_CENTER;
            break;
        case LEFT_ALIGN:
            result = PANGO_ALIGN_LEFT;
            break;
        case RIGHT_ALIGN:
            result = PANGO_ALIGN_RIGHT;
            break;
        case JUSTIFIED:
            /* Not handled now*/
            break;
    }

    return result;
}


static int get_font_size(struct options_t options);
static void render_text(cairo_t *cr, int size, struct options_t options);

static int get_font_size(struct options_t options)
{
    int result = 0;
    int size = 1;
    int h = 0, w = 0;
    cairo_t *cr = NULL;
    cairo_surface_t *surface = NULL;
    PangoLayout *layout = NULL;
    PangoFontDescription *desc = NULL;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         options.width,
                                         options.height);
    cr = cairo_create(surface);
    layout = pango_cairo_create_layout(cr);

    pango_layout_set_text(layout, options.text, -1);
    pango_layout_set_width(layout, options.width * PANGO_SCALE);
    pango_layout_set_height(layout, options.height * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
    if(options.align)
        pango_layout_set_alignment(layout, _alignment_from_opt(options));

    /* every size will be aumented by a factor of 2 */
    desc = pango_font_description_from_string(options.font);
    while(1)
    {
        /* Create and update the layout */
        pango_font_description_set_size(desc, size * PANGO_SCALE);
        pango_layout_set_font_description(layout, desc);
        pango_cairo_update_layout(cr, layout);

        pango_layout_get_pixel_size(layout, &w, &h);
        if(h <= options.height && w <= options.width)
        {
            result = size;
            size = size * 2;
        }
        else
            break;
    }
    /* Reached here, we have size = size*2 of what we want. */
    size = result;

    /* And now linealy. */
    while(1)
    {
        /* Create and update the  layout */
        pango_font_description_set_size(desc, size * PANGO_SCALE);
        pango_layout_set_font_description(layout, desc);
        pango_cairo_update_layout(cr, layout);

        pango_layout_get_pixel_size(layout, &w, &h);
        if(h <= options.height && w <= options.width)
        {
            result = size;
            size++;
        }
        else
            break;
    }
    /* Free all */
    pango_font_description_free(desc);

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    result = result - 1;

    return (result);
}

static void render_text(cairo_t *cr, int size, struct options_t options)
{
    PangoLayout *layout;
    PangoFontDescription *desc;

    layout = pango_cairo_create_layout(cr);

    pango_layout_set_text(layout, options.text, -1);
    pango_layout_set_width(layout, options.width * PANGO_SCALE);
    pango_layout_set_height(layout, options.height * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
    if(options.align)
        pango_layout_set_alignment(layout, _alignment_from_opt(options));

    desc = pango_font_description_from_string(options.font);
    pango_font_description_set_size(desc, (size - options.fpa) * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);

    cairo_set_source_rgb(cr, options.color.red,
                         options.color.green,
                         options.color.blue);
    pango_cairo_update_layout(cr, layout);

    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
}

int make_png(struct options_t options)
{
    int size = 0;
    cairo_t *cr = NULL;
    cairo_surface_t *surface = NULL;
    cairo_status_t status;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                         options.width,
                                         options.height);
    cr = cairo_create(surface);
    cairo_set_source_rgba(cr, 0.0,0.0,0.0, 0.0);
    cairo_paint(cr);
    
    size = get_font_size(options);

    render_text(cr, size, options);

    cairo_destroy(cr);
    status = cairo_surface_write_to_png(surface, options.filename);
    cairo_surface_destroy(surface);
    
    return 0;
}
