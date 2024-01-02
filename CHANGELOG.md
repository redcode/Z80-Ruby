# Z80-Ruby ChangeLog

## 0.2.0 / 2024-01-02

### Enhancements

* Added [`Z80::MINIMUM_CYCLES_PER_STEP`](https://zxe.io/software/Z80/documentation/latest/APIReference.html#c.Z80_MINIMUM_CYCLES_PER_STEP).
* Minor code improvements.

### Bugfixes

* Changed the order in which the files are required so that the extension is loaded before `'z80/version'`.
* Fixed typos in the names of `#xyl`, `#xyl=`, `#wzh`, `#wzh=`, `#wzl` and `#wzl=`.

### Project

* Added CI.
* Added `Gemfile`, `CITATION.cff`.
* Added [rake-compiler](https://rubygems.org/gems/rake-compiler) as a development dependency.

## 0.1.0 / 2023-12-24

Initial public release.
