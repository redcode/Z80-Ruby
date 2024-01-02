require "bundler/gem_tasks"
require 'rake/extensiontask'

CLOBBER.include(
	'.yardoc',
	'doc',
	'pkg'
)

Rake::ExtensionTask.new('z80') do |ext|
	ext.lib_dir = 'lib/z80'
end
