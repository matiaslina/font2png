#ifndef RENDERER_H
#define RENDERER_H

#include <pango/pangocairo.h>
#include "defs.h"

/* Helpers */
const gchar *get_last_line_text(PangoLayout *layout);

PangoLayout *config_layout(PangoLayout *layout, struct options_t options);
void update_layout_size(cairo_t *cr, PangoLayout *layout,
                        PangoFontDescription *desc,
                        int size);
gboolean wrap_is_well_formed(PangoLayout *layout, const gchar *cmpstr);

int make_png(struct options_t options);

#endif
