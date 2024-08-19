project('lexbor', 'c',
  version : '0.2.0',
  meson_version : '>= 0.48',
  default_options : [
    'warning_level=1',
    'buildtype=debugoptimized',
    'c_std=c99'
  ]
)

v_array = meson.project_version().split('.')
v_maj = v_array[0]
v_min = v_array[1]
v_mic = v_array[2]

# install paths

dir_prefix = get_option('prefix')
dir_include = join_paths(dir_prefix, get_option('includedir'))
dir_pkginclude = join_paths(dir_include, meson.project_name())
dir_bin = join_paths(dir_prefix, get_option('bindir'))
dir_lib = join_paths(dir_prefix, get_option('libdir'))
dir_data = join_paths(dir_prefix, get_option('datadir'))
dir_pkgdata = join_paths(dir_data, meson.project_name())

# host

host_os = host_machine.system()
linux = ['linux']
bsd = ['bsd', 'freebsd', 'dragonfly', 'netbsd', 'openbsd']
osx = ['darwin']
win32 = ['windows']
cygwin = ['cygwin']
sys_linux = linux.contains(host_os)
sys_bsd = bsd.contains(host_os)
sys_windows = win32.contains(host_os)
sys_cygwin = cygwin.contains(host_os)
sys_osx = osx.contains(host_os)

# binaries

cc = meson.get_compiler('c')

lexbor_cflags = []
lexbor_cflags_try = [
  '-pedantic',
  '-Wall',
  '-Wextra',
  '-Wshadow',
  '-Wstrict-prototypes',
  '-Werror=missing-prototypes',
  '-Werror=pointer-arith',
  '-Wno-missing-field-initializers']

if sys_windows == true
  lexbor_cflags_try += [ '-Wno-pedantic-ms-format' ]
endif

foreach cf: lexbor_cflags_try
  if cc.has_argument(cf) == true
    lexbor_cflags += cf
  endif
endforeach
add_global_arguments(lexbor_cflags, language: 'c')

have_visibility_hidden = cc.has_argument('-fvisibility=hidden')
if have_visibility_hidden
  add_global_arguments('-fvisibility=hidden', language: 'c')
endif

# libraries

have_ceil = cc.has_header_symbol('math.h', 'ceil')
if have_ceil == false
  have_ceil = cc.has_header_symbol('math.h', 'ceil', dependencies : 'm')
endif
if have_ceil == false
  error('ceil() is required')
endif

config_dir = [include_directories('.'), include_directories('../source')]

lexbor_args = [ '-DLEXBOR_BUILDING' ]
if sys_linux == true
  lexbor_args += [ '-D_POSIX_C_SOURCE=199309L' ]
endif

if get_option('perf')
  lexbor_args += [ '-DLEXBOR_WITH_PERF' ]
endif

# subdirs

subdir('source/lexbor')

if get_option('examples')
  subdir('examples/lexbor')
endif