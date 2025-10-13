#!/bin/bash

if test "$#" -eq 1; then
  REPO_ROOT=$1
else
  echo "No command line arg sent for repo root, assuming . (repository root)"
  REPO_ROOT="."
fi

if test -f "$REPO_ROOT"/scripts/dev/license-check.py; then
  echo "Found the license check file, command configuration seems correct, running..."
else
  echo "Could not find the license check file at: $REPO_ROOT/scripts/dev/license-check.py, bad command call, aborting..."
  exit 2
fi

EXIT_STATUS=0
# would be really nice to create a minimal library for our own testing to unify all the common stuff
# like walking directories, etc.
echo "Running Custom-Check: license-check"; python3 "$REPO_ROOT"/scripts/dev/license-check.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_stray_fields_in_idd"; python3 "$REPO_ROOT"/scripts/dev/check_stray_fields_in_idd.py || EXIT_STATUS=$?
echo "Running Custom-Check: verify_idfs_in_cmake"; python3 "$REPO_ROOT"/scripts/dev/verify_idfs_in_cmake.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_non_utf8"; python3 "$REPO_ROOT"/scripts/dev/check_non_utf8.py || EXIT_STATUS=$?
echo "Running Custom-Check: validate_idd_units"; python3 "$REPO_ROOT"/scripts/dev/validate_idd_units.py || EXIT_STATUS=$?
echo "Running Custom-Check: find_byref_bool_override"; python3 "$REPO_ROOT"/scripts/dev/find_byref_bool_override.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_tabs"; python3 "$REPO_ROOT"/scripts/dev/check_for_tabs.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_bom_in_idfs"; python3 "$REPO_ROOT"/scripts/dev/check_for_bom_in_idfs.py || EXIT_STATUS=$?
echo "Running Custom-Check: verify_cmake_dirs"; python3 "$REPO_ROOT"/scripts/dev/verify_cmake_dirs.py || EXIT_STATUS=$?
echo "Running Custom-Check: find_included_cc_files"; python3 "$REPO_ROOT"/scripts/dev/find_included_cc_files.py || EXIT_STATUS=$?
echo "Running Custom-Check: analyze_state"; python3 "$REPO_ROOT"/scripts/dev/analyze_state.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_c_style_comments"; python3 "$REPO_ROOT"/scripts/dev/check_for_c_style_comments.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_constexpr"; python3 "$REPO_ROOT"/scripts/dev/check_constexpr.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_switch_case_parentheses"; python3 "$REPO_ROOT"/scripts/dev/check_for_switch_case_parentheses.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_malformed_enums"; python3 "$REPO_ROOT"/scripts/dev/check_for_malformed_enums.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_format_strings"; python3 "$REPO_ROOT"/scripts/dev/check_format_strings.py || EXIT_STATUS=$?
echo "Running Custom-Check: check_for_enum_scope_usage"; python3 "$REPO_ROOT"/scripts/dev/check_for_enum_scope_usage.py || EXIT_STATUS=$?

if [ "$EXIT_STATUS" -eq "0" ]; then
  echo "Custom Check Success!";
else
  echo "Custom Check Failed!"
fi
exit $EXIT_STATUS
