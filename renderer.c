#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"
#include "info.h"

#ifndef MAX
#define MAX(a,b) (a>b)?a:b
#endif
#ifndef MIN
#define MIN(a,b) (a<b)?a:b
#endif

/* Pango helpers */
static PangoAlignment _alignment_from_opt(struct options_t options);

/* Cairo helpers */
static void resize_cairo_ctx(cairo_t *ctx, PangoLayout *layout);
static void move_cairo_ctx(cairo_t *ctx, struct options_t options,
                           int height);

/* Private functions */
static gulong render_text(cairo_t *cr, gulong size, struct options_t options);

/**********************************************/

const gchar *
get_last_line_text(PangoLayout *layout)
{
    GSList *last_line;
    PangoLayoutLine *line;
    const gchar *text;
    gint offset = 0;

    last_line = g_slist_last(pango_layout_get_lines_readonly(layout));
    line = (PangoLayoutLine *) last_line->data;

    offset = line->start_index;
    text = pango_layout_get_text(layout);

    return ((const gchar *) text+offset);
}

gboolean
wrap_is_well_formed(PangoLayout *layout, const gchar *cmpstr)
{
    const gchar *text;
    guint offset = 0;

    text = get_last_line_text(layout);
    offset = MAX(strlen(text),strlen(cmpstr)) - MIN(strlen(text),strlen(cmpstr));

    return (!strncmp(cmpstr+offset, text, strlen(cmpstr)-offset));
}

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
                break;
        }
    }

    assert(result != -1);

    return (PangoAlignment) result;
}

PangoLayout *
config_layout(PangoLayout *layout, struct options_t options)
{
    pango_layout_set_markup(layout, options.text, -1);
    pango_layout_set_width(layout, options.width * PANGO_SCALE);
    pango_layout_set_height(layout, options.height * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
    pango_layout_set_single_paragraph_mode(layout, FALSE);
    if(options.align)
        pango_layout_set_alignment(layout, _alignment_from_opt(options));

    return layout;
}

void
update_layout_size(cairo_t *cr, PangoLayout *layout,
                      PangoFontDescription *desc,
                      int size)
{
    /* Create and update the layout */
    pango_font_description_set_size(desc, size * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);
    pango_cairo_update_layout(cr, layout);
}

static void resize_cairo_ctx(cairo_t *ctx, PangoLayout *layout)
{
    double x1, x2, y1, y2;
    int pango_width, pango_height;

    pango_layout_get_pixel_size(layout, &pango_width, &pango_height);
    pango_cairo_layout_path(ctx, layout);
    cairo_path_extents(ctx, &x1, &y1, &x2, &y2);

    /*printf("%f x %f\n", pango_width/(x2-x1),pango_width/(x2-x1));*/
    /*cairo_scale(ctx, pango_width/(x2-x1),pango_width/(x2-x1));*/
}

static void move_cairo_ctx(cairo_t *ctx, struct options_t options,
                           int height)
{
    if(options.valign == CENTER_ALIGN)
        cairo_move_to(ctx, 0, (options.height/2) - (height/2));
    else if(options.valign == BOTTOM_ALIGN)
        cairo_move_to(ctx, 0, (options.height - height));

}

/*****************************************************/


static gulong 
render_text(cairo_t *cr, gulong size, struct options_t options)
{
    PangoLayout *layout;
    PangoFontDescription *description;
    int w, h;
    guint fix_size = 0;

    /* Create layout from a cairo context */
    layout = pango_cairo_create_layout(cr);

    layout = config_layout(layout, options);
    /* Add the font description passed by parameter
     * (for example: `Droid Sans Mono`) */
    description = pango_font_description_from_string(options.font);

    /* Set the size with a percent of the size */
    fix_size = size - (size * ((gfloat)options.fpa/100));
    pango_font_description_set_size(description,
                                    PANGO_SCALE*(fix_size));
    pango_layout_set_font_description(layout, description);
    pango_font_description_free(description);

    /* Get the size of the text in pixels */
    pango_layout_get_pixel_size(layout, &w, &h);
    /*printf("pango_layout_get_pixel_size (%d x %d)\n", w, h);*/

    /* Center vertically and check if we are out of the bounds of the surface */
    /* void cairo_scale(cairo_t *cr, double sx, double sy);*/
    /* Check alignment */

    move_cairo_ctx(cr, options, h);
    resize_cairo_ctx(cr, layout);

    /* Set surface to translucent color (r, g, b, a) without disturbing graphics state. */
    cairo_save (cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint (cr);
    cairo_restore (cr);

    /* Paint everything transparent */

    cairo_save(cr);
    pango_cairo_layout_path(cr, layout);

    /* fill the context */
    cairo_set_source_rgb(cr, options.text_color.red,
            options.text_color.green,
            options.text_color.blue);
    /* Preserve the fill to stroke it after */
    cairo_fill_preserve(cr);

    /* Draw the stroke */
    fix_size = size * ((gfloat)options.stroke_size/100);
    cairo_set_line_width(cr, options.stroke_size);
    cairo_set_source_rgb(cr,
            options.stroke_color.red,
            options.stroke_color.green,
            options.stroke_color.blue);
    cairo_stroke(cr);
    cairo_restore(cr);

    g_object_unref(layout);
    return size - (size * ((gfloat)options.fpa/100));
}

int
make_png(struct options_t options)
{
    gulong size = 0;
    cairo_t *cr = NULL;
    cairo_surface_t *surface = NULL;
    cairo_status_t status;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
            options.width,
            options.height);
    cr = cairo_create(surface);

    /* Transparent background */
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    size = get_font_size(options);

    size = render_text(cr, size, options);

    cairo_destroy(cr);
    status = cairo_surface_write_to_png(surface, options.filename);
    cairo_surface_destroy(surface);

    if(status > 0)
        return -status;

    return size;
}
