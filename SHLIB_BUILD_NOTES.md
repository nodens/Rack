# Note on building Rack using system shared libraries

The idea is to see if I also get rtaudio issues in Audio module with pipewire, when using released RTaudio on Debian (unstable).

## quick and dirty: Arch approach
Arch packaging does exactly that, so we'll copy the approach (from <https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=vcvrack>).

**build-depends**:
`sudo apt install markdown libglew-dev libglfw3-dev libjansson-dev libcurl4-openssl-dev libssl-dev libarchive-dev libzstd-dev libspeexdsp-dev libsamplerate0-dev librtmidi-dev librtaudio-dev`

**vendored submodules (for reference)**
  - "filesystem-7e37433.tar.gz::https://github.com/gulrak/filesystem/archive/7e37433tar.gz"
  - "fuzzysearchdatabase-fe62479.tar.gz::https://bitbucket.org/j_norberg/fuzzysearchdatabase/get/fe62479.tar.gz"
  - "nanosvg-25241c5.tar.gz::https://github.com/memononen/nanosvg/archive/25241c5.tar.gz"
  - "nanovg-0bebdb3.tar.gz::https://github.com/VCVRack/nanovg/archive/0bebdb3.tar.gz"
  - "osdialog-21b9dcc.tar.gz::https://github.com/AndrewBelt/osdialog/archive/21b9dcc.tar.gz"
  - "oui-blendish-2fc6405.tar.gz::https://github.com/VCVRack/oui-blendish/archive/2fc6405.tar.gz"
  - "pffft-74d7261.tar.gz::https://bitbucket.org/jpommier/pffft/get/74d7261.tar.gz"

`dep/Makefile` has been patched to add target `noshlibdeb`  (see `2be4fda`)

**Build**
```
  _ldflags="\
    -shared \
    -ldl \
    $(pkg-config --libs glew \
    glfw3 jansson libcurl openssl \
    libarchive libzstd speexdsp \
    samplerate rtmidi rtaudio)"
  make -C dep nodebshlib
  make LDFLAGS+="$_ldflags" all
```
At this stage, it fails, probably due to `ac73ef4` which appears to rely on unreleased rtaudio changes.
Let's do as the Arch folks, and revert this commit:
`git revert ac73ef4` (done in 95cdd14)

**build fundamental plugins**
```
cd plugins
git clone https://github.com/VCVRack/Fundamental.git
cd Fundamental
git submodule update --init --recursive
make dep
make
make install # installs in ~/.Rack
```

(note that if Fundamental has the same version, the module directory has to be deleted from ~/.Rack2/plugins/)
**run Rack**:
`./Rack`

**make dist zip**
```
make dist; cp dist/*.zip ../
```


## Make a proper Debian package

kinda easy using the same workflow I guess ? it would imply some dh_override but sounds feasible.

Anyway, the vendoring will probably be a no-go for Debian inclusion (it would need packaging the vendored deps first).

