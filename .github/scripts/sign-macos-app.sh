#!/usr/bin/env bash

# Sign (and, when secrets are present, notarize + staple) the komkommer macOS
# app bundles. Works on the .app bundles the CI build already produced.
#
# Usage:
#   sign-macos-app.sh <app> [<app> ...]
#
# Reads the same secrets siliconsun uses:
#   APPLE_CERTIFICATE_P12       base64 "Developer ID Application" .p12
#   APPLE_CERTIFICATE_PASSWORD  its password
#   APPLE_API_KEY               App Store Connect API key (.p8) contents
#   APPLE_API_KEY_ID            the key's Key ID
#   APPLE_API_ISSUER            the key's Issuer (team) ID
#
# With no certificate the bundle is ad-hoc signed; with no API key notarization
# is skipped. This mirrors siliconsun's packaging script so the same workflow
# works with or without Apple secrets.

set -euo pipefail

(($# > 0)) || { echo "usage: sign-macos-app.sh <app> [<app> ...]" >&2; exit 1; }

identity="-"
if [[ -n "${APPLE_CERTIFICATE_P12:-}" ]]; then
	[[ -n "${APPLE_CERTIFICATE_PASSWORD:-}" ]] || {
		echo "APPLE_CERTIFICATE_PASSWORD is required when APPLE_CERTIFICATE_P12 is set." >&2
		exit 1
	}

	previous=()
	while IFS= read -r line; do
		line="${line#"${line%%[![:space:]]*}"}"
		line="${line%\"}"
		line="${line#\"}"
		[[ -n "$line" ]] && previous+=("$line")
	done < <(security list-keychains -d user 2>/dev/null)

	p12=$(mktemp)
	keychain=$(mktemp)
	rm -f "$keychain"
	keychain="${keychain}.keychain-db"
	keychain_password=$(openssl rand -base64 24)

	printf '%s' "$APPLE_CERTIFICATE_P12" | base64 -D > "$p12"
	security create-keychain -p "$keychain_password" "$keychain"
	security set-keychain-settings -lut 21600 "$keychain"
	security unlock-keychain -p "$keychain_password" "$keychain"
	security import "$p12" -f pkcs12 -k "$keychain" -P "$APPLE_CERTIFICATE_PASSWORD" \
		-T /usr/bin/codesign -T /usr/bin/security
	security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$keychain_password" "$keychain"
	if ((${#previous[@]} > 0)); then
		security list-keychains -d user -s "$keychain" "${previous[@]}"
	else
		security list-keychains -d user -s "$keychain"
	fi
	rm -f "$p12"

	identity=$(security find-identity -v -p codesigning \
		| sed -n 's/.*"\(Developer ID Application:[^"]*\)"/\1/p' | head -1)
	[[ -z "$identity" ]] && { echo "certificate has no Developer ID Application identity" >&2; exit 1; }
	echo "Signing with $identity"
fi

notarize=0
if [[ "$identity" != "-" && -n "${APPLE_API_KEY:-}" && -n "${APPLE_API_KEY_ID:-}" && -n "${APPLE_API_ISSUER:-}" ]]; then
	notarize=1
else
	echo "Skipping notarization (missing Apple API key)."
fi

for app in "$@"; do
	[[ -d "$app" ]] || { echo "missing app: $app" >&2; exit 1; }
	name=$(basename "$app" .app)

	sign_args=(--force --sign "$identity")
	if [[ "$identity" != "-" ]]; then
		sign_args+=(--options runtime --timestamp)
	fi

	# Sign bundled dylibs (if any), the executable, then the bundle itself.
	while IFS= read -r -d '' lib; do
		codesign "${sign_args[@]}" "$lib"
	done < <(find "$app/Contents/libs" -type f -name '*.dylib' -print0 2>/dev/null || true)
	codesign "${sign_args[@]}" "$app/Contents/MacOS/$name"
	codesign "${sign_args[@]}" "$app"
	codesign --verify --strict "$app"

	if [[ $notarize -eq 1 ]]; then
		to_submit="$app"
		[[ -d "$to_submit" ]] || exit 1
		notarize_zip=$(mktemp)
		rm -f "$notarize_zip"
		notarize_zip="${notarize_zip}.zip"
		ditto -c -k --keepParent "$to_submit" "$notarize_zip"
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

	echo "Signed $app"
done
