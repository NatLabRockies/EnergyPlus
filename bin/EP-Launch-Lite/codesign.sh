#!/bin/bash

APP_NAME=EP-Launch-Lite
IDENTIFIER="org.nrel.EnergyPlus.$APP_NAME"

function ep_codesign() {
  codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml "$1"
}

function ep_notarize() {
  xcrun notarytool submit --keychain-profile "EnergyPlus" --wait "$1"
}


echo "Dealing wiht Python.framework"

find EP-Launch-Lite.app/Contents/Resources/ -name "*.so" | xargs codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml


codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml \
    EP-Launch-Lite.app/Contents/Frameworks/Python.framework/Versions/2.7/Python

codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --deep \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml \
    EP-Launch-Lite.app/Contents/Frameworks/Python.framework

echo "DYlibs"

find EP-Launch-Lite.app/Contents/ -name "*.dylib" | xargs codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml



cd EP-Launch-Lite.app/Contents/Frameworks
zip -r ./Python.framework.zip ./Python.framework
ep_notarize ./Python.framework.zip
# xcrun stapler staple ./Python.framework
# xcrun stapler validate ./Python.framework
rm -Rf ./Python.framework.zip
cd ../../..

echo "Dealing with EP-Launch-Lite itself"
ep_codesign EP-Launch-Lite.app/Contents/MacOS/EP-Launch-Lite

# Docs say deep should not be used, but whatever
codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --deep \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --options runtime \
    --entitlements entitlements.xml \
    $APP_NAME.app

zip -r ./$APP_NAME.zip ./$APP_NAME.app
ep_notarize ./$APP_NAME.zip
xcrun stapler staple ./$APP_NAME.app

xcrun stapler validate -v ./$APP_NAME.app
spctl -vvvv --assess ./$APP_NAME.app
rm -Rf ./$APP_NAME.zip
