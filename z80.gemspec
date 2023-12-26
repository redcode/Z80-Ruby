require_relative 'lib/z80/version'

Gem::Specification.new do |s|
	s.name        = 'z80'
	s.version     = Z80::VERSION
	s.authors     = 'Manuel Sainz de Baranda y Goñi'
	s.email       = 'manuel@zxe.io'
	s.homepage    = 'https://zxe.io/software/Z80-Ruby'
	s.license     = '0BSD'
	s.summary     = "Ruby binding for the Z80 library."
	s.description = <<~EOS
		Z80-Ruby is a Ruby binding for the Zilog Z80 CPU emulator \
		(https://github.com/redcode/Z80). It is ideal for analysis, \
		hacking, testing and debugging. All from the comfort of Ruby.
		EOS

	s.metadata = {
		'bug_tracker_uri' => "https://github.com/redcode/Z80-Ruby/issues",
		'changelog_uri'   => 'https://github.com/redcode/Z80-Ruby/blob/master/CHANGELOG.md',
		'homepage_uri'    => 'https://zxe.io/software/Z80-Ruby',
		'source_code_uri' => 'https://github.com/redcode/Z80-Ruby',
	}

	s.files = [
		# `git ls-files | sort`.split("\n")
		'.editorconfig',
		'.github/FUNDING.yml',
		'.gitignore',
		'CHANGELOG.md',
		'LICENSE-0BSD',
		'README.md',
		'Rakefile',
		'ext/z80/extconf.rb',
		'ext/z80/z80.c',
		'lib/z80.rb',
		'lib/z80/version.rb',
		'z80.gemspec'
	]

	s.extensions = %w(ext/z80/extconf.rb)
end
