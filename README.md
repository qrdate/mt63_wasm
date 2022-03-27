# mt63-wasm

This WASM module wraps work done by Pawel Jalocha, SP9VRC and Dave Freese, W1HKJ on the MT63 transmitter and receiver in C++ for Linux.

[The original code for this repo](https://github.com/taxilian/mt63_wasm) is by Richard Bateman, founder of [HamStudy.org](https://hamstudy.org/).

### Notes

The resulting code for this module is inlined inside the generated .js file. As the module is not large (the resulting file is about 94kB), we see this as a sufficient workaround to importing the module across environments without resulting to other hacks before `new URL(url, import.meta.url)` is universally supported.

## Release notes

### 2.1.0

- Switched to use clang and [TinyWASI](https://github.com/qrdate/tinywasi) runtime instead of emscripten
- New Lanczos resampler
- Separated received and transmitter into their own binaries
- Removed ESLint

### 2.0.0

- Strip down library to just contain the WASM code
- Switch to building with make
- Patches to C++ glue code
- Update emscripten config to inline the resulting .wasm file
- Center frequency is not fixed to 1500Hz anymore

### Previous

* 1.5.0 - Add support for downsampling in WASM code
* 1.4.0 - Rebuild using a newer more optimized version of emscripten
* 1.3.3 - Misc updates to allow customization of the emscripten load Module
* 1.2.2 - Fixed broken wasm binary on iOS

## Requirements

- Version of clang that supports the reactor model (13 or up)

## Building

Run `make`.

Resulting files will be in `dist/`.

## License

mt63-wasm is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

mt63-wasm is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this project.  If not, see <http://www.gnu.org/licenses/>.
