#!/usr/bin/env bash

#set -x
#set -e

# Let the user pick which game folder from ../games instead of entering 1 or 2.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAMES_DIR="$SCRIPT_DIR/../games"

if [ ! -d "$GAMES_DIR" ]; then
    echo "Games directory not found: $GAMES_DIR"
    exit 1
fi

# Build a list of subdirectories
mapfile -t games < <(find "$GAMES_DIR" -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | sort)
if [ ${#games[@]} -eq 0 ]; then
    echo "No game folders found in $GAMES_DIR"
    exit 1
fi

echo "Select a game folder to build from:" 
for i in "${!games[@]}"; do
    idx=$((i+1))
    echo "  $idx) ${games[$i]}"
done

read -r -p "Enter selection number: " sel
if ! [[ "$sel" =~ ^[0-9]+$ ]] || [ "$sel" -lt 1 ] || [ "$sel" -gt ${#games[@]} ]; then
    echo "Invalid selection"
    exit 1
fi

GAME_NAME="${games[$((sel-1))]}"
GAME_DIR="$GAMES_DIR/$GAME_NAME"

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

# Parse canonical version from file/version.h (required). If missing, abort.
VERSION_HEADER="$SCRIPT_DIR/../../file/version.h"
if [ ! -f "$VERSION_HEADER" ]; then
    echo "Version header not found: $VERSION_HEADER"
    exit 1
fi

VERSION_FULL=$(grep -E '^\s*#\s*define\s+VERSION_NUMBER' "$VERSION_HEADER" | sed -E 's/.*"([^"]+)".*/\1/')
if [ -z "$VERSION_FULL" ]; then
    echo "Could not parse VERSION_NUMBER from $VERSION_HEADER"
    exit 1
fi

VERSIONNAME="$VERSION_FULL"
VERSION_MAJOR=$(echo "$VERSION_FULL" | cut -d. -f1)
version_number="$VERSION_MAJOR"

echo "Using VERSION_NUMBER=\"$VERSIONNAME\" (major: $version_number) from $VERSION_HEADER"

# If the game folder name encodes a version, warn if it mismatches the header
if [[ "$GAME_NAME" =~ ([0-9]+)$ ]]; then
    GAME_MAJOR="${BASH_REMATCH[1]}"
    if [ "$GAME_MAJOR" != "$version_number" ]; then
        echo "Warning: picked game folder '$GAME_NAME' suggests major version $GAME_MAJOR but $VERSION_HEADER reports $version_number"
        read -r -p "Continue with this game folder anyway? [y/N] " _cont
        case $_cont in
            [yY][eE][sS]|[yY]) ;;
            *) echo "Aborting due to version mismatch"; exit 1 ;;
        esac
    fi
fi

export PELYA_ANDROID_SDK=/mnt/SamsungEXT4/development/SDK/Android/commandergenius
export ANDROID_HOME=/mnt/SamsungEXT4/development/SDK/Android/SDK
export ANDROID_NDK_HOME=/mnt/SamsungEXT4/development/SDK/Android/android-ndk-r29
#export ANDROID_NDK_HOME=/mnt/SamsungEXT4/development/SDK/Android/android-ndk-r25c
# add NDK to path
#export PATH=$PATH:$PELYA_ANDROID_SDK:$ANDROID_NDK_HOME/build:$ANDROID_HOME/build-tools/29.0.3
export PATH=$PATH:$PELYA_ANDROID_SDK:$ANDROID_NDK_HOME/build:$ANDROID_HOME/build-tools/36.1.0


read -r -p "Did you remember to update data version to $VERSIONNAME? [y/N] " response
case $response in
	[yY][eE][sS]|[yY])
		ROCKDROIDDIR=`pwd`
		export ROCKDROIDDIR

		#copy icon (from selected game folder)
		cp "$ICON_SRC" /home/iuri/Desenvolvimento/rockbot/icon.png

		#read -p "Press any key to continue..." -n1 -s
		#copy data
		rm -r -f ./Android
		mkdir ./Android
		mkdir ./Android/data
		mkdir ./Android/data/games
		rsync -r --exclude=.svn ../fonts ./Android/data
		rsync -r --exclude=.svn ../shared ./Android/data
		rsync -r --exclude=.svn "$GAME_DIR" ./Android/data/games
		mkdir ./Android/data/games/$GAME_NAME/music/mp3/

		### Convert MOD music to MP3 ###
		for filename in "$GAME_DIR/music/"*; do
			basename=${filename##*/}
			if [[ $basename =~ ".mod" ]] || [[ $basename =~ ".s3m" ]] || [[ $basename =~ ".xm" ]] || [[ $basename =~ ".it" ]]; then
				ffmpeg -i "$filename" -vn -ar 44100 -ac 2 -b:a 48k "./Android/data/games/$GAME_NAME/music/mp3/$basename.mp3"
				#ffmpeg -i "$filename" -vn -ar 22050 -ac 2 -b:a 48k "./Android/data/games/$GAME_NAME/music/mp3/$basename.mp3"
			fi
		done

		#mkdir $PELYA_ANDROID_SDK/project/jni/application/rockbot
		cp -r ./files/android $PELYA_ANDROID_SDK/project/jni/application/rockbot
		cp ./files/android/AndroidAppSettings.cfg $PELYA_ANDROID_SDK/project/jni/application/src/
        cp ./files/android/AndroidAppSettings.cfg $PELYA_ANDROID_SDK/project/jni/application/rockbot/

		#export GRADLE_OPTS="org.gradle.jvmargs=-Xmx2000m -Xms1724m -Xmx5048m"
		rm ./Android/data/games/$GAME_NAME/music/ogg/*
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

		LINEVERSIONSTRING=`echo "$VERSIONNAME" | sed -e 's/\.//g'`
		echo "$LINEVERSIONSTRING"

		if [[ -z "$LINENUMBER" ]]
		then
                    echo "Error getting LINENUMBER"
                    return
		fi
		if [[ -z "$LINENUMBERVERSION" ]]
		then
                    echo "Error getting LINENUMBERVERSION"
                    return
		fi
		if [[ -z "$LINENUMBERAPPNAME" ]]
		then
                    echo "Error getting LINENUMBERAPPNAME"
                    return
		fi
		if [[ -z "$LINENUMBERFULLNAME" ]]
		then
                    echo "Error getting LINENUMBERFULLNAME"
                    return
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
		rm $ROCKDROIDDIR/RockBot_Android_$VERSIONNAME.apk
		pwd
		cp ./project/app/build/outputs/apk/release/app-release-unsigned.apk $ROCKDROIDDIR/TEMP_RockBot_Android_$VERSIONNAME.apk
		# remove as assinaturas do Android (caso haja alguma, por engano)
		zip -d $ROCKDROIDDIR/TEMP_RockBot_Android_$VERSIONNAME.apk META-INF/*
		# assina e realinha o APK
		zipalign -v 4 $ROCKDROIDDIR/TEMP_RockBot_Android_$VERSIONNAME.apk $ROCKDROIDDIR/RockBot_Android_$VERSIONNAME.apk
		apksigner sign --ks ~/.android/my-release-key.keystore --v1-signing-enabled true --v2-signing-enabled true $ROCKDROIDDIR/RockBot_Android_$VERSIONNAME.apk
		#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore $ROCKDROIDDIR/TEMP_RockBot_Android_$VERSIONNAME.apk alias_name
		# copy mappings.txt so we can use for later debugging
		#cp ./project/bin/proguard/mapping.txt $ROCKDROIDDIR/Tmappings_$VERSIONNAME.txt
		cp ./project/app/build/outputs/mapping/release/mapping.txt $ROCKDROIDDIR/Tmappings_$VERSIONNAME.txt
		;;
	*)
		echo "Aborted."
		;;
esac

#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore ./Rockdroid_Android_1.0.2.apk alias_name
#zipalign [-f] [-v] <alignment> infile.apk outfile.apk
