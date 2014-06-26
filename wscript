top = '.'
out = 'build'
APPNAME = 'wldump'
VERSION = '0.1'

def options(ctx):
	ctx.load('compiler_cxx')
	ctx.add_option('-d', '--debug', action='store_true', default=False, help='Compile with debug symbols')

def configure(ctx):
	ctx.load('compiler_cxx')
	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g', '-O0']
	# Checking for libev
	ctx.check_cxx(header_name='ev++.h')
	ctx.check_cxx(lib='ev', use='EV')

def build(bld):
	source_files = bld.path.ant_glob('^src/*.cpp$')
	bld.program(source=source_files, target=APPNAME, use='EV')
