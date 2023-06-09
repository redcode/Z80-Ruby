# Z80-Ruby

Ruby binding for the [Z80 library](https://github.com/redcode/Z80).

## Examples

This small script demonstrates how to run the [CP/M version of ZEXALL](https://github.com/redcode/Z80/wiki/Z80-Instruction-Set-Exerciser) with a few lines of code:

```ruby
require 'z80'

quit   = false
memory = Array.new(65536, 0)
cpu    = Z80.new

cpu.fetch_opcode = cpu.fetch = cpu.read do |context, address|
	memory[address]
end

cpu.write do |context, address, value|
	memory[address] = value
end

cpu.hook do |context, address|
	case address
	when 0 # END
		cpu.cycles = Z80::MAXIMUM_CYCLES
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

program = File.read(ARGV[0])
memory[0x0100, program.size] = program.bytes
memory[0] = memory[5] = Z80::HOOK
cpu.power true
cpu.pc = 0x0100
cpu.run(Z80::MAXIMUM_CYCLES) until quit
puts

```

<sup>**[<sub><img src="https://zxe.io/software/Z80/images/ruby-icon.svg" height="14"></sub> run-yaze-zexall.rb](https://zxe.io/software/Z80/scripts/run-yaze-zexall.rb)**</sup>

This runs any tape from Patrik Rak's [Zilog Z80 CPU Test Suite](https://github.com/raxoft/z80test) (except `z80ccfscr.tap`):

```ruby
require 'z80'

module Opcode
	RET  = 0xC9
	NOP  = 0x00
	CALL = 0xCD
end

quit     = false
tab      = 0
cursor_x = 0
memory   = Array.new(65536, 0)
cpu      = Z80.new

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
		cpu.cycles = Z80::MAXIMUM_CYCLES
		quit = true
		Opcode::NOP
	else
		Opcode::NOP
	end
end

program = File.read(ARGV[0])
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

```

<sup>**[<sub><img src="https://zxe.io/software/Z80/images/ruby-icon.svg" height="14"></sub> run-raxoft-z80test.rb](https://zxe.io/software/Z80/scripts/run-raxoft-z80test.rb)**</sup>

## License

BSD Zero Clause License

Copyright © 2023 Manuel Sainz de Baranda y Goñi.

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
