#! /usr/bin/env python


VERSION = '0.9'
APPNAME = 'minidjvu-mod'

srcdir = '.'
blddir = 'build'


def configure(conf):
    conf.check_tool('gcc g++')

    conf.check(header_name='math.h')
    conf.check(lib='m')

    conf.check(header_name='tiffio.h', define_name='HAVE_LIBTIFF')
    conf.check(lib='tiff')

    conf.check(header_name='libintl.h', define_name='HAVE_I18N')

    conf.check(header_name='stdint.h', define_name='HAVE_STDINT_H')
    conf.write_config_header('config.h') # included from mdjvucfg.h
  
    # Compilation flags 
    common_cflags = '-pipe -O3 -Wall -DHAVE_CONFIG_H -DNDEBUG'.split()
    conf.env.append_value('CCFLAGS', common_cflags + '''
        -D__STRICT_ANSI__ -Wshadow -pedantic-errors 
        -Wpointer-arith -Waggregate-return -Wlong-long 
        -Wredundant-decls -Wcast-qual -Wcast-align 
    '''.split())
    conf.env.append_value('CXXFLAGS', common_cflags)


def build(bld):
    bld.new_task_gen(
        features = 'cc cxx cstaticlib', # cshlib
        source = bld.glob('src/*/*.c') + bld.glob('src/*/*.cpp'),
        target = 'minidjvu-mod',
        includes = '# include', # '#' is where config.h is generated
        install_path = '${PREFIX}/lib',
        uselib = 'M TIFF'
    )
    
    bld.new_task_gen(
        features = 'cc cxx cprogram',
        source = 'tools/minidjvu-mod.c',
        target = 'minidjvu-mod',
        includes = '# include',
        install_path = '${PREFIX}/bin',
        uselib = 'M TIFF',
        uselib_local = 'minidjvu-mod'
    )
   
    headers = bld.glob('include/minidjvu-mod/*.h') + \
              bld.glob('include/minidjvu-mod/*/*.h')
    for i in headers:
        bld.install_files('${PREFIX}/' + i, i)
