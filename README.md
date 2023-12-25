# Z80-Ruby

Ruby binding for the [Z80](https://github.com/redcode/Z80) library.

## Installation

First, make sure that you have the Z80 library [installed](https://github.com/redcode/Z80#installation) on your system.

Then add the `z80` gem to the `Gemfile` of your project and run `bundle`:

```ruby
gem 'z80'
```

Or install the gem directly:

```shell
gem install z80
```

### Troubleshooting

#### "I installed the Z80 library through Homebrew, but `bundle` does not find it."

Configure the environment variables [`C_INCLUDE_PATH`](https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html#index-C_005fINCLUDE_005fPATH) and [`LIBRARY_PATH`](https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html#index-LIBRARY_005fPATH) by adding the installation prefix of the library:

```shell
export C_INCLUDE_PATH="$C_INCLUDE_PATH:$(brew --prefix)/include"
export LIBRARY_PATH="$LIBRARY_PATH:$(brew --prefix)/lib"
```

#### "I installed the Z80 library through Homebrew, but `gem` does not find it."

Tell `gem` the installation prefix of the library:

```shell
gem install z80 -- --with-Z80-dir=$(brew --prefix)
```

## Examples

### Z80 Instruction Set Exerciser

This small script demonstrates how to run the [CP/M versions of `zexall` and `zexdoc`](https://github.com/redcode/Z80/wiki/Z80-Instruction-Set-Exerciser) with a few lines of code:

```ruby
require 'z80'

memory = quit = nil
cpu = Z80.new

cpu.fetch_opcode = cpu.fetch = cpu.read do |context, address|
	memory[address]
end

cpu.write do |context, address, value|
	memory[address] = value
end

cpu.hook do |context, address|
	case address
	when 0 # END
		cpu.terminate
		quit = true
		0 # NOP
	when 5 # PRINT
		i = cpu.de
		while (c = memory[i]) != 0x24
			putc(c == 0xA ? "\n" : c) if c != 0xD
			i = (i + 1) & 0xFFFF
		end
		0xC9 # RET
	else
		0 # NOP
	end
end

ARGV.each do |file_path|
	program = file_path == '-' ? $stdin.read : File.read(file_path)
	puts "#{file_path}:"
	quit = false
	memory = Array.new(65536, 0)
	memory[0x0100, program.size] = program.bytes
	memory[0] = memory[5] = Z80::HOOK
	cpu.power true
	cpu.pc = 0x0100
	cpu.run(Z80::MAXIMUM_CYCLES) until quit
	puts
	break if file_path == '-'
end
```

<sup>**[<sub><img src="https://zxe.io/software/Z80/assets/images/ruby-icon.svg" height="14"></sub> run-yaze-zex.rb](https://zxe.io/software/Z80/scripts/run-yaze-zex.rb)**</sup>

Give it a try:

```
curl ftp://ftp.ping.de/pub/misc/emulators/yaze-1.14.tar.gz | tar -xOzf- yaze-1.14/test/zexall.com | ruby -e'eval `curl https://zxe.io/software/Z80/scripts/run-yaze-zex.rb`' -
```

### Zilog Z80 CPU Test Suite

This runs any tape from Patrik Rak's [Zilog Z80 CPU Test Suite](https://github.com/raxoft/z80test) (except `z80ccfscr.tap`):

```ruby
require 'z80'

module Opcode
	RET  = 0xC9
	NOP  = 0x00
	CALL = 0xCD
end

quit = cursor_x = tab = memory = nil
cpu = Z80.new

cpu.fetch_opcode = cpu.fetch = cpu.read do |context, address|
	memory[address]
end

cpu.write do |context, address, value|
	memory[address] = value if address > 0x3FFF
end

cpu.in do |context, port|
	port.odd? ? 255 : 191
end

cpu.hook do |context, address|
	case address
	when 0x0010 # PRINT
		if tab == 0
			case (c = cpu.a)
			when 0x0D # CR
				putc "\n"
				cursor_x = 0
			when 0x17 # TAB
				tab = 2
			when 0x7F # ©
				printf "©"
				cursor_x += 1
			else
				if c >= 32 && c < 127
					putc c
					cursor_x += 1
				end
			end
		elsif (tab -= 1) != 0
			c = 0x1F & cpu.a
			x = 0x1F & cursor_x
			if c < x
				putc "\n"
				cursor_x = 0
			else
				cursor_x += (c -= x)
			end
			print ' ' * c
		end
		Opcode::RET
	when 0x7003 # Exit
		cpu.terminate
		quit = true
		Opcode::NOP
	else
		Opcode::NOP
	end
end

ARGV.each do |file_path|
	program = file_path == '-' ? $stdin.read : File.read(file_path)
	puts "#{file_path}:"
	quit     = false
	cursor_x = tab = 0
	memory   = Array.new(65536, 0)
	memory[0x8000, program.size - 91] = program.bytes[91..-1]
	memory[0x0010] = Z80::HOOK    # THE 'PRINT A CHARACTER' RESTART
	memory[0x0D6B] = Opcode::RET  # THE 'CLS' COMMAND ROUTINE
	memory[0x1601] = Opcode::RET  # THE 'CHAN_OPEN' SUBROUTINE
	memory[0x7000] = Opcode::CALL # -.
	memory[0x7001] = 0x00         #  |- call 8000h
	memory[0x7002] = 0x80         # -'
	memory[0x7003] = Z80::HOOK
	cpu.power true
	cpu.im = 1
	cpu.i  = 0x3F
	cpu.pc = 0x7000
	cpu.run(Z80::MAXIMUM_CYCLES) until quit
	break if file_path == '-'
end
```

<sup>**[<sub><img src="https://zxe.io/software/Z80/assets/images/ruby-icon.svg" height="14"></sub> run-raxoft-z80test.rb](https://zxe.io/software/Z80/scripts/run-raxoft-z80test.rb)**</sup>

Give it a try:

```shell
curl http://zxds.raxoft.cz/taps/misc/z80test-1.2a.zip | bsdtar -xOf- z80test-1.2a/z80full.tap | ruby -e'eval `curl https://zxe.io/software/Z80/scripts/run-raxoft-z80test.rb`' -
```

## License

<img src="https://zxe.io/software/Z80/assets/images/0bsd.svg" width="160" align="right">

Copyright © 2023 Manuel Sainz de Baranda y Goñi.

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
