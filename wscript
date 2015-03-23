#! /usr/bin/env python

APPNAME = 'wlanalyzer'
VERSION = '0.3'

top = '.'
out = 'build'

def options(ctx):
    ctx.add_option('-d', '--debug', action='store_true', default=False, help='Compile with debug symbols')
    ctx.add_option('--analyzer', action='store_true', default=False, help='Build the protocol analyzer. It is required to have qt5 libs installed on the system')
    ctx.recurse('src')


def configure(ctx):
	ctx.env.CXXFLAGS += ['-Wall', '-fPIC', '--std=c++11']
	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g', '-O2', '-DDEBUG_BUILD', '-Wall', '-Werror' ]

    if ctx.options.analyzer:
        ctx.env.analyzer = True
    else:
        ctx.env.analyzer = False

    ctx.recurse('src')


def build(bld):
    bld.recurse('src')
