top = '.'
out = 'build'
DUMPER = 'wldump'
ANALYZER = 'wlanalyzer'
VERSION = '0.1'

def options(ctx):
	ctx.load('compiler_cxx')
	ctx.add_option('-d', '--debug', action='store_true', default=False, help='Compile with debug symbols')
	ctx.add_option('--analyzer', action='store_true', default=False, help='Build the protocol analyzer. It is required to have qt5 libs installed on the system')

def configure(ctx):
	ctx.load('compiler_cxx')
	if ctx.options.analyzer:
		ctx.load('qt5')
	ctx.env.CXXFLAGS += ['-Wall', '-fPIC']
	if ctx.options.debug:
		ctx.env.CXXFLAGS += ['-g', '-O0', '-DDEBUG_BUILD']
	# Check for libev
	ctx.check_cxx(header_name='ev++.h')
	ctx.check_cxx(lib='ev', uselib_store='EV')
	# Check for pugixml
	ctx.check_cxx(lib='pugixml', uselib_store='PUGI')

	if ctx.env.LIB_QT5QUICK and ctx.env.INCLUDES_QT5QUICK:
		ctx.env.BUILD_WLANALYZER = True
		print("Building a GUI analyzer")

def build(bld):
	source_files = bld.path.ant_glob('^src/**/*.cpp$', excl=['^src/**/wldump.cpp', '^src/**/wlanalyzer.cpp'])
	wldumper = [bld.path.make_node('/src/wldump.cpp')]
	wlanalyzer = [bld.path.make_node('/src/wlanalyzer.cpp')]
	bld.program(source=source_files + wldumper, target=DUMPER, use=['EV', 'PUGI'])

	usage = [ 'EV', 'PUGI' ]
	feature_set = 'cxx cxxprogram'

	if bld.env.BUILD_WLANALYZER:
		usage += [ 'QT5QUICK', 'QT5CORE', 'QT5WIDGETS' ]
		feature_set += 'qt5'

	if bld.env.BUILD_WLANALYZER:
		bld.program(source=source_files + wlanalyzer, target=ANALYZER, use=usage, features=feature_set)
