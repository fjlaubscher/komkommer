#!/usr/bin/env bash

# Package the Linux release binaries as AppImages.
#
# Usage:
#   scripts/package-linux.sh [bin-dir] [out-dir]
#
#   bin-dir  directory holding komkommertd and komkommerra
#            (default: build/release/RelWithDebInfo)
#   out-dir  where the .AppImage files are written
#            (default: build/release/linux-dist)
#
# Each AppImage bundles SDL2 and OpenAL. PulseAudio, PipeWire, and ALSA client
# libraries stay on the host: openal-soft dlopens them, and they have to match
# the machine's sound server. Needs rsvg-convert (librsvg2-bin).

set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
bin_dir=${1:-$root/build/release/RelWithDebInfo}
out_dir=${2:-$root/build/release/linux-dist}
tools=$root/build/appimage-tools

command -v rsvg-convert >/dev/null 2>&1 || {
	echo "rsvg-convert not found. Install it with: apt install librsvg2-bin" >&2
	exit 1
}

mkdir -p "$tools" "$out_dir"

fetch() {
	local url=$1 name=$2
	if [[ ! -x $tools/$name ]]; then
		curl -fsSL -o "$tools/$name.partial" "$url"
		mv "$tools/$name.partial" "$tools/$name"
		chmod +x "$tools/$name"
	fi
}

fetch https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage \
	linuxdeploy-x86_64.AppImage
fetch https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage \
	linuxdeploy-plugin-appimage-x86_64.AppImage
ln -sfn linuxdeploy-plugin-appimage-x86_64.AppImage "$tools/linuxdeploy-plugin-appimage"

export PATH="$tools:$PATH"
export APPIMAGE_EXTRACT_AND_RUN=1
export ARCH=x86_64

# Host audio clients. openal-soft dlopens Pulse, PipeWire, and ALSA, and SDL2
# links the first two. linuxdeploy still copies the rest of that ldd closure,
# and AppRun puts usr/lib ahead of the system path, so the private Pulse
# library and its dependencies have to be excluded too.
exclude_audio=(
	--exclude-library='libpulse.so.*'
	--exclude-library='libpulse-simple.so.*'
	--exclude-library='libpulsecommon*'
	--exclude-library='libasound.so.*'
	--exclude-library='libpipewire*.so.*'
	--exclude-library='libdbus-1.so.*'
	--exclude-library='libsndfile.so.*'
	--exclude-library='libasyncns.so.*'
	--exclude-library='libsystemd.so.*'
	--exclude-library='libapparmor.so.*'
	--exclude-library='libFLAC.so.*'
	--exclude-library='libvorbis.so.*'
	--exclude-library='libvorbisenc.so.*'
	--exclude-library='libopus.so.*'
	--exclude-library='libogg.so.*'
	--exclude-library='libmpg123.so.*'
	--exclude-library='libmp3lame.so.*'
	--exclude-library='libcap.so.*'
	--exclude-library='libgcrypt.so.*'
	--exclude-library='liblz4.so.*'
	--exclude-library='liblzma.so.*'
	--exclude-library='libzstd.so.*'
)

package_one() {
	local bin_name=$1 slug=$2 svg=$3
	local src=$bin_dir/$bin_name
	[[ -f $src ]] || {
		echo "missing $src" >&2
		exit 1
	}

	local stage
	stage=$(mktemp -d)
	# shellcheck disable=SC2064
	trap "rm -rf '$stage'" RETURN

	cp "$src" "$stage/$bin_name"
	strip --strip-unneeded "$stage/$bin_name"
	rsvg-convert -w 256 -h 256 -o "$stage/$slug.png" "$root/resources/$svg"

	local output=$out_dir/${slug}-x86_64.AppImage
	rm -f "$output"
	(
		cd "$stage"
		# LDAI_OUTPUT is the appimage plugin's output path. An unset value
		# names the file from the desktop entry, which is not the release name.
		LDAI_OUTPUT=$output linuxdeploy-x86_64.AppImage \
			--appdir "$stage/AppDir" \
			--executable "$stage/$bin_name" \
			--desktop-file "$root/resources/$slug.desktop" \
			--icon-file "$stage/$slug.png" \
			"${exclude_audio[@]}" \
			--output appimage
	)
	[[ -f $output ]] || {
		echo "linuxdeploy did not write $output" >&2
		exit 1
	}
	chmod +x "$output"
}

package_one komkommertd komkommer-td komkommertd_icon.svg
package_one komkommerra komkommer-ra komkommerra_icon.svg

echo "Wrote:"
echo "  $out_dir/komkommer-td-x86_64.AppImage"
echo "  $out_dir/komkommer-ra-x86_64.AppImage"
