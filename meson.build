project('gst-plugin-tux-audiodelay', 'c', version : '1.19.0.1', license : 'LGPL',
)

plugins_install_dir = join_paths(get_option('libdir'), 'gstreamer-1.0')

gst_version = meson.project_version()

api_version = '1.0'

gst_dep = dependency('gstreamer-1.0', version : '>=1.19',
    required : true, fallback : ['gstreamer', 'gst_dep'])
gstbase_dep = dependency('gstreamer-base-1.0', version : '>=1.19',
  fallback : ['gstreamer', 'gst_base_dep'])

plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_version)
cdata.set_quoted('PACKAGE', 'gst-plugin-tux-audiodelay')
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', api_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'tux-audiodelay')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://tux-in.com')
configure_file(output : 'config.h', configuration : cdata)

gstaudio_dep = dependency('gstreamer-audio-1.0',
    fallback: ['gst-plugins-base', 'audio_dep'])

# Plugin 1
# The tuxaudiodelay Plugin
 gsttuxaudiodelay_sources = [
  'src/gsttuxaudiodelay.c',
  ]

gsttuxaudiodelayexample = library('gsttuxaudiodelay',
  gsttuxaudiodelay_sources,
  c_args: plugin_c_args,
  dependencies : [gst_dep, gstbase_dep],
  install : true,
  install_dir : plugins_install_dir,
)
