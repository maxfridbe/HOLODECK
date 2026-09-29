#!/bin/bash
# Versions look like YY.MMDD.## (e.g. 26.0929.03 = the 3rd release on 2026-09-29).
#
#   ./increment_version.sh          bump: today's date, next release number
#   ./increment_version.sh --print  print the next version, change nothing (CI uses this)
#   ./increment_version.sh --set V  write a specific version everywhere (CI records the release)
#
# The release number restarts at 01 each day. It continues from version.txt
# and from any existing v<YY.MMDD>.## git tags, so a version is never reused.
#
# The padded form goes in version.txt, git tags, release names and the Android
# versionName. Cargo insists on semver (no leading zeros) and cargo-apk packs
# each of major.minor.patch into one byte, so Cargo.toml carries the date as
# YY.M.D with the release number as build metadata: 26.0929.03 -> 26.9.29+03.
# The Gradle versionCode is YYMMDDNN, which grows with every release.
set -e
cd "$(dirname "$0")"

DAY=$(date -u +%y.%m%d)
LAST=0

# Highest release number already used today, from version.txt and git tags.
current=$(cat version.txt 2>/dev/null || true)
if [[ "$current" == "$DAY".* ]]; then
    LAST=$((10#${current##*.}))
fi
if git rev-parse --git-dir >/dev/null 2>&1; then
    while read -r tag; do
        n=$((10#${tag##*.}))
        (( n > LAST )) && LAST=$n
    done < <(git tag -l "v$DAY.*")
fi

NEXT=$(printf "%s.%02d" "$DAY" $((LAST + 1)))

if [[ "$1" == "--print" ]]; then
    echo "$NEXT"
    exit 0
fi
if [[ "$1" == "--set" ]]; then
    [[ "$2" =~ ^[0-9]{2}\.[0-9]{4}\.[0-9]{2}$ ]] || { echo "version must look like YY.MMDD.## (got '$2')"; exit 1; }
    NEXT="$2"
fi

IFS=. read -r YY MMDD NN <<< "$NEXT"
CARGO="$((10#$YY)).$((10#${MMDD:0:2})).$((10#${MMDD:2:2}))+$NN"
CODE="${YY}${MMDD}${NN}"
CODE=$((10#$CODE))

echo "$NEXT" > version.txt
sed -i "0,/^version = \".*\"/s//version = \"$CARGO\"/" Cargo.toml
sed -i "s/versionName \".*\"/versionName \"$NEXT\"/" app/build.gradle
sed -i "s/versionCode [0-9]*/versionCode $CODE/" app/build.gradle
# Keep the lock file's own entry in step so --locked builds keep working.
if [ -f Cargo.lock ]; then
    sed -i '/^name = "holodeck"$/{n;s/^version = ".*"/version = "'"$CARGO"'"/}' Cargo.lock
fi

echo "Version: ${current:-none} -> $NEXT (Cargo $CARGO, android versionCode $CODE)"
