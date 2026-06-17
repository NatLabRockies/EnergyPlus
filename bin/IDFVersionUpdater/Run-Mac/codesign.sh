#!/bin/bash

APP_NAME=IDFVersionUpdater
IDENTIFIER="org.nrel.EnergyPlus.$APP_NAME"

function ep_codesign() {
  codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime "$1"
}

function ep_notarize() {
  xcrun notarytool submit --keychain-profile "EnergyPlus" --wait "$1"
}

ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/XojoFramework.framework

#cd IDFVersionUpdater.app/Contents/Frameworks/
#zip -r ./XojoFramework.framework.zip ./XojoFramework.framework
#ep_notarize ./XojoFramework.framework.zip
#xcrun stapler staple ./XojoFramework.framework
#xcrun stapler validate ./XojoFramework.framework
#rm -Rf ./XojoFramework.framework.zip
#cd ../../..

ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/Shell.dylib
ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/AppearancePakCocoa.dylib
ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/Crypto.dylib
ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/InternetEncodings.dylib
ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/Shell.dylib
ep_codesign ./IDFVersionUpdater.app/Contents/Frameworks/libGzip.dylib

ep_codesign IDFVersionUpdater.app/Contents/MacOS/IDFVersionUpdater

codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    $APP_NAME.app

zip -r ./$APP_NAME.zip ./$APP_NAME.app
ep_notarize ./$APP_NAME.zip
xcrun stapler staple ./$APP_NAME.app

xcrun stapler validate ./$APP_NAME.app
spctl -vvvv --assess ./$APP_NAME.app
rm -Rf ./$APP_NAME.zip
