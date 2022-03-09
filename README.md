# mt63_wasm

This is very much a work in progress; the idea is to compile a mt63 library to wasm
using emscripten to allow encoding MT63 messages and "sending" them from an HTML5 audio
web application; this could be used to reliably transmit data from a smartphone app
across Amateur Radio FM signals on VHF/UHF.


# Release notes

* ?.?.? - Strip down library to just contain the WASM code, switch to building with make
* 1.5.0 - Add support for downsampling in WASM code
* 1.4.0 - Rebuild using a newer more optimized version of emscripten
* 1.3.3 - Misc updates to allow customization of the emscripten load Module
* 1.2.2 - Fixed broken wasm binary on iOS 


# Preparing to build

Install a emscripten.


# Building

Run `make`.


# Shameless plugs

This project relies heavily on the fldigi MT63 code and was put together by Richard Bateman, founder of HamStudy.org. To support our efforts, check out https://signalstuff.com/antennas (our main source of funding) and https://hamstudy.org/appstore for well engineered study apps for only $3.99. HamStudy.org is sponsored by Icom, which means that in a round-about sort of way Icom also sponsors this project =] They really
do a lot to build the ham radio community in the US so support them in whatever ways you can!

# License

mt63-wasm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

mt63-wasm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this project.  If not, see <http://www.gnu.org/licenses/>.
