#!/usr/bin/env bash

# Package the komkommer macOS .app bundles for distribution:
#   1. bundle the Homebrew SDL2/OpenAL dylibs so the app is portable,
#   2. sign with a Developer ID Application certificate (hardened runtime),
#   3. when an App Store Connect API key is supplied, notarize and staple,
#   4. zip each app.
#
# Usage:
#   scripts/package-macos.sh [app-dir] [out-dir]
#
#   app-dir  directory holding komkommertd.app + komkommerra.app
#            (default: build/release/RelWithDebInfo)
#   out-dir  where the .zip files are written (default: build/release/macos-dist)
#
# Environment (only needed for the notarization step):
#   APPLE_API_KEY     App Store Connect API key (.p8) contents
#   APPLE_API_KEY_ID  the key's Key ID
#   APPLE_API_ISSUER  the key's Issuer (team) ID
#
# The signing identity is taken from your login keychain (a "Developer ID
# Application" certificate). With no certificate the build is ad-hoc signed,
# matching what komkommer's own CI ships. Without an API key the script still
# bundles and signs; it just skips notarization.

set -euo pipefail

root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
app_dir="${1:-$root/build/release/RelWithDebInfo}"
out_dir="${2:-$root/build/release/macos-dist}"

# Source the local, git-ignored secrets file if present.
if [[ -f "$root/scripts/macos-notary.env" ]]; then
	# shellcheck disable=SC1091
	source "$root/scripts/macos-notary.env"
fi

command -v dylibbundler >/dev/null 2>&1 || {
	echo "dylibbundler not found. Install it with: brew install dylibbundler" >&2
	exit 1
}

APPS=(komkommertd komkommerra)

identity=$(security find-identity -v -p codesigning 2>/dev/null \
	| sed -n 's/.*"\(Developer ID Application:[^"]*\)".*/\1/p' | head -1)
if [[ -n "${identity:-}" ]]; then
	echo "Signing with: $identity"
else
	echo "No Developer ID Application identity found; falling back to ad-hoc signing."
	identity="-"
fi

notarize=1
for v in APPLE_API_KEY APPLE_API_KEY_ID APPLE_API_ISSUER; do
	[[ -n "${!v:-}" ]] || notarize=0
done
if [[ "$notarize" -eq 1 && "$identity" == "-" ]]; then
	echo "Notarization requested but no signing identity is available." >&2
	exit 1
fi
if [[ "$notarize" -eq 0 ]]; then
	echo "No App Store Connect API key in the environment; skipping notarization."
fi

rm -rf "$out_dir"
mkdir -p "$out_dir"

for name in "${APPS[@]}"; do
	src="$app_dir/$name.app"
	[[ -d "$src" ]] || { echo "missing $src" >&2; exit 1; }

	# 0. Stage a copy so the build tree stays pristine.
	app="$out_dir/$name.app"
	ditto "$src" "$app"

	# 1. Bundle Homebrew dylibs into the staged app.
	dylibbundler \
		--create-dir --bundle-deps --overwrite-files \
		--dest-dir "$app/Contents/libs" \
		--fix-file "$app/Contents/MacOS/$name"

	# 1b. sdl2-compat dlopen()s "libSDL3.dylib" at runtime, so dylibbundler
	#     misses it. Bundle SDL3 and an unversioned symlink so it resolves
	#     inside the app.
	sdl3_lib_dir="$(brew --prefix sdl3 2>/dev/null)/lib"
	sdl3_real=$(ls "$sdl3_lib_dir"/libSDL3.*.dylib 2>/dev/null | head -1)
	if [[ -n "${sdl3_real:-}" && -f "$sdl3_real" ]]; then
		cp -p "$sdl3_real" "$app/Contents/libs/libSDL3.0.dylib"
		chmod +w "$app/Contents/libs/libSDL3.0.dylib"
		install_name_tool -id "@rpath/libSDL3.0.dylib" "$app/Contents/libs/libSDL3.0.dylib"
		ln -sf libSDL3.0.dylib "$app/Contents/libs/libSDL3.dylib"
	else
		echo "warning: SDL3 not found at $sdl3_lib_dir; app needs it to launch" >&2
	fi

	# 2. Sign: bundled dylibs first, then the executable, then the bundle.
	sign_args=(--force --sign "$identity")
	if [[ "$identity" != "-" ]]; then
		sign_args+=(--options runtime --timestamp)
	fi
	while IFS= read -r -d '' lib; do
		codesign "${sign_args[@]}" "$lib"
	done < <(find "$app/Contents/libs" -type f -name '*.dylib' -print0 2>/dev/null || true)
	codesign "${sign_args[@]}" "$app/Contents/MacOS/$name"
	codesign "${sign_args[@]}" "$app"
	codesign --verify --strict --verbose=2 "$app"

	# 3. Notarize and staple when credentials are present.
	if [[ "$notarize" -eq 1 ]]; then
		notarize_zip="$out_dir/$name-notarize.zip"
		rm -f "$notarize_zip"
		ditto -c -k --keepParent "$app" "$notarize_zip"
		keyfile=$(mktemp)
		chmod 600 "$keyfile"
		printf '%s' "$APPLE_API_KEY" > "$keyfile"
		[[ "$APPLE_API_KEY" == *$'\n' ]] || printf '\n' >> "$keyfile"
		xcrun notarytool submit "$notarize_zip" \
			--key "$keyfile" \
			--key-id "$APPLE_API_KEY_ID" \
			--issuer "$APPLE_API_ISSUER" \
			--wait
		rm -f "$keyfile" "$notarize_zip"
		xcrun stapler staple "$app"
		xcrun stapler validate "$app"
	fi

	# 4. Zip the portable, signed (and stapled) app.
	ditto -c -k --keepParent "$app" "$out_dir/$name.zip"
	echo "Wrote $out_dir/$name.zip"
done

echo "Done. Distributables are in $out_dir"
