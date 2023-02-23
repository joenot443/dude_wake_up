cd shaders

sed -i '' 's/uni_Texture/tex/g' $1.frag

sed -i '' 's/texCoord/coord/g' $1.frag

sed -i '' 's/out_Color/outputColor/g' $1.frag

