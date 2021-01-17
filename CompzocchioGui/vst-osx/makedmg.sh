#!/bin/sh

mkdir Molot.vst/Contents/MacOS
mkdir Molot.vst/Contents/Resources

# binary
cp Molot Molot.vst/Contents/MacOS/

# resources
cp ../graphics/background0.png Molot.vst/Contents/Resources/bmp00100.png
cp ../graphics/english0.png Molot.vst/Contents/Resources/bmp00101.png
cp ../graphics/knobs0.png Molot.vst/Contents/Resources/bmp00102.png
cp ../graphics/lamp.png Molot.vst/Contents/Resources/bmp00103.png
cp ../graphics/meter_animation0.png Molot.vst/Contents/Resources/bmp00104.png
cp ../graphics/switch0.png Molot.vst/Contents/Resources/bmp00105.png
cp ../graphics/knob7o.png Molot.vst/Contents/Resources/bmp00106.png
cp ../graphics/knob4o.png Molot.vst/Contents/Resources/bmp00107.png
cp ../graphics/en_ru0.png Molot.vst/Contents/Resources/bmp00108.png
cp ../graphics/d_arrow.png Molot.vst/Contents/Resources/bmp00109.png
cp ../graphics/d_input.png Molot.vst/Contents/Resources/bmp00110.png
cp ../graphics/d_lim.png Molot.vst/Contents/Resources/bmp00111.png
cp ../graphics/d_sat.png Molot.vst/Contents/Resources/bmp00112.png
cp ../graphics/d_bypass.png Molot.vst/Contents/Resources/bmp00113.png
cp ../graphics/d_render.png Molot.vst/Contents/Resources/bmp00114.png
cp ../graphics/d_dither.png Molot.vst/Contents/Resources/bmp00115.png

DMG="Molot-vst-bin-osx-UB.dmg"

rm $DMG

hdiutil create -volname Molot-vst tmp-Molot-vst.dmg -fs HFS+J -size 16m
hdiutil attach -noverify "tmp-Molot-vst.dmg"
cp -R ./*.vst "/Volumes/Molot-vst"
cp install.txt "/Volumes/Molot-vst"
hdiutil detach "/Volumes/Molot-vst"
hdiutil convert "tmp-Molot-vst.dmg" -format UDZO -o $DMG
rm -f "tmp-Molot-vst.dmg"
