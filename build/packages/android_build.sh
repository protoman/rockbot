#!/bin/bash

#set -x

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

demoName=""
demoFullName=""

if [ "$version_number" == "2" ]
then
    read -r -p "Build demo version (d) or full (f)? " demo_mode
    if [ "$demo_mode" == "d" ]
    then
        demoName="Demo"
        demoFullName="demo"
    fi
fi

VERSIONNAME=`cat version_name_v$version_number.txt`


read -r -p "Did you remember to update data version in version_name_v$version_number.txt? [y/N] " response
case $response in
	[yY][eE][sS]|[yY]) 
		ROCKBOTDIR=`pwd`
		export ROCKBOTDIR
		#copy icon
		cp ../packages/files/android/icon_v$version_number.png ~/Programas/android-studio/sdk/rockbot_build/src/icon.png
		#read -p "Press any key to continue..." -n1 -s
		#copy data
		rm -r -f ./Android
		mkdir ./Android
		mkdir ./Android/data
		mkdir ./Android/data/games
		rsync -r --exclude=.svn ../fonts ./Android/data
		rsync -r --exclude=.svn ../games/Rockbot$version_number ./Android/data/games
		rsync -r --exclude=.svn ../shared ./Android/data
		rm ./Android/data/game*.sav
		rm ./Android/data/config_v*.sav
		cd ./Android/data
		zip -r ../data_$VERSIONNAME$demoName.zip ./fonts ./games ./shared
		cd ..
		rm $ANDROIDSDK/rockbot_build/project/jni/application/src/AndroidData/*.zip
		cp ./data_$VERSIONNAME$demoName.zip $ANDROIDSDK/rockbot_build/project/jni/application/src/AndroidData/
		rm $ANDROIDSDK/rockbot_build/project/jni/application/src/libapplication.so
		rm $ANDROIDSDK/rockbot_build/project/obj/local/armeabi/libapplication.so
		rm $ANDROIDSDK/rockbot_build/project/libs/armeabi/libapplication.so
		cp ../../libapplication.so ../../../Android/project/jni/application/src/libapplication.so
		cd $ANDROIDSDK/rockbot_build

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
		
		sed $LINENUMBER'c\'"AppDataDownloadUrl=\"!Game Data|data_$VERSIONNAME$demoName.zip\"" AndroidAppSettings.cfg > AndroidAppSettings.cfg.temp1
		sed $LINENUMBERVERSION'c\'"AppVersionName=\"$VERSIONNAME\"" AndroidAppSettings.cfg.temp1 > AndroidAppSettings.cfg.temp2
		sed $LINENUMBERAPPNAME'c\'"AppName=\"Rockbot$version_number$demoName\"" AndroidAppSettings.cfg.temp2 > AndroidAppSettings.cfg.temp3
		if [ "$version_number" == "1" ]; then
                    sed $LINENUMBERFULLNAME'c\'"AppFullName=net.upperland.rockbot" AndroidAppSettings.cfg.temp3 > AndroidAppSettings.cfg.temp4
                else
                    sed $LINENUMBERFULLNAME'c\'"AppFullName=net.upperland.rockbot$version_number$demoFullName" AndroidAppSettings.cfg.temp3 > AndroidAppSettings.cfg.temp4
                fi
		
		sed $LINEVERSIONNUMBER'c\'"AppVersionCode=$LINEVERSIONSTRING" AndroidAppSettings.cfg.temp4 > AndroidAppSettings.cfg.new
		
		
		cp AndroidAppSettings.cfg AndroidAppSettings.cfg.old
		cp AndroidAppSettings.cfg.new AndroidAppSettings.cfg
		# build debug and copy library so we can track
		sh ./build.sh rockbot debug
		cp ./project/jni/application/rockbot/libapplication-armeabi.so $ROCKBOTDIR/libapplication-armeabi_$VERSIONNAME$demoName.so
		
		# build release
		sh ./build.sh rockbot release
		rm $ROCKBOTDIR/Rockbot_Android_$VERSIONNAME$demoName.apk
		cp ./project/bin/MainActivity-release-unsigned.apk $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME$demoName.apk
		# remove as assinaturas do Android (caso haja alguma, por engano)
		zip -d $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME$demoName.apk META-INF/*
		# assina e realinha o APK
		/opt/java/jdk1.8.0_121/bin/jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore  $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME$demoName.apk alias_name
		/home/iuri/Programas/android-studio/sdk/build-tools/23.0.3/zipalign -v 4 $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME$demoName.apk $ROCKBOTDIR/Rockbot_Android_$VERSIONNAME$demoName.apk
		# copy mappings.txt so we can use for later debugging
		cp ./project/bin/proguard/mapping.txt $ROCKBOTDIR/Tmappings_$VERSIONNAME$demoName.txt
		;;
	*)
		echo "Aborted."
		;;
esac

#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore ./Rockbot_Android_1.0.2.apk alias_name
#zipalign [-f] [-v] <alignment> infile.apk outfile.apk