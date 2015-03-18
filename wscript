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
	ctx.env.CXXFLAGS += ['-Wall', '-fPIC']
	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g', '-O0', '-DDEBUG_BUILD']

	ctx.recurse('src')

	if ctx.env.LIB_QT5QUICK and ctx.env.INCLUDES_QT5QUICK:
		ctx.env.BUILD_WLANALYZER = True
		print("Building a GUI analyzer")


def build(bld):
	bld.recurse('src')
	bld.recurse('src/base')
	bld.recurse('src/dumper')
	bld.recurse('src/analyzer')
