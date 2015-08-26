#!/bin/bash
################################################################################
## Hero of Allacrost, Mac OS X build script                                   ##
## Cobbled together by Alistair Lynn <alistair@alynn.co.uk>, 2015-06-28       ##
################################################################################
set -e

TARGET=$1
BUILD=$(mktemp -d -t allacrost-build)

echo "Building in $BUILD"

# Default to the game/ directory
if [ -z "$TARGET" ]; then
    TARGET=game
fi

# Make sure the target is actually a directory
if [ ! -d "$TARGET" ]; then
    echo "Not a valid Allacrost source tree?"
    exit 1
fi

# Test for Homebrew being installed
if ! brew --version >/dev/null; then
    echo "Homebrew not installed!"
    echo "Please see http://brew.sh/ for install instructions."
    exit 1
fi

# Test for Homebrew dependencies
function require_installed {
    if brew info $1 | grep 'Not installed' >/dev/null; then
        echo "$1 not installed."
        echo "Run $ brew install $1"
        exit 1
    fi
}

require_installed libpng
require_installed libjpeg
require_installed libogg
require_installed libvorbis
require_installed boost
require_installed lua51
require_installed luabind
require_installed gettext
require_installed sdl
require_installed sdl_ttf

# Collect all .cpp source files from $TARGET/src
# We exclude the editor since it's not part of the OS X build, and luabind
# because we use a distribution from Homebrew.
SOURCES=$(find $TARGET/src -name '*.cpp' | grep -v editor | grep -v luabind)

# Copy in dependency files into the temporary build source
echo "Copying in temporary build sources..."
SYSINC="$BUILD/system-include"
mkdir "$SYSINC"
SYSLIB="$BUILD/system-lib"
mkdir "$SYSLIB"

echo "  SDL..."
cp -LR /usr/local/include/SDL "$SYSINC/SDL"
cp /usr/local/lib/libSDL.a "$SYSLIB/libSDL.a"
echo "  SDL_ttf..."
mkdir "$SYSINC/SDL_ttf"
echo "#include <SDL/SDL_ttf.h>" > "$SYSINC/SDL_ttf/SDL_ttf.h"
cp /usr/local/lib/libSDL_ttf.a "$SYSLIB/SDL_ttf.a"
cp /usr/local/lib/libfreetype.a "$SYSLIB/libfreetype.a"
echo "  Lua..."
cp /usr/local/include/lua5.1/* "$SYSINC/"
echo "  luabind..."
cp -LR /usr/local/include/luabind "$SYSINC/luabind"
echo "  gettext..."
cp $(brew list gettext | cut -f1 -d' ' | grep -E '/include/([^/]+)\.h$') "$SYSINC/"
cp $(brew list gettext | cut -f1 -d' ' | grep -E '/lib/libintl\.a$') "$SYSLIB/"
echo "  libpng..."
cp $(brew list libpng | cut -f1 -d' ' | grep -E '/include/([^/]+)\.h$') "$SYSINC/"
cp /usr/local/lib/libpng.a "$SYSLIB/libpng.a"
echo "  libjpeg..."
cp $(brew list libjpeg | cut -f1 -d' ' | grep -E '/include/([^/]+)\.h$') "$SYSINC/"
cp /usr/local/lib/libjpeg.a "$SYSLIB/libjpeg.a"
echo "  libvorbis..."
cp -LR /usr/local/include/vorbis "$SYSINC/vorbis"
cp /usr/local/lib/libvorbis.a "$SYSLIB/libvorbis.a"
cp /usr/local/lib/libvorbisfile.a "$SYSLIB/libvorbisfile.a"
cp /usr/local/lib/libvorbisenc.a "$SYSLIB/libvorbisenc.a"
cp /usr/local/lib/libogg.a "$SYSLIB/libogg.a"

INCLUDES=$(find $TARGET/src -type d | grep -v editor | grep -v luabind | sed 's/^/-I/')

touch "$BUILD/objects.txt"

for source in $SOURCES; do
    hash=$(echo $source | md5sum | cut -f1 -d' ')
    outfile="$BUILD/$hash.o"
    echo "Compiling $source to $outfile"
    clang++ -O3 -flto -c -o "$outfile" "$source" -I"$SYSINC/SDL" -I"$SYSINC" $INCLUDES
    echo $outfile >> "$BUILD/objects.txt"
done

echo "Compiling $TARGET/src/SDLMain.m to $BUILD/SDLMain.o"
clang -O3 -flto -c -o "$BUILD/SDLMain.o" -I"$SYSINC/SDL" -I"$SYSINC" "$TARGET/src/SDLMain.m"
echo "$BUILD/SDLMain.o" >> "$BUILD/objects.txt"

echo "Linking..."
cat "$BUILD/objects.txt" | xargs clang++ -O3 -flto -o "$BUILD/allacrost" -framework Cocoa -framework OpenGL -framework OpenAL -framework IOKit -lbz2 -liconv "$SYSLIB"/*.a -llua5.1 -lluabind -lz -framework CoreAudio -framework Carbon -framework AudioUnit
strip "$BUILD/allacrost"

BUNDLE="$BUILD/Hero of Allacrost.app"
echo "Constructing application bundle..."
mkdir "$BUNDLE"
mkdir "$BUNDLE/Contents"
mkdir "$BUNDLE/Contents/MacOS"
cp "$BUILD/allacrost" "$BUNDLE/Contents/MacOS/allacrost"
cat >"$BUNDLE/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>CFBundleExecutable</key>
    <string>allacrost</string>
    <key>CFBundleIconFile</key>
    <string>Allacrost.icns</string>
    <key>CFBundleIdentifier</key>
    <string>org.allacrost.Allacrost</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Hero of Allacrost</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
    <key>CFBundleDisplayName</key>
    <string>Hero of Allacrost</string>
</dict>
</plist>
EOF
mkdir "$BUNDLE/Contents/Resources"
cp "$TARGET/img/logos/mac_logo.icns" "$BUNDLE/Contents/Resources/Allacrost.icns"
for data in img lua mus snd txt; do
    cp -LR "$TARGET/$data" "$BUNDLE/Contents/Resources/$data"
done

echo "Fixing shared libraries..."
mkdir "$BUNDLE/Contents/SharedLibraries"
cp /usr/local/lib/liblua.5.1.dylib "$BUNDLE/Contents/SharedLibraries/liblua.5.1.dylib"
cp /usr/local/lib/libluabind.dylib "$BUNDLE/Contents/SharedLibraries/libluabind.dylib"
chmod 644 "$BUNDLE/Contents/SharedLibraries"/*.dylib
install_name_tool -change "/usr/local/lib/liblua.5.1.dylib" "@executable_path/../SharedLibraries/liblua.5.1.dylib" "$BUNDLE/Contents/MacOS/allacrost"
install_name_tool -change "/usr/local/lib/liblua.5.1.dylib" "@executable_path/../SharedLibraries/liblua.5.1.dylib" "$BUNDLE/Contents/SharedLibraries/libluabind.dylib"
install_name_tool -change "/usr/local/lib/libluabind.dylib" "@executable_path/../SharedLibraries/libluabind.dylib" "$BUNDLE/Contents/MacOS/allacrost"

echo "Copying built app..."
rm -rf "./Hero of Allacrost.app"
cp -R "$BUNDLE" "./Hero of Allacrost.app"

echo "Cleaning up..."
rm -rf "$BUILD"
