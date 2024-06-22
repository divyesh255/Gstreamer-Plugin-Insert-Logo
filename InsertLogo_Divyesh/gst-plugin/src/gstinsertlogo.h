/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2020 Niels De Graef <niels.degraef@gmail.com>
 * Copyright (C) 2024 Divyesh Kapadiya <<Divyesh.Kapadiya@moschip.com>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_INSERTLOGO_H__
#define __GST_INSERTLOGO_H__

#include <gst/gst.h>

#include <gst/video/gstvideofilter.h>
#include <gst/video/video-overlay-composition.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <math.h>
#include <cairo.h>


G_BEGIN_DECLS



/**
 * G_DECLARE_FINAL_TYPE:
 * @TypeName: The name of the type to declare.
 * @type_name: The function name to get the GType of the type.
 * @TypeNamespace: The namespace for the type.
 * @type_namespace: The name of the type in the GType system.
 * @ParentType: The parent type of the new type.
 *
 * Macro to declare the type of a GStreamer element. This macro is used
 * to declare a new GStreamer element type and provides the necessary
 * declarations for the GType system to work with the new type.
 */
G_DECLARE_FINAL_TYPE (GstInsertLogo, gst_insert_logo,
    GST, INSERTLOGO, GstElement)

#define DFLT_ROTATE		"NoRotate"
#define DFLT_VAL			-1
#define DFLT_BOOL 		TRUE
#define DFLT_NOT_BOOL FALSE
#define DFLE_ROTATE		0
#define DFLT_SCROLL		"off"
#define DFLT_SPEED		"slow"

/**
 * GST_TYPE_INSERTLOGO:
 *
 * The type identifier for the `GstInsertLogo` type.
 */
#define GST_TYPE_INSERTLOGO (gst_insert_logo_get_type())


/**
 * GST_MY_ELEMENT:
 * @obj: a pointer to a #GstObject.
 *
 * Casts the object to a `GstInsertLogo`.
 */
#define GST_MY_ELEMENT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_INSERTLOGO, GstInsertLogo))


/**
 * GST_INSERTLOGO_CLASS:
 * @klass: a pointer to a #GstObjectClass.
 *
 * Casts the class object to a `GstInsertLogoClass`.
 */
#define GST_INSERTLOGO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_INSERTLOGO, GstInsertLogoClass))


/**
 * GST_IS_INSERTLOGO:
 * @obj: a pointer to a #GstObject.
 *
 * Checks if the object is an instance of `GstInsertLogo`.
 */
#define GST_IS_INSERTLOGO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_INSERTLOGO))


/**
 * GST_IS_INSERTLOGO_CLASS:
 * @klass: a pointer to a #GstObjectClass.
 *
 * Checks if the class object is an instance of `GstInsertLogoClass`.
 */  
#define GST_IS_INSERTLOGO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_INSERTLOGO))

/**
 * GstInsertLogo:
 * @element: The parent GstElement instance.
 * @sinkpad: The sink pad of the element.
 * @srcpad: The source pad of the element.
 * @silent: Whether the element is in silent mode.
 * @coordinate: An array of two integers representing the coordinates.
 * @rotation: The rotation state of the logo.
 * @speed: The speed state of the logo.
 * @scroll: The scroll state of the logo.
 * @strict: Whether the element is in strict mode.
 * @scrlEnable: Whether scrolling is enabled.
 * @rotateEnable: Whether rotation is enabled.
 * @alpha: The alpha value of the logo.
 * @logo: The path to the logo file.
 * @frame_width: The width of the frame.
 * @frame_height: The height of the frame.
 * @degree: The degree of rotation.
 * @dflt: Whether the element is in default mode.
 * @overlay_width: The width of the overlay.
 * @overlay_height: The height of the overlay.
 * @scrl_dflt_cord: Whether scrolling default coordinates are used.
 * @dfltLogo: Whether the default logo is used.
 * @cord_negative: Whether the coordinates are negative.
 * @dflt_rotate: Whether default rotation is used.
 * @dflt_speed: Whether default speed is used.
 * @dflt_scrl: Whether default scrolling is used.
 * @dflt_alpha: Whether default alpha value is used.
 * @check_Property_validation: Whether property validation is checked.
 */
struct _GstInsertLogo
{
  GstElement element;
  GstPad *sinkpad, *srcpad;
  gboolean silent;
  gint coordinate[2];
  gchar *rotation;
  gchar *speed;
  gchar *scroll;
  gboolean strict;
  gboolean scrlEnable;
  gboolean rotateEnable;
  gint alpha;
  gchar *logo;
  gint frame_width;
  gint frame_height;
  gdouble degree;
  gboolean dflt;
  gint overlay_width;
  gint overlay_height;
  gboolean scrl_dflt_cord;
  gboolean dfltLogo;
  gboolean cord_negative;
  gboolean dflt_rotate;
  gboolean dflt_speed;
  gboolean dflt_scrl;
  gboolean dflt_alpha;
  gboolean check_Property_validation;
  gboolean adjust_y_cord;
};



G_END_DECLS

#endif /* __GST_INSERTLOGO_H__ */
