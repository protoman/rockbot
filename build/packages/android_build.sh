#!/usr/bin/env bash

#set -x
#set -e

# Native Android build (commandergenius / NDK / Gradle on this machine, no Docker).
# Usage: ./android_build.sh [1|2] [--no-sign]     (asks which game if omitted)
# Signing: the release APK is signed with ~/.android/my-release-key.keystore when
# ANDROID_KEYSTORE_PASS is set (or when run from a terminal, which prompts for the password);
# otherwise, or with --no-sign, the aligned APK is left unsigned.
# Output: build/packages/Rockbot<N>_Android_<version>.apk
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

NO_SIGN=false
game=""
for arg in "$@"; do
    case "$arg" in
        1|2) game="$arg" ;;
        --no-sign) NO_SIGN=true ;;
        *) echo "Usage: $0 [1|2] [--no-sign]" >&2; exit 1 ;;
    esac
done
choose_game "$game" || exit 1

GAME_NAME="RockDroid$version_number"
GAME_DIR="$SCRIPT_DIR/../games/$GAME_NAME"
if [ ! -d "$GAME_DIR" ]; then
    echo "Game folder not found: $GAME_DIR"
    exit 1
fi

# Expect icon and logo in images/ within the chosen game folder
ICON_SRC="$GAME_DIR/images/icon_android.png"
LOGO_SRC="$GAME_DIR/images/logo_android.png"

if [ ! -f "$ICON_SRC" ]; then
    echo "Icon not found: $ICON_SRC"
    exit 1
fi
if [ ! -f "$LOGO_SRC" ]; then
    echo "Logo not found: $LOGO_SRC"
    exit 1
fi

# Android SDK folder (override with ANDROID_BASE): the first one that has the commandergenius project.
if [ -z "${ANDROID_BASE:-}" ]; then
    for d in /mnt/SamsungEXT4/development/SDK/Android /mnt/ExtraData/Devel/SDK/Android; do
        if [ -d "$d/commandergenius" ]; then ANDROID_BASE="$d"; break; fi
    done
fi
if [ -z "${ANDROID_BASE:-}" ] || [ ! -d "$ANDROID_BASE/commandergenius" ]; then
    echo "Android SDK folder not found (set ANDROID_BASE to the folder with commandergenius/, SDK/ and android-ndk-r29/)" >&2
    exit 1
fi
export PELYA_ANDROID_SDK="$ANDROID_BASE/commandergenius"

# Patches for the commandergenius tree that are newer than the Docker image / the pinned commit
# (ports/android/overlay/0003-*.patch and up). Applied once; a patch that is already there is skipped.
for overlay_patch in "$SCRIPT_DIR/../../ports/android/overlay/"0003-*.patch; do
    [ -f "$overlay_patch" ] || continue
    if git -C "$PELYA_ANDROID_SDK" apply --check "$overlay_patch" 2>/dev/null; then
        git -C "$PELYA_ANDROID_SDK" apply "$overlay_patch" && echo "Applied $(basename "$overlay_patch") to $PELYA_ANDROID_SDK"
    fi
done
export ANDROID_HOME="$ANDROID_BASE/SDK"
export ANDROID_NDK_HOME="$ANDROID_BASE/android-ndk-r29"
# add NDK to path
#export PATH=$PATH:$PELYA_ANDROID_SDK:$ANDROID_NDK_HOME/build:$ANDROID_HOME/build-tools/29.0.3
export PATH=$PATH:$PELYA_ANDROID_SDK:$ANDROID_NDK_HOME/build:$ANDROID_HOME/build-tools/36.1.0


response=y   # the data version is the version file, bumped by release.sh
case $response in
	[yY][eE][sS]|[yY])
		ROCKDROIDDIR=`pwd`
		export ROCKDROIDDIR

		#copy icon (from selected game folder)
		cp "$ICON_SRC" "$SCRIPT_DIR/../../icon.png"

		#read -p "Press any key to continue..." -n1 -s
		#copy data
		rm -r -f ./Android
		mkdir ./Android
		mkdir ./Android/data
		mkdir ./Android/data/games
		rsync -r --exclude=.svn ../fonts ./Android/data
		rsync -r --exclude=.svn ../shared ./Android/data
		rsync -r --exclude=.svn "$GAME_DIR" ./Android/data/games

		#mkdir $PELYA_ANDROID_SDK/project/jni/application/rockbot
		cp -r ./files/android $PELYA_ANDROID_SDK/project/jni/application/rockbot
		cp ./files/android/AndroidAppSettings.cfg $PELYA_ANDROID_SDK/project/jni/application/src/
        cp ./files/android/AndroidAppSettings.cfg $PELYA_ANDROID_SDK/project/jni/application/rockbot/

		#export GRADLE_OPTS="org.gradle.jvmargs=-Xmx2000m -Xms1724m -Xmx5048m"
		rm ./Android/data/game*.sav
		rm ./Android/data/config_v*.sav
		cd ./Android/data
		zip -r ../data_$VERSIONNAME.zip ./fonts ./games ./shared
		cd ..

		if [ ! -d "$PELYA_ANDROID_SDK/project/jni/application/rockbot/AndroidData/" ]; then
			mkdir $PELYA_ANDROID_SDK/project/jni/application/rockbot/AndroidData/
		fi
		rm $PELYA_ANDROID_SDK/project/jni/application/rockbot/AndroidData/*.zip
		cp ./data_$VERSIONNAME.zip $PELYA_ANDROID_SDK/project/jni/application/rockbot/AndroidData/
		#copy splashscreen (from selected game folder)
		cp "$LOGO_SRC" $PELYA_ANDROID_SDK/project/jni/application/src/AndroidData/logo.png

		cd $PELYA_ANDROID_SDK

		LINENUMBER=`grep -n "AppDataDownloadUrl=" AndroidAppSettings.cfg | cut -f1 -d:`
		LINENUMBERVERSION=`grep -n "AppVersionName=" AndroidAppSettings.cfg | cut -f1 -d:`
		LINENUMBERAPPNAME=`grep -n "AppName=" AndroidAppSettings.cfg | cut -f1 -d:`
		LINENUMBERFULLNAME=`grep -n "AppFullName=" AndroidAppSettings.cfg | cut -f1 -d:`
		LINEVERSIONNUMBER=`grep -n "AppVersionCode=" AndroidAppSettings.cfg | cut -f1 -d:`

		LINEVERSIONSTRING="$(version_code)" || exit 1
		echo "$LINEVERSIONSTRING"

		if [[ -z "$LINENUMBER" ]]
		then
                    echo "Error getting LINENUMBER"
                    exit 1
		fi
		if [[ -z "$LINENUMBERVERSION" ]]
		then
                    echo "Error getting LINENUMBERVERSION"
                    exit 1
		fi
		if [[ -z "$LINENUMBERAPPNAME" ]]
		then
                    echo "Error getting LINENUMBERAPPNAME"
                    exit 1
		fi
		if [[ -z "$LINENUMBERFULLNAME" ]]
		then
                    echo "Error getting LINENUMBERFULLNAME"
                    exit 1
		fi

		sed $LINENUMBER'c\'"AppDataDownloadUrl=\"!Game Data|data_$VERSIONNAME.zip\"" ./AndroidAppSettings.cfg > ./AndroidAppSettings.cfg.temp1
		sed $LINENUMBERVERSION'c\'"AppVersionName=\"$VERSIONNAME\"" AndroidAppSettings.cfg.temp1 > AndroidAppSettings.cfg.temp2
		sed $LINENUMBERAPPNAME'c\'"AppName=\"Rockbot $version_number\"" AndroidAppSettings.cfg.temp2 > AndroidAppSettings.cfg.temp3
		if [ "$version_number" == "1" ]; then
			sed $LINENUMBERFULLNAME'c\'"AppFullName=net.upperland.rockdroid" AndroidAppSettings.cfg.temp3 > AndroidAppSettings.cfg.temp4
		else
			sed $LINENUMBERFULLNAME'c\'"AppFullName=net.upperland.rockdroid$version_number" AndroidAppSettings.cfg.temp3 > AndroidAppSettings.cfg.temp4
		fi
		sed $LINEVERSIONNUMBER'c\'"AppVersionCode=$LINEVERSIONSTRING" AndroidAppSettings.cfg.temp4 > AndroidAppSettings.cfg.new
		# keep the native libs compressed in the APK (the commandergenius default stores them uncompressed: ~170 MB instead of ~60 MB)
		grep -q '^ExtractNativeLibs=' AndroidAppSettings.cfg.new || echo 'ExtractNativeLibs=y' >> AndroidAppSettings.cfg.new
		sed -i 's/^ExtractNativeLibs=.*/ExtractNativeLibs=y/' AndroidAppSettings.cfg.new


        cd $PELYA_ANDROID_SDK
#       pwd
#		read -p "Press any key to continue... " -n1 -s

		cp AndroidAppSettings.cfg AndroidAppSettings.cfg.old
		cp AndroidAppSettings.cfg.new AndroidAppSettings.cfg

		./changeAppSettings.sh rockbot -a

		# build debug and copy library so we can track
		./build.sh rockbot debug

		# build release
		./build.sh rockbot release
		APK_NAME="Rockbot${version_number}_Android_$VERSIONNAME.apk"
		rm -f "$ROCKDROIDDIR/$APK_NAME"
		cp ./project/app/build/outputs/apk/release/app-release-unsigned.apk "$ROCKDROIDDIR/TEMP_$APK_NAME"
		# remove as assinaturas do Android (caso haja alguma, por engano)
		zip -d "$ROCKDROIDDIR/TEMP_$APK_NAME" 'META-INF/*'
		# realinha o APK
		zipalign -f -v 4 "$ROCKDROIDDIR/TEMP_$APK_NAME" "$ROCKDROIDDIR/$APK_NAME"
		rm -f "$ROCKDROIDDIR/TEMP_$APK_NAME"
		# assina o APK (v1 + v2)
		KEYSTORE="${ANDROID_KEYSTORE:-$HOME/.android/my-release-key.keystore}"
		if $NO_SIGN || [ ! -f "$KEYSTORE" ]; then
			echo "Leaving the APK unsigned (--no-sign or no keystore at $KEYSTORE)"
		elif [ -n "${ANDROID_KEYSTORE_PASS:-}" ]; then
			apksigner sign --ks "$KEYSTORE" --ks-pass env:ANDROID_KEYSTORE_PASS --v1-signing-enabled true --v2-signing-enabled true "$ROCKDROIDDIR/$APK_NAME"
		elif [ -t 0 ]; then
			apksigner sign --ks "$KEYSTORE" --v1-signing-enabled true --v2-signing-enabled true "$ROCKDROIDDIR/$APK_NAME"
		else
			echo "No ANDROID_KEYSTORE_PASS and no terminal to ask the keystore password: APK left unsigned"
		fi
		if [ ! -s "$ROCKDROIDDIR/$APK_NAME" ]; then
			echo "ERROR: the Android build did not produce $APK_NAME (see the compiler errors above)" >&2
			exit 1
		fi
		# copy mappings.txt so we can use for later debugging
		cp ./project/app/build/outputs/mapping/release/mapping.txt "$ROCKDROIDDIR/Rockbot${version_number}_mappings_$VERSIONNAME.txt"
		echo "Package: $ROCKDROIDDIR/$APK_NAME"
		;;
	*)
		echo "Aborted."
		;;
esac

#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore ./Rockdroid_Android_1.0.2.apk alias_name
#zipalign [-f] [-v] <alignment> infile.apk outfile.apk
