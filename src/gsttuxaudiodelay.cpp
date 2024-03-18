/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2024  <<user@hostname.org>>
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
 * SECTION:element-tuxaudiodelay
 *
 * an Element to create a speficied delay in milliseconds up to 30 seconds
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch osxaudiosrc device=74 ! audioconvert ! audioresample ! tux_audio_delay delay_ms=1000 ! autoaudiosink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include "gsttuxaudiodelay.h"

GST_DEBUG_CATEGORY_STATIC (gst_tux_audio_delay_debug);
#define GST_CAT_DEFAULT gst_tux_audio_delay_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DELAY_MS
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_tux_audio_delay_parent_class parent_class
G_DEFINE_TYPE (GstTuxAudioDelay, gst_tux_audio_delay, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (tux_audio_delay, "tux_audio_delay", GST_RANK_NONE,
    GST_TYPE_TUXAUDIODELAY);

static void gst_tux_audio_delay_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_tux_audio_delay_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_tux_audio_delay_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_tux_audio_delay_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the tuxaudiodelay's class */
static void
gst_tux_audio_delay_class_init (GstTuxAudioDelayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_tux_audio_delay_set_property;
  gobject_class->get_property = gst_tux_audio_delay_get_property;

  g_object_class_install_property (gobject_class, PROP_DELAY_MS,
      g_param_spec_uint ("delay_ms", "Delay (ms)", "Delay in Milliseconds",0, 30000,
          0, G_PARAM_READWRITE));

  gst_element_class_set_details_simple (gstelement_class,
      "tuxaudiodelay",
      "Filter/Effect/Audio",
      "Delay audio by a specific amount of time", " <<ufkfir@icloud.com>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}



/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void
gst_tux_audio_delay_init (GstTuxAudioDelay * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_tux_audio_delay_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
      GST_DEBUG_FUNCPTR (gst_tux_audio_delay_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->isFirstElement = true;
  filter->delay_ms=0;
    filter->totalBufferDuration=0;
    filter->bufferQueue = g_queue_new();
}

static void
gst_tux_audio_delay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTuxAudioDelay *filter = GST_TUXAUDIODELAY (object);

  switch (prop_id) {
  case PROP_DELAY_MS:
      filter->delay_ms = g_value_get_uint(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_tux_audio_delay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstTuxAudioDelay *filter = GST_TUXAUDIODELAY (object);

  switch (prop_id) {
  case PROP_DELAY_MS:
      g_value_set_uint(value, filter->delay_ms);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_tux_audio_delay_sink_event (GstPad * pad, GstObject * parent,
    GstEvent * event)
{
  GstTuxAudioDelay *filter;
  gboolean ret;

  filter = GST_TUXAUDIODELAY (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps *caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}



static GstFlowReturn
gst_tux_audio_delay_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    GstTuxAudioDelay *filter = GST_TUXAUDIODELAY (parent);

    bool isValidMetadata = false;
    // Create silent buffer if buffer duration is valid
    if (GST_BUFFER_PTS_IS_VALID(buf) && GST_BUFFER_DURATION_IS_VALID(buf)) {
        isValidMetadata= true;
        filter->latestBufferDuration = GST_BUFFER_DURATION(buf);
        GST_DEBUG_OBJECT(filter,"valid metadata!");
    } else {
        GST_DEBUG_OBJECT(filter,"not valid metadata!");
        if (filter->isFirstElement) {
            GST_ERROR_OBJECT(filter,"got first packet without valid metadata!");
        }
    }
        filter->isFirstElement=false;
        const GstClockTime duration_ms = filter->latestBufferDuration / GST_MSECOND;
        GST_DEBUG_OBJECT(filter,"Buffer duration: %" G_GUINT64_FORMAT " ms", duration_ms);
        // Store buffer to queue
        const auto element = g_new(QueueElement, 1);
        element->time = filter->latestBufferDuration;
        element->buffer = gst_buffer_ref(buf);
        g_queue_push_tail(filter->bufferQueue, element);
        filter->totalBufferDuration += filter->latestBufferDuration;
        GST_DEBUG_OBJECT(filter,"new total buffer duration (ms): %lld",(long long int)(filter->totalBufferDuration / GST_MSECOND));
        // Erase old (exceeding delay) buffers from end of list

        while (filter->totalBufferDuration > filter->delay_ms * GST_MSECOND) {
            GST_DEBUG_OBJECT(filter,"this happend if buffer was lowered in real time!");
            // Peek at the last element in the queue (equivalent to back() in std::list)

            if (auto last_element = static_cast<QueueElement *>(g_queue_peek_tail(filter->bufferQueue)); last_element != nullptr) {
                // Dereference the buffer
                gst_buffer_unref(last_element->buffer);

                // Subtract the buffer time from the total duration
                filter->totalBufferDuration -= last_element->time;
                GST_DEBUG_OBJECT(filter,"lowered buffer by %llu to total duration of %u", (long long int)(last_element->time), filter->totalBufferDuration);

                // Remove and free the last element of the queue (equivalent to pop_back() in std::list)
                g_free(g_queue_pop_tail(filter->bufferQueue));
            }
        }

        GST_DEBUG_OBJECT(filter,"totalBufferDuration %u while delay is %llu", filter->totalBufferDuration, (long long int)(filter->delay_ms * GST_MSECOND));

        // In the first 'delay' seconds play silent audio
        if (filter->totalBufferDuration < filter->delay_ms * GST_MSECOND) {
            GST_DEBUG_OBJECT(filter,"playing silent buffer");
            const guint bufferSize = gst_buffer_get_size(buf);
            const auto silent_buffer = gst_buffer_new_and_alloc(bufferSize);
                gst_buffer_memset (silent_buffer, 0, 0, bufferSize);
                if (isValidMetadata) {
                    gst_buffer_copy_into(silent_buffer, buf, GST_BUFFER_COPY_METADATA, 0, bufferSize);
                }

            return gst_pad_push(filter->srcpad, silent_buffer);
        } else {
            // Peek at the first element in the queue (equivalent to front() in std::list)

            if (const auto first_element = static_cast<QueueElement*>(g_queue_peek_head(filter->bufferQueue)); first_element != nullptr) {
                GST_DEBUG_OBJECT(filter,"playing from buffer!");
                const GstClockTime delay_time = filter->delay_ms * GST_MSECOND;
                GST_BUFFER_PTS(first_element->buffer) += delay_time;
                const GstFlowReturn result = gst_pad_push(filter->srcpad, first_element->buffer);

                // Subtract the buffer time from the total duration
                filter->totalBufferDuration -= first_element->time;

                gst_buffer_unref(first_element->buffer);
                g_free(g_queue_pop_head(filter->bufferQueue));
                return result;
            }
        }
        return gst_pad_push(filter->srcpad, buf);
}



/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
tuxaudiodelay_init (GstPlugin * tuxaudiodelay)
{
  /* debug category for filtering log messages
   *
   * exchange the string 'Template tuxaudiodelay' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_tux_audio_delay_debug, "tux_audio_delay",
      0, "Template tuxaudiodelay");

  return GST_ELEMENT_REGISTER (tux_audio_delay, tuxaudiodelay);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirsttuxaudiodelay"
#endif

/* gstreamer looks for this structure to register tuxaudiodelays
 *
 * exchange the string 'Template tuxaudiodelay' with your tuxaudiodelay description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    tuxaudiodelay,
    "Gstreamer Plugin for Delaying Audio",
    tuxaudiodelay_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
