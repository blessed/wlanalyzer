top = '.'
out = 'build'
DUMPER = 'wldump'
ANALYZER = 'wlanalyzer'
VERSION = '0.1'

def options(ctx):
	ctx.load('compiler_cxx')
	ctx.add_option('-d', '--debug', action='store_true', default=False, help='Compile with debug symbols')

def configure(ctx):
	ctx.load('compiler_cxx')
	ctx.env.CXXFLAGS += ['-Wall']
	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g', '-O0', '-DDEBUG_BUILD']
	# Check for libev
	ctx.check_cxx(header_name='ev++.h')
	ctx.check_cxx(lib='ev', uselib_store='EV')
	# Check for pugixml
	ctx.check_cxx(lib='pugixml', uselib_store='PUGI')

def build(bld):
	source_files = bld.path.ant_glob('^src/**/*.cpp$', excl=['^src/**/wldump.cpp', '^src/**/wlanalyzer.cpp'])
	wldumper = [bld.path.make_node('/src/wldump.cpp')]
	wlanalyzer = [bld.path.make_node('/src/wlanalyzer.cpp')]
	print(source_files + wldumper)
	bld.program(source=source_files + wldumper, target=DUMPER, use=['EV', 'PUGI'])
	bld.program(source=source_files + wlanalyzer, target=ANALYZER, use=['EV', 'PUGI'])
