gxmlviewer_sources = [
  'callbacks.c',
  'callbacks.h',
  'constants.h',
  'gxmlviewer.h',
  'interface.c',
  'interface.h',
  'main.c',
  'support.c',
  'support.h',
  'xmlparser.c',
  'xmlparser.h',
  'xmlview-control.c',
  'xmlview-control.h',
  'xmlwindow.c',
  'xmlwindow.h'
]

gxmlviewer_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0', version: '>= 3.22'),
  dependency('libxml-2.0', version: '>= 2.9'),
]

gnome = import('gnome')

gxmlviewer_sources += gnome.compile_resources('gxmlviewer-resources',
  'gxmlviewer.gresource.xml',
  c_name: 'gxmlviewer'
)

executable('gxmlviewer', gxmlviewer_sources,

  dependencies: gxmlviewer_deps,
  install: true,
)


