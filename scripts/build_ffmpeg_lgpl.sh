#!/bin/bash
#
# Builds the minimal LGPL ffmpeg the Windows app ships: decoders, demuxers
# and parsers for the video and audio formats people play in Nottawa, plus
# swscale and swresample. No encoders, muxers, network, devices, filters or
# external libraries, so the DLLs are small and carry only ffmpeg's own
# LGPL-2.1-or-later licence (no --enable-gpl, no --enable-version3, no
# --enable-nonfree).
#
# Cross-compiles from WSL/Linux with mingw-w64:
#   sudo apt-get install -y mingw-w64 nasm make pkg-config
#
# Installs into the layout the build uses (as scripts/fetch_ffmpeg_lgpl.ps1
# did): bin/<lib>-<major>.dll, libs/ffmpeg/include, libs/ffmpeg/lib/<lib>.lib.
# The MSVC import libraries are made with lib.exe (Visual Studio) on the
# Windows side. Also writes the exact source tarball to dist/ffmpeg-source/,
# the "corresponding source" the LGPL asks us to offer.
#
# Usage: scripts/build_ffmpeg_lgpl.sh
set -euo pipefail

VERSION=8.1.2
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="$ROOT/.cache/ffmpeg-build"
SRC="$WORK/ffmpeg-$VERSION"
PREFIX="$WORK/install"
TARBALL="ffmpeg-$VERSION.tar.xz"

for tool in x86_64-w64-mingw32-gcc nasm make pkg-config; do
  command -v "$tool" >/dev/null || { echo "missing $tool: sudo apt-get install -y mingw-w64 nasm make pkg-config" >&2; exit 1; }
done

mkdir -p "$WORK"
cd "$WORK"
if [[ ! -f "$TARBALL" ]]; then
  curl -fL --retry 3 -o "$TARBALL" "https://ffmpeg.org/releases/$TARBALL"
  curl -fL --retry 3 -o "$TARBALL.asc" "https://ffmpeg.org/releases/$TARBALL.asc"
fi
rm -rf "$SRC" "$PREFIX"
tar xf "$TARBALL"
cd "$SRC"

# Video: H.264, HEVC, VP8/9, ProRes, DNxHD/HR, MPEG-1/2/4, Motion JPEG,
# QuickTime Animation and other intermediate codecs, WMV/VC-1, Theora, DV,
# PNG/GIF/APNG, raw. Audio (a clip's soundtrack): AAC, MP3, Opus, Vorbis,
# FLAC, ALAC, AC-3/E-AC-3, PCM. Containers: MP4/MOV, Matroska/WebM, AVI,
# MPEG-TS/PS, FLV, Ogg, ASF/WMV, MXF, DV, and raw streams.
DECODERS=h264,hevc,vp8,vp9,prores,dnxhd,mpeg1video,mpeg2video,mpeg4,msmpeg4v2,msmpeg4v3,h263,mjpeg,mjpegb,qtrle,cinepak,rawvideo,v210,huffyuv,ffvhuff,utvideo,wmv1,wmv2,wmv3,vc1,theora,vp6,vp6f,flv,dvvideo,png,apng,gif,aac,aac_latm,mp3float,mp3,mp2float,opus,vorbis,flac,alac,ac3,eac3,wmav2,pcm_s16le,pcm_s16be,pcm_s24le,pcm_s24be,pcm_s32le,pcm_f32le,pcm_f32be,pcm_u8,pcm_alaw,pcm_mulaw,adpcm_ima_qt,adpcm_ms
DEMUXERS=mov,matroska,avi,mpegts,mpegps,flv,ogg,asf,mxf,dv,h264,hevc,m4v,mpegvideo,ivf,gif,apng,wav,mp3,aac,flac,ac3,eac3
PARSERS=h264,hevc,vp8,vp9,mpeg4video,mpegvideo,h263,mjpeg,dnxhd,vc1,png,gif,dvbsub,aac,aac_latm,mpegaudio,opus,vorbis,flac,ac3,dca

./configure \
  --prefix="$PREFIX" \
  --target-os=mingw32 --arch=x86_64 --cross-prefix=x86_64-w64-mingw32- \
  --enable-shared --disable-static \
  --disable-programs --disable-doc --disable-debug --disable-autodetect \
  --disable-everything --disable-network --disable-avdevice --disable-avfilter \
  --enable-avcodec --enable-avformat --enable-swscale --enable-swresample \
  --enable-w32threads \
  --enable-protocol=file \
  --enable-decoder="$DECODERS" --enable-demuxer="$DEMUXERS" --enable-parser="$PARSERS" \
  --enable-bsf=h264_mp4toannexb,hevc_mp4toannexb,vp9_superframe_split,mpeg4_unpack_bframes \
  --extra-ldflags="-static-libgcc" | tee "$WORK/configure.log"
# The licence configure settled on must be plain LGPL 2.1+.
grep -q '^License: LGPL version 2.1 or later' "$WORK/configure.log" || { echo "refusing: not an LGPL-2.1-or-later configuration" >&2; exit 1; }

make -j"$(nproc)"
make install

# Install into the tree.
BIN="$ROOT/bin"
LIBS="$ROOT/libs/ffmpeg"
rm -f "$BIN"/avcodec-*.dll "$BIN"/avformat-*.dll "$BIN"/avutil-*.dll "$BIN"/swscale-*.dll "$BIN"/swresample-*.dll \
      "$BIN"/avdevice-*.dll "$BIN"/avfilter-*.dll "$BIN"/postproc-*.dll
cp "$PREFIX"/bin/*.dll "$BIN"/
rm -rf "$LIBS/include"
mkdir -p "$LIBS/lib"
cp -r "$PREFIX/include" "$LIBS/include"
# MSVC import libraries from the .def files ffmpeg writes next to each DLL.
DEFS="$WORK/defs"
rm -rf "$DEFS"; mkdir -p "$DEFS"
for def in "$SRC"/lib*/*.def; do cp "$def" "$DEFS/"; done
WIN_DEFS=$(wslpath -w "$DEFS")
WIN_LIB=$(wslpath -w "$LIBS/lib")
powershell.exe -NoProfile -Command "
  \$vs = & \"\${env:ProgramFiles(x86)}\\Microsoft Visual Studio\\Installer\\vswhere.exe\" -latest -products '*' -property installationPath
  \$tools = Get-ChildItem (Join-Path \$vs 'VC\\Tools\\MSVC') -Directory | Sort-Object Name -Descending | Select-Object -First 1
  \$lib = Join-Path \$tools.FullName 'bin\\Hostx64\\x64\\lib.exe'
  foreach (\$def in Get-ChildItem '$WIN_DEFS' -Filter *.def) {
    \$name = (\$def.BaseName -split '-')[0]
    & \$lib /nologo /machine:x64 /def:\$(\$def.FullName) /out:'$WIN_LIB'\\\$name.lib | Out-Null
    if (\$LASTEXITCODE -ne 0) { throw \"lib.exe failed for \$(\$def.Name)\" }
  }"

# The corresponding source for the LGPL, with how it was configured.
OUT="$ROOT/dist/ffmpeg-source"
mkdir -p "$OUT"
cp "$WORK/$TARBALL" "$WORK/$TARBALL.asc" "$OUT/"
cp "$0" "$OUT/build_ffmpeg_lgpl.sh"
cp "$SRC/COPYING.LGPLv2.1" "$ROOT/packaging/windows/licenses/LGPL-2.1.txt"

echo
ls -l "$PREFIX"/bin/*.dll | awk '{printf "%6.1f MB  %s\n", $5/1048576, $9}'
echo "installed into bin/ and libs/ffmpeg/; source in dist/ffmpeg-source/"
