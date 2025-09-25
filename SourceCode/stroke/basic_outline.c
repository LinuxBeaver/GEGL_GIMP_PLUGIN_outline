/* This file is an image processing operation for GEGL
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * 2006 Pippin
 * 2025 Beaver GEGL Stroke
* 
* Recreate plugin using this GEGL syntax
* 
* id=1 dst-over aux=[ ref=1
median-blur radius=10 alpha-percentile=100 abyss-policy=none
color-overlay value=white
opacity value=1
dst-out aux=[ ref=1 median-blur radius=-4 abyss-policy=none alpha-percentile=100  ]
]


 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_double (puff, _("Blur radius"), 0.0)
   description (_("A very mild blur to puff the outline"))
   value_range (0.0, 3.0)
   ui_range    (0, 3.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

enum_start (gegl_outlinebasic_grow_shape)
  enum_value (GEGL_OUTLINEBASIC_GROW_SHAPE_SQUARE,  "square",  N_("Square"))
  enum_value (GEGL_OUTLINEBASIC_GROW_SHAPE_CIRCLE,  "circle",  N_("Circle"))
  enum_value (GEGL_OUTLINEBASIC_GROW_SHAPE_DIAMOND, "diamond", N_("Diamond"))
enum_end (GegloutlinebasicGrowShape)

property_enum   (shape, _("Grow shape"),
                 GegloutlinebasicGrowShape, gegl_outlinebasic_grow_shape,
                 GEGL_OUTLINEBASIC_GROW_SHAPE_CIRCLE)
  description   (_("The internal median shape of the outline"))

property_int (grow, _("Grow radius"), 12)
   description (_("The size of the outline"))
   value_range (0, 200)
   ui_range    (0, 75)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_color  (color, _("Color"), "black")
  description   (_("The outline's color (defaults to 'black')"))

property_double (opacity, _("Opacity"), 1)
  description   (_("Opacity of the outline"))
  value_range   (0.0, 1.0)
  ui_steps      (0.01, 0.10)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     basic_outline
#define GEGL_OP_C_SOURCE basic_outline.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");

  GeglNode *median  = gegl_node_new_child (gegl,
                                          "operation", "gegl:median-blur",
                                          "abyss_policy", 0,
                                          "radius", 12,
                                          "alpha-percentile", 100.0,
                                          NULL);
  GeglNode *color  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color-overlay",
                                          NULL);
  GeglNode *behind  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-over",
                                          NULL);
  GeglNode *gaus  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gaussian-blur", "std-dev-x", 0.0, "std-dev-y", 0.0, "clip-extent", 0, "abyss-policy", 0, 
                                          NULL);
  GeglNode *opacity  = gegl_node_new_child (gegl,
                                          "operation", "gegl:opacity",
                                          NULL);
  GeglNode *erase  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-out",
                                          NULL);
  GeglNode *median2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:median-blur",
                                          "abyss_policy", 0,
                                          "radius", -3,
                                          "alpha-percentile", 100.0,
                                          NULL);
  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

  gegl_node_link_many (input, behind, output, NULL);
  gegl_node_link_many (input, median, gaus, color, erase, opacity, NULL);
  gegl_node_connect (behind, "aux", opacity, "output");
  gegl_node_link_many (input, median2, NULL);
  gegl_node_connect (erase, "aux", median2, "output");
            
  gegl_operation_meta_redirect (operation, "grow", median, "radius");
  gegl_operation_meta_redirect (operation, "shape", median, "neighborhood");
  gegl_operation_meta_redirect (operation, "puff", gaus, "std-dev-x");
  gegl_operation_meta_redirect (operation, "puff", gaus, "std-dev-y");
  gegl_operation_meta_redirect (operation, "opacity", opacity, "value");
  gegl_operation_meta_redirect (operation, "color", color, "value");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:outline",
    "title",          _("Stroke"),
    "reference-hash", "goatsavedtheworld",
    "description",
    _("Creates a outline border for transparent images"),
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Stroke..."),
                                 NULL);
}

#endif

