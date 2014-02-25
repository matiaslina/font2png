#include <assert.h>
#include <stdio.h>
#include <pango/pangocairo.h>
#include <string.h>

#include "renderer.h"

#define FONT        "Traditional Arabic"
#define SHORT_TEXT  "Hello world\0"
#define LONG_TEXT   "Hello my dear world, how are you? Hope that fine.\0"
#define ARABIC_TEXT "شيء يحشر السوبر المخادع"

/* Helpers */

/* Pango/Cairo helpers */
static PangoAlignment _alignment_from_opt(struct options_t options);
static PangoLayout *config_layout(PangoLayout *layout, struct options_t options);

static int get_font_size(struct options_t options);
static void render_text(cairo_t *cr, int size, struct options_t options);

static PangoAlignment
_alignment_from_opt(struct options_t options)
{
    /* By default return this.*/
    int result = -1;

    if(options.align != NONE_ALIGN)
    {
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
            default:
                fprintf(stderr, "_alignment_from_opt() undefined or none align option: %d",
                        options.align);
                result = PANGO_ALIGN_LEFT;
        }
    }

    assert(result != -1);

    return (PangoAlignment) result;
}
static PangoLayout *
config_layout(PangoLayout *layout, struct options_t options)
{
    pango_layout_set_text(layout, options.text, strlen(options.text));
    pango_layout_set_width(layout, options.width * PANGO_SCALE);
    pango_layout_set_height(layout, options.height * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
    if(options.align)
        pango_layout_set_alignment(layout, _alignment_from_opt(options));

    return layout;
}

static int
get_font_size(struct options_t options)
{
    int result = 1;
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

    layout = config_layout(layout, options);
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
    size = result/2;

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

    pango_font_description_free(desc);

    g_object_unref(layout);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    result = result - 1;

    return (result);
}

static void
render_text(cairo_t *cr, int size, struct options_t options)
{
    PangoLayout *layout;
    PangoFontDescription *description;
    int w, h;

    /* Create layout from a cairo context */
    layout = pango_cairo_create_layout(cr);

    layout = config_layout(layout, options);
    /* Add the font description passed by parameter (for example: `Droid Sans Mono`) */
    description = pango_font_description_from_string(options.font);

    pango_font_description_set_size(description, (size - options.fpa) * PANGO_SCALE);

    pango_layout_set_font_description(layout, description);
    pango_font_description_free(description);

    /* Get the size of the text in pixels */
    pango_layout_get_pixel_size(layout, &w, &h);

    printf("pango_layout_get_pixel_size(%d,%d)\n", w, h);
    /* Center vertically */
    cairo_move_to(cr, 0, (options.height/2) - (h/2));


    /* Update cairo with the layout */
    pango_cairo_update_layout(cr, layout);
    /* And set the path of the fonts. */
    pango_cairo_layout_path(cr, layout);

    cairo_set_line_width(cr, options.stroke_size);
    cairo_set_source_rgb(cr,
                         options.stroke_color.red,
                         options.stroke_color.green,
                         options.stroke_color.blue);
    cairo_stroke_preserve(cr);

    cairo_set_source_rgb(cr, options.text_color.red,
                         options.text_color.green,
                         options.text_color.blue);
    cairo_fill(cr);


    g_object_unref(layout);
}

int
make_png(struct options_t options)
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
    
    return status;
}
