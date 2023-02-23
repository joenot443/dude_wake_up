cd shaders
sed -i '' 's/iResolution/dimensions/g' $1.frag

sed -i '' 's/iChannel0/tex/g' $1.frag

sed -i '' 's/fragCoord/coord/g' $1.frag

sed -i '' 's/iTime/time/g' $1.frag

sed -i '' 's/fragColor/outputColor/g' $1.frag

sed -i '' 's/out vec4 outputColor, in vec2 coord//g' $1.frag
sed -i '' 's/mainImage/main/g' $1.frag

