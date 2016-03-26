#!/bin/sh
VERSIONNAME=`cat version_name.txt`

read -r -p "Did you remember to update data version and data zip name in AndroidAppSettings.cfg? [y/N] " response
case $response in
	[yY][eE][sS]|[yY]) 
		ROCKBOTDIR=`pwd`
		export ROCKBOTDIR
		rm -r -f ./Android
		mkdir ./Android
		mkdir ./Android/data
		rsync -r --exclude=.svn ../fonts ./Android/data
		rsync -r --exclude=.svn ../games ./Android/data
		rsync -r --exclude=.svn ../shared ./Android/data
		rm ./Android/data/game*.sav
		rm ./Android/data/config_v*.sav
		cd ./Android/data
		zip -r ../data_$VERSIONNAME.zip ./fonts ./games ./shared
		cd ..
		rm $ANDROIDSDK/rockbot_build/project/jni/application/src/AndroidData/*.zip
		cp ./data_$VERSIONNAME.zip $ANDROIDSDK/rockbot_build/project/jni/application/src/AndroidData/
		rm $ANDROIDSDK/rockbot_build/project/jni/application/src/libapplication.so
		rm $ANDROIDSDK/rockbot_build/project/obj/local/armeabi/libapplication.so
		rm $ANDROIDSDK/rockbot_build/project/libs/armeabi/libapplication.so
		cp ../../libapplication.so ../../../Android/project/jni/application/src/libapplication.so
		cd $ANDROIDSDK/rockbot_build
		LINENUMBER=`grep -n "AppDataDownloadUrl" AndroidAppSettings.cfg | cut -f1 -d:`
		sed $LINENUMBER'c\'"AppDataDownloadUrl=\"Game data is about 9 Mb|data_$VERSIONNAME.zip\"" AndroidAppSettings.cfg > AndroidAppSettings.cfg.new
		cp AndroidAppSettings.cfg AndroidAppSettings.cfg.old
		cp AndroidAppSettings.cfg.new AndroidAppSettings.cfg
		sh ./build.sh -s release
		rm $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME.apk
		rm $ROCKBOTDIR/Rockbot_Android_$VERSIONNAME.apk
		cp ./project/bin/MainActivity-release-unsigned.apk $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME.apk
		#remove as assinaturas do Android (porque ele está colocando uma do além)
		zip -d $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME.apk META-INF/*
		#assina e realinha o APK
		/opt/java/jdk1.7.0_71/bin/jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore  $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME.apk alias_name
		zipalign -v 4 $ROCKBOTDIR/TEMP_Rockbot_Android_$VERSIONNAME.apk $ROCKBOTDIR/Rockbot_Android_$VERSIONNAME.apk
		;;
	*)
		echo "Aborted."
		;;
esac

#jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/my-release-key.keystore ./Rockbot_Android_1.0.2.apk alias_name
#zipalign [-f] [-v] <alignment> infile.apk outfile.apk