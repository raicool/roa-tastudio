# roa-tastudio
[![Release](https://badgen.net/github/release/raicool/roa-tastudio)](https://github.com/raicool/roa-tastudio/releases)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/raicool/roa-tastudio/graphs/commit-activity)
[![GitHub last commit](https://badgen.net/github/last-commit/raicool/roa-tastudio/master)](https://github.com/raicool/roa-tastudio/commits/master)

rivals 1 mod that implements a tas tool inspired by BizHawk's TAStudio.
###### requires roa-mod-loader to be installed to Rivals of Aether's game directory

<img width="2249" height="1265" alt="RivalsofAether_GVG7mwkqPg" src="https://github.com/user-attachments/assets/b9a4a9a4-7ed2-470e-9bfc-3c869ef7f09e" />

# building & installing
requires Visual Studio 2022 in order to be built (older visual studio versions most likely work but i have not tested them)

1. clone the repository as well as submodules:
```
	git clone https://github.com/raicool/roa-tastudio --recurse-submodules
	cd roa-tastudio
```
2. using Visual Studio's x86 Dev command prompt *vcvars32.bat* run
```
	cmake -G "Ninja" -DCMAKE_SYSTEM_PROCESSOR=i386 -B bin/
	ninja -C bin/
```
3. copy the dll `bin/bin/roa-hook-XXXXXX-MSVC.dll` into the mods directory of rivals of aether, e.g `C:/Program Files (x86)/Steam/steamapps/common/Rivals of Aether/mods/`
	- if the mods directory doesnt already exists, create one
	- copy the roa-hook folder into the mods folder as well
