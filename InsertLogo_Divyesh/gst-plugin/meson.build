plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-template-plugin')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer template Plug-ins')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')
configure_file(output : 'config.h', configuration : cdata)

gstaudio_dep = dependency('gstreamer-audio-1.0',
    fallback: ['gst-plugins-base', 'audio_dep'])

#add depemdency for pixelbuf library    
dep_gdk_pixbuf = dependency('gdk-pixbuf-2.0')

#add depemdency for video frame
gstvideo_dep = dependency('gstreamer-video-1.0')

cairo_dep = dependency('cairo')

math_dep = cc.find_library('m', required: false)

# Plugin 1
plugin_sources = [
  'src/gstplugin.c'
  ]

gstpluginexample = library('gstplugin',
  plugin_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep],
  install : true,
  install_dir : plugins_install_dir,
)

# Plugin 2 (audio filter example)
audiofilter_sources = [
  'src/gstaudiofilter.c',
  ]

gstaudiofilterexample = library('gstaudiofilterexample',
  audiofilter_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstaudio_dep],
  install : true,
  install_dir : plugins_install_dir,
)

# The insertlogo Plugin
 gstinsertlogo_sources = [
  'src/gstinsertlogo.c',
  ]

gstinsertlogoexample = library('gstinsertlogo',
  gstinsertlogo_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstbase_dep, dep_gdk_pixbuf, gstvideo_dep, cairo_dep, math_dep],
  install : true,
  install_dir : plugins_install_dir,
)
