# Z80-Ruby Changelog

This is the version history and changelog of Z80-Ruby. Version numbers do not correlate with those of the Z80 library. Release dates are in UTC time zone.

## 0.3.2 / 2024-01-05

### Bugfixes

* Added `Z80::MINIMUM_CYCLES_PER_STEP` (again). It was not actually added in v0.2.0 by mistake.

## 0.3.1 / 2024-01-05

### Bugfixes

* Fixed `Z80#full_r`, which previously acted as `Z80#refresh_address` by mistake.

## 0.3.0 / 2024-01-03

### Enhancements

* Added `Z80#sf?`, `Z80#zf?`, `Z80#yf?`, `Z80#hf?`, `Z80#xf?`, `Z80#pf?`, `Z80#nf?`, `Z80#cf?`, `Z80#iff1?`, `Z80#iff2?`, `Z80#int_line?` and `Z80#halt_line?`.
* Changed `Z80#sf`, `Z80#zf`, `Z80#yf`, `Z80#hf`, `Z80#xf`, `Z80#pf`, `Z80#nf` and `Z80#cf` to return an Integer instead of a boolean.
* The parameter of `Z80#power` is now optional and the default value is `true`.
* `Z80#to_h` now accepts one optional parameter. Passing `true` will create a Hash with boolean values for the following keys: `:iff1`, `:iff2`, `:int_line` and `:halt_line`.

### Bugfixes

* Fixed a potential bug or error when compiling the extension for Ruby `< 3.0`.

## 0.2.0 / 2024-01-02

### Enhancements

* Added `Z80::MINIMUM_CYCLES_PER_STEP`.
* Minor code improvements.

### Bugfixes

* Changed the order in which the files are required so that the extension is loaded before `'z80/version'`.
* Fixed typos in the names of `Z80#xyl`, `Z80#xyl=`, `Z80#wzh`, `Z80#wzh=`, `Z80#wzl` and `Z80#wzl=`.

### Project

* Added CI.
* Added `Gemfile`, `CITATION.cff`.
* Added [rake-compiler](https://rubygems.org/gems/rake-compiler) as a development dependency.

## 0.1.0 / 2023-12-24

Initial public release.
