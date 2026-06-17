#!/bin/bash

function ep_codesign() {
  codesign -vvvv -s "Developer ID Application: National Renewable Energy Laboratory (K7JYVQJL7R)" \
    --force --timestamp \
    --identifier "$IDENTIFIER" \
    --entitlements entitlements.xml \
    --options runtime "$1"
}

function ep_notarize() {
  xcrun notarytool submit --keychain-profile "EnergyPlus" --wait "$1"
}


for APP_NAME in EPLaunch VersionUpdater; do
  echo "Signing $APP_NAME"
  IDENTIFIER="org.nrel.EnergyPlus.$APP_NAME"

  ep_codesign $APP_NAME.app/Contents/MacOS/$APP_NAME
  ep_codesign $APP_NAME.app

  zip -r ./$APP_NAME.zip ./$APP_NAME.app
  ep_notarize ./$APP_NAME.zip
  xcrun stapler staple ./$APP_NAME.app

  xcrun stapler validate ./$APP_NAME.app
  spctl -vvvv --assess ./$APP_NAME.app || true
  rm -Rf ./$APP_NAME.zip
done
