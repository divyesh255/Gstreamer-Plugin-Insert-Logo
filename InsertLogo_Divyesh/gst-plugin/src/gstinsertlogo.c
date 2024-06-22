/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
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

/**
 * SECTION:element-insertlogo
 *
 * The insertlogo element adds a logo overlay to a video stream.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! insertlogo ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

/**
 * API calling sequence
 * 1. insertlogo_init()
 * 2. _class_init()
 * 3. _init()
 * 4. _set_property()
 * 5. _sink_event()
 * 6. _chain()
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include "gstinsertlogo.h"
#include <unistd.h>  
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>  
#include <gst/video/gstvideometa.h>
#include <gst/video/video.h>
#include <stdlib.h>
#include <string.h>

GST_DEBUG_CATEGORY_STATIC (gst_insert_logo_debug);
#define GST_CAT_DEFAULT gst_insert_logo_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_COORDINATE,
  PROP_ROTATION,
  PROP_SPEED,
  PROP_SCROLL,
  PROP_STRICT_MODE,
  PROP_ALPHA,
  PROP_LOGO,
  N_PROPERTIES
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw, format=NV12")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_insert_logo_parent_class parent_class
G_DEFINE_TYPE (GstInsertLogo, gst_insert_logo, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (insert_logo, "insertlogo", GST_RANK_NONE,
    GST_TYPE_INSERTLOGO);

/* Property setter and getter functions */
static void gst_insert_logo_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_insert_logo_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

/* Sink pad event handler */
static gboolean gst_insert_logo_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);

/* Chain function for processing buffers */
static GstFlowReturn gst_insert_logo_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* Functions for imposing, scrolling, and rotating the logo */
static void gst_insert_logo_impose_logo(GstInsertLogo *filter, guint8 *y_pixels, 
		guint8 *uv_pixels, guint y_stride, guint uv_stride);
static void gst_insert_logo_scroll_logo(GstInsertLogo *filter, guint8 *y_pixels, 
		guint8 *uv_pixels, guint y_stride, guint uv_stride);
static void gst_insert_logo_rotate_logo(GstInsertLogo *filter, guint8 *y_pixels, 
		guint8 *uv_pixels, guint y_stride, guint uv_stride);

/* Helper functions */
static void gst_insert_logo_set_logo(GstInsertLogo *filter);
static void gst_insert_logo_check_property_validation(GstInsertLogo *filter);

/**
 * gst_insert_logo_class_init:
 * @klass: a #GstInsertLogoClass instance
 *
 * Initialize the insertlogo's class.
 */
static void
gst_insert_logo_class_init (GstInsertLogoClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_insert_logo_set_property;
  gobject_class->get_property = gst_insert_logo_get_property;

  /* Install properties */
  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));
  
  g_object_class_install_property(gobject_class, PROP_COORDINATE,
    gst_param_spec_array("coordinate", "Coordinate", "X And Y coordinate value. Dafault: Top right corner of the given resolution.",
        g_param_spec_int("element", "Element", "cordinates ('<x,y>')", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS),
        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_ROTATION,
    g_param_spec_string ("rotation", "Rotation",
              "Rotate the logo. (no-rotate, clockwise, counter-clockwise) Disabled when scrolling is enabled.",
              "no-rotate", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
              
  g_object_class_install_property (gobject_class, PROP_SPEED,
    g_param_spec_string ("speed", "Speed",
              "Animation Speed. (slow, medium, fast).",
              "slow", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_property (gobject_class, PROP_SCROLL,
    g_param_spec_string ("scrolling", "Scroll",
              "Scroll side (LeftToRight (ltr), RightToLeft (rtl), off)",
              "off", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_property (gobject_class, PROP_STRICT_MODE,
      g_param_spec_boolean ("strict-mode", "Strict", "Treat every warning as an error and exit the plugin when set to 'TRUE' else give warning and continue with 'Default' value.",
          FALSE, G_PARAM_READWRITE));
  
  g_object_class_install_property(gobject_class,PROP_ALPHA,
      g_param_spec_int ("alpha","Alpha","Set alpha (opacity) blending on plugin. Range is 0 to 100", G_MININT, G_MAXINT, 
      		-1, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_property (gobject_class, PROP_LOGO,
    g_param_spec_string ("logo-file", "Logo",
              "Path of logo file. If not provided, plugin will take default logo (Moschip logo). \n\t\t\tLogo should be present in current directory with name 'moschip.png'.",
              NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  
  /* Set element details */
  gst_element_class_set_details_simple (gstelement_class,
      "InsertLogo",
      "Filter/Effect",
      "This Plugin can impose logo on live video streamInsertLogo GStreamer Plugin - Adds a logo overlay to video frames.", "Divyesh Kapadiya <<Divyesh.Kapadiya@moschip.com>>");

  /* Add pad templates */
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}


/**
 * gst_insert_logo_init:
 * @filter: a #GstInsertLogo instance
 *
 * Initialize the new element, instantiate pads and add them to element,
 * set pad callback functions, and initialize instance structure.
 */
static void
gst_insert_logo_init (GstInsertLogo * filter)
{
  /* Create sink pad */
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_insert_logo_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_insert_logo_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  /* Create src pad */
  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  /* Initialize properties */
  filter->silent = DFLT_NOT_BOOL;
  filter->coordinate[0] = DFLT_VAL;
  filter->coordinate[1] = DFLT_VAL;
  filter->rotation = DFLT_ROTATE;
  filter->speed = DFLT_SPEED;
  filter->scroll = DFLT_SCROLL;
  filter->logo = NULL;
  filter->strict = DFLT_NOT_BOOL;
  filter->degree = DFLE_ROTATE;
  filter->alpha = DFLT_VAL;
  filter->dflt = DFLT_BOOL;
  filter->scrlEnable = DFLT_NOT_BOOL;
  filter->scrl_dflt_cord = DFLT_NOT_BOOL;
  filter->dfltLogo = DFLT_BOOL;
  filter->cord_negative = DFLT_NOT_BOOL;
  filter->dflt_rotate = DFLT_BOOL;
  filter->dflt_speed = DFLT_BOOL;
  filter->dflt_scrl = DFLT_BOOL;
  filter->dflt_alpha = DFLT_BOOL;
  filter->rotateEnable = DFLT_NOT_BOOL;
  filter->check_Property_validation = DFLT_NOT_BOOL;
  filter->adjust_y_cord = DFLT_NOT_BOOL;
}


/**
 * @brief Sets a property on the InsertLogo element.
 *
 * This function is called when a property of the InsertLogo element is set.
 * It handles different properties like 'silent', 'coordinate', 'rotation', etc.
 *
 * @param object The GObject representing the InsertLogo element.
 * @param prop_id The ID of the property to set.
 * @param value The new value of the property.
 * @param pspec The GParamSpec for the property.
 */
static void
gst_insert_logo_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstInsertLogo *filter = GST_INSERTLOGO (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_COORDINATE: {
        const GValue *v;
        gint i, temp;

        // Check if the array size is correct
        if (gst_value_array_get_size (value) != 2)
          goto wrong_format;

        // Iterate through the array to get the coordinates
        for (i = 0; i < 2; i++) {
          v = gst_value_array_get_value (value, i);

          // Check if the value is an integer
          if (!G_VALUE_HOLDS_INT (v))
            goto wrong_format;

          // Get the integer value
          temp = g_value_get_int (v);
          
          // Check if the value is non-negative
          if (temp >= 0){
            filter->coordinate[i] = temp;
          } else{
            // If negative, store the absolute value and set a flag
            filter->coordinate[i] = temp;
            filter->cord_negative = TRUE;
          }
        }
        filter->dflt = FALSE;

      wrong_format:
        break;
      }
    case PROP_ROTATION:
      {
        // Set the rotation property
        filter->rotation = g_strdup (g_value_get_string (value));
        filter->dflt_rotate = FALSE;
        if (strcmp (filter->rotation, "no-rotate") == 0) {
          filter->rotateEnable = FALSE;
        } else {
        	filter->rotateEnable = TRUE;
        }
        g_print ("Rotation set to : %s\n", filter->rotation);
      }
      break;
    case PROP_SPEED:
      {
        // Set the speed property
        filter->speed = g_strdup (g_value_get_string (value));
        g_print ("Speed set to : %s\n", filter->speed);
        filter->dflt_speed = FALSE;
      }
      break;
    case PROP_SCROLL:
      {
        // Set the scroll property
        filter->scroll = g_strdup (g_value_get_string (value));
        filter->dflt_scrl = FALSE;

        // Enable or disable scrolling based on the value
        if (strcmp (filter->scroll, "off") == 0) {
          filter->scrlEnable = FALSE;
        } else if (strcmp (filter->scroll, "ltr") == 0 ||
                   strcmp (filter->scroll, "rtl") == 0) {
          filter->scrlEnable = TRUE;
          g_print ("scrlEnable");
        }
        g_print ("Scroll set to : %s\n", filter->scroll);
      }
      break;
    case PROP_STRICT_MODE:
      // Set the strict mode property
      filter->strict = g_value_get_boolean (value);
      break;
    case PROP_ALPHA:
      // Set the alpha property
      filter->alpha = g_value_get_int (value);
      g_print ("Alpha set to : %d", filter->alpha);
      filter->dflt_alpha = FALSE;
      break;
    case PROP_LOGO:
      {
        // Set the logo property
        g_free (filter->logo);
        filter->logo = g_strdup (g_value_get_string (value));
        filter->dfltLogo = FALSE;
        g_print ("Logo argument was changed to %s\n", filter->logo);
        break;
      }
    default:
      // Warn if an invalid property ID is provided
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


/**
 * @brief Gets a property from the InsertLogo element.
 *
 * This function is called when a property of the InsertLogo element is requested.
 * It retrieves the value of the requested property and sets it in the provided GValue.
 *
 * @param object The GObject representing the InsertLogo element.
 * @param prop_id The ID of the property to get.
 * @param value The GValue where the property value should be stored.
 * @param pspec The GParamSpec for the property.
 */
static void
gst_insert_logo_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstInsertLogo *filter = GST_INSERTLOGO (object);

  switch (prop_id) {
    case PROP_SILENT:
      // Get the silent property value
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_COORDINATE:
      {
        // Get the coordinate property values
        gint i;
        for (i = 0; i < 2; i++) {
          GValue v = G_VALUE_INIT;
          g_value_init (&v, G_TYPE_INT);
          g_value_set_int (&v, filter->coordinate[i]);
          gst_value_array_append_and_take_value (value, &v);
          g_value_unset (&v);
        }
        
        // Print the default coordinates
        g_print ("Default: <%d,%d>\n", g_value_get_int (gst_value_array_get_value (value, 0)), 
                 g_value_get_int (gst_value_array_get_value (value, 1)));
        
        break;
      }
    case PROP_ROTATION:
      // Get the rotation property value
      g_value_set_string (value, filter->rotation);
      break;
    case PROP_SPEED:
      // Get the speed property value
      g_value_set_string (value, filter->speed);
      break;
    case PROP_SCROLL:
      // Get the scroll property value
      g_value_set_string (value, filter->scroll);
      break;
    case PROP_STRICT_MODE:
      // Get the strict mode property value
      g_value_set_boolean (value, filter->strict);
      break;
    case PROP_ALPHA:
      // Get the alpha property value
      g_value_set_int (value, filter->alpha);
      break;
    case PROP_LOGO:
      // Get the logo property value
      g_value_set_string (value, filter->logo);
      break;
    default:
      // Warn if an invalid property ID is provided
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


/**
 * @brief Chain function for the InsertLogo element.
 *
 * This function is called when a new buffer is available on the sink pad.
 * It processes the buffer by imposing the logo, scrolling if enabled, and rotating if specified.
 *
 * @param pad The sink pad that received the buffer.
 * @param parent The parent object of the pad.
 * @param buf The buffer to process.
 * @return The result of the processing operation.
 */
static GstFlowReturn
gst_insert_logo_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
		GstInsertLogo *filter = GST_INSERTLOGO (parent);
		GstFlowReturn ret;
  	GstVideoInfo video_info;
		GstVideoFrame video_frame;		
		
		// Check and validate the filter properties
		if(!filter->check_Property_validation){
			gst_insert_logo_check_property_validation(filter);
			filter->check_Property_validation = TRUE;
		}
		
    // Retrieve video information from the buffer's caps
  	if(!(gst_video_info_from_caps(&video_info, gst_pad_get_current_caps(pad))))
		{
			g_print("failed to retrieve information from caps\n");
			exit(0);
		}
		
		// Map the video frame for writing
		if(!(gst_video_frame_map(&video_frame, &video_info, buf, GST_MAP_WRITE)))
		{
			g_print("failed to map the video frame\n");
			exit(0);
		}
		
		// Extract pixel data and stride information
		guint8 *y_pixels = GST_VIDEO_FRAME_PLANE_DATA(&video_frame, 0); 
		guint8 *uv_pixels = GST_VIDEO_FRAME_PLANE_DATA(&video_frame, 1); 
		guint y_stride = GST_VIDEO_FRAME_PLANE_STRIDE(&video_frame, 0); 
		guint uv_stride = GST_VIDEO_FRAME_PLANE_STRIDE(&video_frame, 1);
		
		// Impose the logo on the frame
		if(!filter->rotateEnable && !filter->scrlEnable){
			gst_insert_logo_impose_logo(filter, y_pixels, uv_pixels, y_stride, uv_stride);
		}
		
		// Scroll the logo if scrolling is enabled
		if(filter->scrlEnable){
			gst_insert_logo_scroll_logo(filter, y_pixels, uv_pixels, y_stride, uv_stride);
		}
		
		// Rotate the logo if rotation is enabled		
		if(filter->rotateEnable && !filter->scrlEnable){
			gst_insert_logo_rotate_logo(filter, y_pixels, uv_pixels, y_stride, uv_stride);
		}  
		
		// Push the processed buffer to the src pad
		ret = gst_pad_push(filter->srcpad, buf);
		gst_video_frame_unmap(&video_frame);
		
		return ret;
}


/**
 * @brief Handles sink events for the InsertLogo element.
 *
 * This function is called when an event is received on the sink pad.
 * It handles CAPS events to extract frame width and height information.
 *
 * @param pad The sink pad that received the event.
 * @param parent The parent object of the pad.
 * @param event The event to handle.
 * @return TRUE if the event was handled successfully, FALSE otherwise.
 */
static gboolean
gst_insert_logo_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstInsertLogo *filter;
  gboolean ret;

  filter = GST_INSERTLOGO (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;
      gchar *caps_str;
      gst_event_parse_caps (event, &caps);
      caps_str = gst_caps_to_string (caps);
      g_print ("Received caps in sink: %s\n", caps_str);

      // Get frame width and height from caps
      GstStructure *structure = gst_caps_get_structure (caps, 0); // Assuming only one structure in caps
      if (structure) {
        gint frame_width, frame_height;
        if (gst_structure_get_int (structure, "width", &frame_width) &&
            gst_structure_get_int (structure, "height", &frame_height)) {
          g_print ("Frame Width: %d, Frame Height: %d\n", frame_width, frame_height);
          filter->frame_width = frame_width;
          filter->frame_height = frame_height;
        }
      }
      g_free (caps_str);

      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}


/**
 * @brief Check and validate the properties of the InsertLogo element.
 *
 * This function checks and validates the logo file, rotation, speed, scroll, alpha,
 * and their default values. It sets default values if necessary and handles warnings
 * or errors based on the strict mode.
 *
 * @param filter The InsertLogo element instance.
 */
static void
gst_insert_logo_check_property_validation (GstInsertLogo *filter)
{
  if (!filter->dfltLogo) {
    // Check if the logo file exists and has the correct extension
    if (access (filter->logo, F_OK) == -1) {
      if (filter->strict) {        
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("File does not exist."),
		    ("Only files with the extension .png are valid."));
		    exit(1);
      } else {
        g_warning ("File '%s' does not exist. Only files with the extension .png are valid.", filter->logo);
        filter->dfltLogo = TRUE;
        goto skip;
      }
    }

    const gchar *file_ext = strrchr (filter->logo, '.');
    if (file_ext == NULL || strcmp (file_ext, ".png") != 0) {
      if (filter->strict) {
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid logo file format."),
		    ("Only PNG files are supported."));
		    exit(1);
      } else {
        g_warning ("Invalid logo file format '%s'. Only files with the extension .png are valid.", file_ext);
        filter->dfltLogo = TRUE;
      }
    }
  skip:
  }
  // Set the default logo if necessary
    if (filter->logo == NULL || filter->dfltLogo) {
      gst_insert_logo_set_logo (filter);
    }

  // Check and validate the rotation property
  if (!filter->dflt_rotate) {
    if (strcmp (filter->rotation, "no-rotate") != 0 &&
        strcmp (filter->rotation, "clockwise") != 0 &&
        strcmp (filter->rotation, "counter-clockwise") != 0) {
      if (filter->strict) {
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid Rotation Property."),
		    ("Valid values are 'no-rotate', 'clockwise', or 'counter-clockwise'."));
		    exit(1);
      } else {
        g_warning ("Invalid value '%s' for rotation property. Valid values are 'no-rotate', 'clockwise', or 'counter-clockwise'.", filter->rotation);
        filter->rotation = "no-rotate";
        g_print ("Default value set to : %s\n", filter->rotation);
        filter->dflt_rotate = TRUE;
      }
    }
  }

  // Check and validate the speed property
  if (!filter->dflt_speed) {
    if (strcmp (filter->speed, "slow") != 0 &&
        strcmp (filter->speed, "medium") != 0 &&
        strcmp (filter->speed, "fast") != 0) {
      if (filter->strict) {
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid speed Property."),
		    ("Valid values are 'slow', 'medium', or 'fast'."));
		    exit(1);
      } else {
        g_warning ("Invalid value '%s' for speed property. Valid values are 'slow', 'medium', or 'fast'.", filter->speed);
        filter->speed = "slow";
        g_print ("Default value set to : %s\n", filter->speed);
        filter->dflt_speed = TRUE;
      }
    }
  }

  // Check and validate the scroll property
  if (!filter->dflt_scrl) {
    if (strcmp (filter->scroll, "off") != 0 &&
        strcmp (filter->scroll, "ltr") != 0 &&
        strcmp (filter->scroll, "rtl") != 0) {
      if (filter->strict) {
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid scroll Property."),
		    ("Valid values are 'off', 'ltr', or 'rtl'."));
		    exit(1);
      } else {
        g_warning ("Invalid value '%s' for scroll property. Valid values are 'off', 'ltr', or 'rtl'.", filter->scroll);
        filter->scroll = "off";
        g_print ("Default value set to : %s\n", filter->scroll);
        filter->dflt_scrl = TRUE;
      }
    }
  }

  // Check and validate the alpha property
  if (!filter->dflt_alpha) {
    if (!(filter->alpha >= -1 && filter->alpha <= 100)) {
      if (filter->strict) {
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid alpha Property."),
		    ("Valid values are '-1 (default)' or '0 to 100 (integer)'."));
		    exit(1);
      } else {
        g_warning ("Invalid value '%d' for alpha property. Valid values are '-1 (default)' or '0 to 100 (integer)'.", filter->alpha);
        filter->alpha = -1;
        g_print ("Default value set to : %d\n", filter->alpha);
        filter->dflt_alpha = TRUE;
      }
    }
  }

  // Check if both rotation and scroll are enabled
  if (filter->rotateEnable && filter->scrlEnable) {
    if (filter->strict) {
      GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Rotation and Scroll Both are Enabled."),
		  ("At a particular time, either Rotation or Scroll will work."));
		  exit(1);
    } else {
      g_warning ("Rotation and Scroll Both are Enabled. At a particular time, either Rotation or Scroll will work.");
      filter->rotateEnable = FALSE;
      g_print ("Set default animation scroll to : %s\n", filter->scroll);
    }
  }
}


/**
 * @brief Set the logo file path for the InsertLogo element.
 *
 * This function sets the logo file path for the InsertLogo element
 * based on the current working directory and the default logo file name.
 * It allocates memory for the new path and updates the element's logo
 * file path property.
 *
 * @param filter The InsertLogo element instance.
 */
static void
gst_insert_logo_set_logo(GstInsertLogo *filter)
{
    FILE *fp;
    char path[1035];

    // Open the command for reading
    fp = popen("pwd", "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        return;
    }

    // Read the output a line at a time
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        printf("Current working directory: %s", path);
    }

    // Close the pipe
    pclose(fp);

    // Remove the newline character at the end of the path
    path[strcspn(path, "\n")] = '\0';

    // Calculate the length of the new path
    size_t path_len = strlen(path);
    size_t new_path_len = path_len + strlen("/moschip.png") + 1;

    // Allocate memory for the new path
    char *new_path = malloc(new_path_len);
    if (new_path == NULL) {
        printf("Failed to allocate memory\n");
        return;
    }

    // Create the new path
    snprintf(new_path, new_path_len, "%s/moschip.png", path);
    g_print("Logo : %s\n", new_path);

    // Set the new path as the logo file path
    filter->logo = new_path;
    filter->dfltLogo = FALSE;
}


/**
 * @brief Imposes the logo onto the YUV frame at the specified coordinates with rotation and alpha blending.
 *
 * This function loads the logo image from the file specified in the filter and imposes it onto the YUV frame
 * at the specified coordinates with alpha blending.
 *
 * @param filter The GstInsertLogo filter instance.
 * @param y_pixels The Y plane pixels of the YUV frame.
 * @param uv_pixels The UV plane pixels of the YUV frame.
 * @param y_stride The stride of the Y plane.
 * @param uv_stride The stride of the UV plane.
 */
static void
gst_insert_logo_impose_logo(GstInsertLogo *filter, guint8 *y_pixels, guint8 *uv_pixels, guint y_stride, guint uv_stride)
{
    cairo_surface_t *image_surface = cairo_image_surface_create_from_png(filter->logo);
    if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS) {
        g_printerr("Error loading logo image: %s\n", cairo_status_to_string(cairo_surface_status(image_surface)));
        return;
    }

    guint width_logo = cairo_image_surface_get_width(image_surface);
    guint height_logo = cairo_image_surface_get_height(image_surface);
    
    // If the coordinate values are negative
    if(filter->cord_negative){
		  
		  if(filter->strict){
		      // If strict mode is enabled, raise an error and abort
			    cairo_surface_destroy(image_surface);
			    GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid value (negative) for coordinate property."),
		      ("Valid values are positive integer numbers."));
		      exit(1);
		  } else {
		      // If not in strict mode, issue a warning
		      g_warning ("Invalid value (negative) '<%d, %d>' for coordinate property. Valid values are positive integer numbers.",
		                      filter->coordinate[0], filter->coordinate[1]);
		  }
		  
		  // Set default coordinates (top right corner)
		  filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;
		  
		  // Ensure the second coordinate is at least 1
		  if(filter->coordinate[1] == 0){
		      filter->coordinate[1] = 1;
		  }
		  
		  // Print the default coordinates
		  g_print("Default (Top Right) value set '<%d, %d>'\n", filter->coordinate[0], filter->coordinate[1]);
		  // Reset the flag
		  filter->cord_negative = FALSE;
		}
		
		// If the default flag is not set and the coordinates are out of bounds
		if(!filter->dflt && (filter->coordinate[0] > filter->frame_width || filter->coordinate[1] > filter->frame_height)){
				if(filter->strict){
				    // If strict mode is enabled, raise an error and abort
				    cairo_surface_destroy(image_surface);
				    GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid coordinates for the coordinate property."),
				    ("Valid values are within the frame dimensions."));
				    exit(1);
				} else {
				    // If not in strict mode, issue a warning
				    g_warning ("Coordinates out of bounds '<%d, %d>' for coordinate property. Valid values are within the frame dimensions.",
				                    filter->coordinate[0], filter->coordinate[1]);
				}
				filter->dflt = TRUE;
		}
		
		// setting default coordinates
		if(filter->dflt){
			filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;		
		}
    
    guint8 *data = cairo_image_surface_get_data(image_surface);
    
    //setting alpha to 100
    if(filter->alpha == -1){
    	filter->alpha = 100;
    }
    gdouble Alpha_Factor = (filter->alpha / 100.0);
    
    gint j, i;
		gint wrapped_i, wrapped_j;
		guint8 *y_pixel, *uv_pixel;
		guint8 Red, Green, Blue, Alpha;
		guint8 Y, U, V;

    if((width_logo < (filter->frame_width/6)) && (height_logo < (filter->frame_height/6)))
    {
		  for (j = 0; j < height_logo; j++)
			{
					for (i = 0; i < width_logo; i++)
					{
						  // Calculate the wrapped-around i and j coordinates
						  wrapped_i = (i + filter->coordinate[0]) % filter->frame_width;
						  wrapped_j = (j + filter->coordinate[1]) % filter->frame_height;

						  // calculate y and uv pixels address
						  y_pixel = y_pixels + wrapped_j * y_stride + wrapped_i;
						  uv_pixel = uv_pixels + (wrapped_j / 2) * uv_stride + (wrapped_i / 2) * 2;

						  // Reading RGBA values from the image file
						  Red = data[(j * width_logo + i) * 4];
						  Green = data[(j * width_logo + i) * 4 + 1];
						  Blue = data[(j * width_logo + i) * 4 + 2];
						  Alpha = data[(j * width_logo + i) * 4 + 3] * Alpha_Factor;

						  // convert RGB to YUV
						  Y = (0.257 * Red) + (0.504 * Green) + (0.098 * Blue) + 16;  // Y component
						  U = (0.439 * Red) - (0.368 * Green) - (0.071 * Blue) + 128;  // U component
						  V = -(0.148 * Red) - (0.291 * Green) + (0.439 * Blue) + 128;  // V component

						  // Alpha blending
						  Y = ((255 - Alpha) * (*y_pixel) + Alpha * Y) / 255;
						  U = ((255 - Alpha) * (uv_pixel[0]) + Alpha * U) / 255;
						  V = ((255 - Alpha) * (uv_pixel[1]) + Alpha * V) / 255;

						  *y_pixel = Y;
						  uv_pixel[0] = U;
						  uv_pixel[1] = V;
					}
			}
			cairo_surface_destroy(image_surface);
		} else {
        // Error if the logo dimensions are too large
        cairo_surface_destroy(image_surface);
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Logo dimensions are too large."),
				("The logo size should be less than one-sixth of the frame size."));
				exit(1);
    }
}

/**
 * @brief Imposes the logo onto the YUV frame and scroll it from the specified coordinates with alpha blending.
 *
 * This function loads the logo image from the file specified in the filter and imposes it onto the YUV frame
 * at the specified coordinates with alpha blending and scroll it.
 *
 * @param filter The GstInsertLogo filter instance.
 * @param y_pixels The Y plane pixels of the YUV frame.
 * @param uv_pixels The UV plane pixels of the YUV frame.
 * @param y_stride The stride of the Y plane.
 * @param uv_stride The stride of the UV plane.
 */
static void
gst_insert_logo_scroll_logo(GstInsertLogo *filter, guint8 *y_pixels, guint8 *uv_pixels, guint y_stride, guint uv_stride)
{
    cairo_surface_t *image_surface = cairo_image_surface_create_from_png(filter->logo);
    if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS) {
        g_printerr("Error loading logo image: %s\n", cairo_status_to_string(cairo_surface_status(image_surface)));
        return;
    }

    guint width_logo = cairo_image_surface_get_width(image_surface);
    guint height_logo = cairo_image_surface_get_height(image_surface);
    
    // If the coordinate values are negative
    if(filter->cord_negative){
		  
		  if(filter->strict){
		      // If strict mode is enabled, raise an error and abort
			    cairo_surface_destroy(image_surface);
			    GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid value (negative) for coordinate property."),
		      ("Valid values are positive integer numbers."));
		      exit(1);
		  } else {
		      // If not in strict mode, issue a warning
		      g_warning ("Invalid value (negative) '<%d, %d>' for coordinate property. Valid values are positive integer numbers.",
		                      filter->coordinate[0], filter->coordinate[1]);
		  }
		  
		  // Set default coordinates (top right corner)
		  filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;
		  
		  // Ensure the second coordinate is at least 1
		  if(filter->coordinate[1] == 0){
		      filter->coordinate[1] = 1;
		  }
		  
		  // Print the default coordinates
		  g_print("Default (Top Right) value set '<%d, %d>'\n", filter->coordinate[0], filter->coordinate[1]);
		  // Reset the flag
		  filter->cord_negative = FALSE;
		}
		
		// If the default flag is not set and the coordinates are out of bounds
		if(!filter->dflt && (filter->coordinate[0] > filter->frame_width || filter->coordinate[1] > filter->frame_height)){
				if(filter->strict){
				    // If strict mode is enabled, raise an error and abort
				    cairo_surface_destroy(image_surface);
				    GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid coordinates for the coordinate property."),
				    ("Valid values are within the frame dimensions."));
				    exit(1);
				} else {
				    // If not in strict mode, issue a warning
				    g_warning ("Coordinates out of bounds '<%d, %d>' for coordinate property. Valid values are within the frame dimensions.",
				                    filter->coordinate[0], filter->coordinate[1]);
				}
				filter->dflt = TRUE;
				filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  	filter->coordinate[1] = height_logo/30;
		}
		
		// Check if the default coordinates are set and scrolling default coordinates are not set
		if(filter->dflt && !filter->scrl_dflt_cord){
			// Calculate the X and Y coordinate for the default position
			filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;
		  
		  // Set the scrolling default coordinates flag to TRUE to indicate that the default coordinates are set
		  filter->scrl_dflt_cord = TRUE;		
		}
    
    guint8 *data = cairo_image_surface_get_data(image_surface);
    
    //setting alpha to 100
    if(filter->alpha == -1){
    	filter->alpha = 100;
    }
    gdouble Alpha_Factor = (filter->alpha / 100.0);
    
    if (strcmp (filter->scroll, "ltr") == 0 ) {
			// Scroll from left to right
			if(strcmp (filter->speed, "slow") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] + 2;
		 		if(filter->coordinate[0] >= filter->frame_width + filter->overlay_width){
   				filter->coordinate[0] = 0 - filter->overlay_width;
   			}   		
		 	} else if(strcmp (filter->speed, "medium") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] + 3;
		 		if(filter->coordinate[0] >= filter->frame_width + filter->overlay_width){
   				filter->coordinate[0] = 0 - filter->overlay_width;
   			}
		 	} else if(strcmp (filter->speed, "fast") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] + 4;
		 		if(filter->coordinate[0] >= filter->frame_width + filter->overlay_width){
   				filter->coordinate[0] = 0 - filter->overlay_width;
   			}   		
		 	} 
  	} else if(strcmp (filter->scroll, "rtl") == 0 ){
			// Scroll from right to left
			if(strcmp (filter->speed, "slow") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] - 2;
		 		if(filter->coordinate[0] <= (0 - filter->overlay_width)){
   				filter->coordinate[0] = filter->frame_width + filter->overlay_width;
   			}
		 	} else if(strcmp (filter->speed, "medium") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] - 3;
		 		if(filter->coordinate[0] <= (0 - filter->overlay_width)){
   				filter->coordinate[0] = filter->frame_width + filter->overlay_width;
   			}
		 	} else if(strcmp (filter->speed, "fast") == 0){
		 		filter->coordinate[0] = filter->coordinate[0] - 4;
		 		if(filter->coordinate[0] <= (0 - filter->overlay_width)){
   				filter->coordinate[0] = filter->frame_width + filter->overlay_width;
   			}
		 	} 
  	}
    
    gint j, i;
		gint wrapped_i, wrapped_j;
		guint8 *y_pixel, *uv_pixel;
		guint8 Red, Green, Blue, Alpha;
		guint8 Y, U, V;
		    
		if((width_logo < (filter->frame_width/6)) && (height_logo < (filter->frame_height/6)))
    {
		  for (j = 0; j < height_logo; j++)
			{
					for (i = 0; i < width_logo; i++)
					{
						  // Calculate the wrapped-around i and j coordinates
						  wrapped_i = (i + filter->coordinate[0]) % filter->frame_width;
						  wrapped_j = (j + filter->coordinate[1]) % filter->frame_height;

						  // Get pointers to the Y and UV components of the frame at the rotated coordinates
						  y_pixel = y_pixels + wrapped_j * y_stride + wrapped_i;
						  uv_pixel = uv_pixels + (wrapped_j / 2) * uv_stride + (wrapped_i / 2) * 2;

						  // Extract RGBA values from the logo image
						  Red = data[(j * width_logo + i) * 4];
						  Green = data[(j * width_logo + i) * 4 + 1];
						  Blue = data[(j * width_logo + i) * 4 + 2];
						  Alpha = data[(j * width_logo + i) * 4 + 3] * Alpha_Factor;

						  // Convert RGB to YUV
						  Y = (0.257 * Red) + (0.504 * Green) + (0.098 * Blue) + 16;  // Y component
						  U = (0.439 * Red) - (0.368 * Green) - (0.071 * Blue) + 128;  // U component
						  V = -(0.148 * Red) - (0.291 * Green) + (0.439 * Blue) + 128;  // V component

						  // Alpha blending
						  Y = ((255 - Alpha) * (*y_pixel) + Alpha * Y) / 255;
						  U = ((255 - Alpha) * (uv_pixel[0]) + Alpha * U) / 255;
						  V = ((255 - Alpha) * (uv_pixel[1]) + Alpha * V) / 255;

						  *y_pixel = Y;
						  uv_pixel[0] = U;
						  uv_pixel[1] = V;
					}
			}
		} else {
        // Error if the logo dimensions are too large
        cairo_surface_destroy(image_surface);
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Logo dimensions are too large."),
				("The logo size should be less than one-sixth of the frame size."));
				exit(1);
    }
    cairo_surface_destroy(image_surface);
}

/**
 * @brief Imposes the logo onto the YUV frame and rotate it on the specified coordinates with alpha blending.
 *
 * This function loads the logo image from the file specified in the filter and imposes it onto the YUV frame
 * at the specified coordinates with alpha blending and rotate it.
 *
 * @param filter The GstInsertLogo filter instance.
 * @param y_pixels The Y plane pixels of the YUV frame.
 * @param uv_pixels The UV plane pixels of the YUV frame.
 * @param y_stride The stride of the Y plane.
 * @param uv_stride The stride of the UV plane.
 */
static void
gst_insert_logo_rotate_logo(GstInsertLogo *filter, guint8 *y_pixels, guint8 *uv_pixels, guint y_stride, guint uv_stride)
{
          
    // Check the Rotete direction
		if (strcmp (filter->rotation, "clockwise") == 0 ) {
			// Rotate Clockwise
			if(strcmp (filter->speed, "slow") == 0){
		 		filter->degree = filter->degree + 0.5;
		 		if(filter->degree >= 360){
		 			filter->degree = 0 ;
		 		}
		 	} else if(strcmp (filter->speed, "medium") == 0){
		 		filter->degree = filter->degree + 1.5;
		 		if(filter->degree >= 360){
		 			filter->degree = 0 ;
		 		}
		 	} else if(strcmp (filter->speed, "fast") == 0){
		 		filter->degree = filter->degree + 2.5;
		 		if(filter->degree >= 360){
		 			filter->degree = 0 ;
		 		}
		 	}
		} else if (strcmp (filter->rotation, "counter-clockwise") == 0 ) {
			// Rotate AntiClockwise
			if(strcmp (filter->speed, "slow") == 0){
		 		filter->degree = filter->degree - 0.5;
		 		if(filter->degree <= -360){
		 			filter->degree = 0 ;
		 		}
		 	} else if(strcmp (filter->speed, "medium") == 0){
		 		filter->degree = filter->degree - 1.5;
		 		if(filter->degree <= -360){
		 			filter->degree = 0 ;
		 		}
		 	} else if(strcmp (filter->speed, "fast") == 0){
		 		filter->degree = filter->degree - 2.5;
		 		if(filter->degree <= -360){
		 			filter->degree = 0 ;
		 		}
		 	}
		}
		
    // Load the PNG image
    cairo_surface_t *image_surface = cairo_image_surface_create_from_png(filter->logo);
    if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
    {
        fprintf(stderr, "Error loading image: %s\n", cairo_status_to_string(cairo_surface_status(image_surface)));
        return;
    }

    int width = cairo_image_surface_get_width(image_surface);
    int height = cairo_image_surface_get_height(image_surface);

    // Determine the size of the rotated image
    double angle_radians = filter->degree * (M_PI / 180.0);
    //double cos_theta = fabs(cos(angle_radians));
    //double sin_theta = fabs(sin(angle_radians));
    //int width_rotated = ceil(width * cos_theta + height * sin_theta);
    //int height_rotated = ceil(width * sin_theta + height * cos_theta);

    // Create a new surface for the rotated image
    int max_size = width > height ? width : height;
    cairo_surface_t *rotated_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, max_size, max_size);
    cairo_t *cr = cairo_create(rotated_surface);

    // Calculate the center of the new surface
    double center_x = max_size / 2.0;
    double center_y = max_size / 2.0;

    // Calculate the offset to center the original image
    double offset_x = (max_size - width) / 2.0;
    double offset_y = (max_size - height) / 2.0;
    
   // Rotate the image around its center
    cairo_translate(cr, center_x, center_y);
    cairo_rotate(cr, angle_radians);
    cairo_translate(cr, -center_x, -center_y);

    // Draw the rotated image
    cairo_set_source_surface(cr, image_surface, offset_x, offset_y);
    cairo_paint(cr);

    guint width_logo = max_size;
    guint height_logo = max_size;
    
    // If the coordinate values are negative
    if(filter->cord_negative){
		  
		  if(filter->strict){
		      // If strict mode is enabled, raise an error and abort
			    cairo_surface_destroy(image_surface);
			    cairo_surface_destroy(rotated_surface);
			    GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid value (negative) for coordinate property."),
			    ("Valid values are positive integer numbers."));
			    exit(1);
		  } else {
		      // If not in strict mode, issue a warning
		      g_warning ("Invalid value (negative) '<%d, %d>' for coordinate property. Valid values are positive integer numbers.",
		                      filter->coordinate[0], filter->coordinate[1]);
		  }
		  
		  // Set default coordinates (top right corner)
		  filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;
		  
		  // Ensure the second coordinate is at least 1
		  if(filter->coordinate[1] == 0){
		      filter->coordinate[1] = 1;
		  }
		  
		  // Print the default coordinates
		  g_print("Default (Top Right) value set '<%d, %d>'\n", filter->coordinate[0], filter->coordinate[1]);
		  // Reset the flag
		  filter->cord_negative = FALSE;
		}
		
		// If the default flag is not set and the coordinates are out of bounds
		if(!filter->dflt && (filter->coordinate[0] > filter->frame_width || filter->coordinate[1] > filter->frame_height)){
				if(filter->strict){
				    // If strict mode is enabled, raise an error and abort
				    cairo_surface_destroy(image_surface);
			    	cairo_surface_destroy(rotated_surface);
			    	GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Invalid coordinates for the coordinate property."),
			    ("Valid values are within the frame dimensions."));
				} else {
				    // If not in strict mode, issue a warning
				    g_warning ("Coordinates out of bounds '<%d, %d>' for coordinate property. Valid values are within the frame dimensions.",
				                    filter->coordinate[0], filter->coordinate[1]);
				}
				filter->dflt = TRUE;
				filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  	filter->coordinate[1] = height_logo/30;
		}
		
		if(filter->dflt && !filter->scrl_dflt_cord){
			filter->coordinate[0] = (filter->frame_width - width_logo - (height_logo/30));
		  filter->coordinate[1] = height_logo/30;
		  g_print("filter->coordinate[0] : %d, filter->coordinate[1] : %d\n", filter->coordinate[0], filter->coordinate[1]);
		  filter->scrl_dflt_cord = TRUE;		
		}
    
    guint8 *data = cairo_image_surface_get_data(rotated_surface);
    
    //setting alpha to 100
    if(filter->alpha == -1){
    	filter->alpha = 100;
    }
    
    if(!filter->adjust_y_cord){
    	filter->coordinate[1] = filter->coordinate[1] - ((int)max_size / 2);
    	if(filter->coordinate[1] < 0){
    		filter->coordinate[1] = filter->frame_height + filter->coordinate[1] + (height / 2); 
    	}
    	filter->adjust_y_cord = TRUE;
    }
    
    gdouble Alpha_Factor = (filter->alpha / 100.0);
    				
		if((width < (filter->frame_width/6)) && (height < (filter->frame_height/6)))
    {		
			for (gint j = 0; j < height_logo; j++)
			{
					// Calculate the wrapped-around j coordinate
					gint wrapped_j = (j + filter->coordinate[1]) % filter->frame_height;

					for (gint i = 0; i < width_logo; i++)
					{
						  // Calculate the wrapped-around i coordinate
						  gint wrapped_i = (i + filter->coordinate[0]) % filter->frame_width;

						  guint8 *y_pixel = y_pixels + wrapped_j * y_stride + wrapped_i;
						  guint8 *uv_pixel = uv_pixels + (wrapped_j / 2) * uv_stride + (wrapped_i / 2) * 2;

						  // Extract RGBA values from the logo image
						  guint8 Red = data[(j * width_logo + i) * 4];
						  guint8 Green = data[(j * width_logo + i) * 4 + 1];
						  guint8 Blue = data[(j * width_logo + i) * 4 + 2];
						  guint8 Alpha = data[(j * width_logo + i) * 4 + 3] * Alpha_Factor;

						  // Convert RGBA to YUV
						  guint8 Y = (0.257 * Red) + (0.504 * Green) + (0.098 * Blue) + 16;  // Y component
						  guint8 U = (0.439 * Red) - (0.368 * Green) - (0.071 * Blue) + 128;  // U component
						  guint8 V = -(0.148 * Red) - (0.291 * Green) + (0.439 * Blue) + 128;  // V component

						  // Alpha blending
						  Y = ((255 - Alpha) * (*y_pixel) + Alpha * Y) / 255;
						  U = ((255 - Alpha) * (uv_pixel[0]) + Alpha * U) / 255;
						  V = ((255 - Alpha) * (uv_pixel[1]) + Alpha * V) / 255;

						  // Update the Y and UV components of the frame with the blended values
						  *y_pixel = Y;
						  uv_pixel[0] = U;
						  uv_pixel[1] = V;
					}
			}
		} else {
        // Error if the logo dimensions are too large
        cairo_surface_destroy(image_surface);
        GST_ELEMENT_ERROR(filter, RESOURCE, NOT_FOUND, ("Logo dimensions are too large."),
				("The logo size should be less than one-sixth of the frame size."));
				exit(1);
    }

		cairo_destroy(cr);
    cairo_surface_destroy(rotated_surface);
    cairo_surface_destroy(image_surface);
}

/**
 * @brief Entry point to initialize the insertlogo plug-in.
 *
 * This function initializes the insertlogo plug-in itself by registering the element factories
 * and other features.
 *
 * @param insertlogo The GstPlugin instance representing the insertlogo plug-in.
 * @return TRUE if initialization was successful, FALSE otherwise.
 */
static gboolean
insertlogo_init (GstPlugin * insertlogo)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template insertlogo' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_insert_logo_debug, "insertlogo",
      0, "Template insertlogo");

  return GST_ELEMENT_REGISTER (insert_logo, insertlogo);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstinsertlogo"
#endif


/**
 * @brief Macro used by GStreamer to define a plugin.
 *
 * This macro defines the necessary information for GStreamer to identify and initialize the plugin.
 * It specifies the plugin's version, name, initialization function, package version, license, package name,
 * and package origin.
 *
 * @param major Major version of GStreamer
 * @param minor Minor version of GStreamer
 * @param name Name of the plugin
 * @param description Description of the plugin
 * @param init_function Initialization function for the plugin
 * @param version Package version
 * @param license License of the plugin
 * @param package_name Name of the package
 * @param package_origin Origin of the package
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    insertlogo,
    "insert_logo",
    insertlogo_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
