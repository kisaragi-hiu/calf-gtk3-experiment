/* Calf DSP Library
 * Custom controls (line graph, knob).
 * Copyright (C) 2007-2015 Krzysztof Foltman, Torben Hohn, Markus Schmidt
 * and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include <calf/ctl_notebook.h>

using namespace calf_plugins;
using namespace dsp;


///////////////////////////////////////// notebook ///////////////////////////////////////////////

GtkWidget *
calf_notebook_new()
{
    GtkWidget *widget = GTK_WIDGET( g_object_new (CALF_TYPE_NOTEBOOK, NULL ));
    return widget;
}
static gboolean
calf_notebook_draw (GtkWidget *widget, cairo_t *cr)
{
    g_assert(CALF_IS_NOTEBOOK(widget));
    
    GtkNotebook *notebook = GTK_NOTEBOOK(widget);
    CalfNotebook *nb = CALF_NOTEBOOK(widget);
    
    if (gtk_widget_is_drawable (widget)) {
        
        // GdkWindow *window = gtk_widget_get_window(widget);
        cairo_pattern_t *pat = NULL;
        
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        int x  = allocation.x;
        int y  = allocation.y;
        int sx = allocation.width;
        int sy = allocation.height;
        int tx = gtk_widget_get_style(widget)->xthickness;
        int ty = gtk_widget_get_style(widget)->ythickness;
        int lh = 19;
        int bh = lh + 2 * ty;
        
        float r, g, b;
        float alpha;
        gtk_widget_style_get(widget, "background-alpha", &alpha, NULL);
        
        cairo_rectangle(cr, x, y, sx, sy);
        cairo_clip(cr);
        
        int add = 0;
        
        if (gtk_notebook_get_show_tabs(notebook)) {
            GtkWidget *page;
            GtkWidget *cur_page;
            GList *pages;
            GtkWidget *tab_label;
            
            gint sp;
            gtk_widget_style_get(widget, "tab-overlap", &sp, NULL);
            
            pages = gtk_container_get_children(GTK_CONTAINER(notebook));
            
            int cn = 0;
            while (pages) {
                cur_page = gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook));
                page = GTK_WIDGET(pages->data);
                pages = pages->next;
                tab_label = gtk_notebook_get_tab_label(notebook, page);
                if (gtk_widget_get_window(tab_label) ==
                      gtk_widget_get_window(widget) &&
                    gtk_widget_is_drawable(tab_label)) {
                    GtkAllocation allocation;
                    gtk_widget_get_allocation(tab_label, &allocation);
                    int lx = allocation.x;
                    int lw = allocation.width;
                    
                    // fix the labels position
                    allocation.y = y + ty;
                    allocation.height = lh;
                    gtk_widget_set_allocation(tab_label, &allocation);
                    
                    // draw tab background
                    cairo_rectangle(cr, lx - tx, y, lw + 2 * tx, bh);
                    get_base_color(widget, NULL, &r, &g, &b);
                    cairo_set_source_rgba(cr, r,g,b, page != cur_page ? alpha / 2 : alpha);
                    cairo_fill(cr);
                    
                    if (page == cur_page) {
                        // draw tab light
                        get_bg_color(widget, NULL, &r, &g, &b);
                        cairo_rectangle(cr, lx - tx + 2, y + 2, lw + 2 * tx - 4, 2);
                        cairo_set_source_rgb(cr, r, g, b);
                        cairo_fill(cr);
                        
                        cairo_rectangle(cr, lx - tx + 2, y + 1, lw + 2 * tx - 4, 1);
                        cairo_set_source_rgba(cr, 0,0,0,0.5);
                        cairo_fill(cr);
                        
                        cairo_rectangle(cr, lx - tx + 2, y + 4, lw + 2 * tx - 4, 1);
                        cairo_set_source_rgba(cr, 1,1,1,0.3);
                        cairo_fill(cr);
                    
                    }
                    // draw labels
                    gtk_container_propagate_draw
                        (GTK_CONTAINER (notebook),
                         tab_label,
                         cr);
                }
                cn++;
            }
            add = bh;
        }
        
        // draw main body
        get_base_color(widget, NULL, &r, &g, &b);
        cairo_rectangle(cr, x, y + add, sx, sy - add);
        cairo_set_source_rgba(cr, r, g, b, alpha);
        cairo_fill(cr);
        
        // draw frame
        cairo_rectangle(cr, x + 0.5, y + add + 0.5, sx - 1, sy - add - 1);
        pat = cairo_pattern_create_linear(x, y + add, x, y + sy - add);
        cairo_pattern_add_color_stop_rgba(pat,   0,   0,   0,   0, 0.3);
        cairo_pattern_add_color_stop_rgba(pat, 0.5, 0.5, 0.5, 0.5,   0);
        cairo_pattern_add_color_stop_rgba(pat,   1,   1,   1,   1, 0.2);
        cairo_set_source (cr, pat);
        cairo_set_line_width(cr, 1);
        cairo_stroke_preserve(cr);
                    
        int sw = gdk_pixbuf_get_width(nb->screw);
        int sh = gdk_pixbuf_get_height(nb->screw);
        
        // draw screws
        if (nb->screw) {
            gdk_cairo_set_source_pixbuf(cr, nb->screw, x, y + add);
            cairo_fill_preserve(cr);
            gdk_cairo_set_source_pixbuf(cr, nb->screw, x + sx - sw, y + add);
            cairo_fill_preserve(cr);
            gdk_cairo_set_source_pixbuf(cr, nb->screw, x, y + sy - sh);
            cairo_fill_preserve(cr);
            gdk_cairo_set_source_pixbuf(cr, nb->screw, x + sx - sh, y + sy - sh);
            cairo_fill_preserve(cr);
        }
        // propagate expose to all children
        // Beware that get_current_page returns 0 as an index for the
        // first page, which is falsy if used directly.
        if (gtk_notebook_get_nth_page(notebook, gtk_notebook_get_current_page(notebook)))
        {
            gtk_container_propagate_draw(
                GTK_CONTAINER(notebook),
                gtk_notebook_get_nth_page(
                    notebook,
                    gtk_notebook_get_current_page(notebook)),
                cr);
        }
        
        cairo_pattern_destroy(pat);
        // cairo_destroy(cr);
        
    }
    return FALSE;
}

void
calf_notebook_set_pixbuf (CalfNotebook *self, GdkPixbuf *image)
{
    self->screw = image;
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void
calf_notebook_class_init (CalfNotebookClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->draw = calf_notebook_draw;
    gtk_widget_class_install_style_property(
        widget_class, g_param_spec_float("background-alpha", "Alpha Background", "Alpha of background",
        0.0, 1.0, 0.5, GParamFlags(G_PARAM_READWRITE)));
}

static void
calf_notebook_init (CalfNotebook *self)
{
    //GtkWidget *widget = GTK_WIDGET(self);
}

GType
calf_notebook_get_type (void)
{
    static GType type = 0;
    if (!type) {
        static const GTypeInfo type_info = {
            sizeof(CalfNotebookClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc)calf_notebook_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof(CalfNotebook),
            0,    /* n_preallocs */
            (GInstanceInitFunc)calf_notebook_init
        };

        for (int i = 0; ; i++) {
            const char *name = "CalfNotebook";
            //char *name = g_strdup_printf("CalfNotebook%u%d", 
                //((unsigned int)(intptr_t)calf_notebook_class_init) >> 16, i);
            if (g_type_from_name(name)) {
                //free(name);
                continue;
            }
            type = g_type_register_static(GTK_TYPE_NOTEBOOK,
                                          name,
                                          &type_info,
                                          (GTypeFlags)0);
            //free(name);
            break;
        }
    }
    return type;
}
