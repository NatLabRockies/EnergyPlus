#!/usr/bin/env bash

set -euo pipefail

readonly PANDOC_VERSION="3.11"
readonly PANDOC_SHA256="37edb3bbcf722f921a009941bf5874e2e0c09263226c9b4a2d980788cb062ab6"
readonly INSTALL_ROOT="${1:?usage: install_pandoc.sh INSTALL_ROOT}"
readonly ARCHIVE_NAME="pandoc-${PANDOC_VERSION}-linux-amd64.tar.gz"
readonly ARCHIVE_PATH="${INSTALL_ROOT}/${ARCHIVE_NAME}"
readonly DOWNLOAD_URL="https://github.com/jgm/pandoc/releases/download/${PANDOC_VERSION}/${ARCHIVE_NAME}"

mkdir -p "${INSTALL_ROOT}"
curl --fail --location --silent --show-error --retry 3 \
  --output "${ARCHIVE_PATH}" \
  "${DOWNLOAD_URL}"

printf '%s  %s\n' "${PANDOC_SHA256}" "${ARCHIVE_PATH}" | sha256sum --check --status
tar --extract --gzip --file "${ARCHIVE_PATH}" --directory "${INSTALL_ROOT}"

"${INSTALL_ROOT}/pandoc-${PANDOC_VERSION}/bin/pandoc" --version
