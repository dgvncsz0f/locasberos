#!/bin/sh

bin_sed=${bin_sed:-/bin/sed}

major=$1
minor=$2
patch=$3
build=$4

read_version() {
  [ -z "$major" ] && read -p "major: " major
  [ -z "$minor" ] && read -p "minor: " minor
  [ -z "$build" ] && read -p "patch: " patch
  [ -z "$build" ] && read -p "build: " build
  if [ -n "$build" ]
  then
    build="+$build"
  fi
  version="$major.$minor.$patch$build"
}

print_usage() {
  echo "[usage] version.sh [MAJOR [MINOR [PATCH [BUILD]]]]"
}

check_environ() {
  test -z "$major" && {
    print_usage
    echo "major can not be blank" >&2
    exit 1
  }

  test -z "$minor" && {
    print_usage
    echo "minor can not be blank" >&2
    exit 1
  }

  test -z "$patch" && {
    print_usage
    echo "patch can not be blank" >&2
    exit 1
  }

  test -x "$bin_sed" || {
    echo "$bin_sed (sed) program not found or not executable" >&2
    exit 1
  }
}

update_version() {
  echo " updating file: $1"
  $bin_sed -i -r 's/\$version[^\$]*\$/\$version '"$version"'$/' "$1"
  $bin_sed -i -r 's/^version\s*=\s*["'\''][0-9]+\.[0-9]+\.[0-9]+.*$/version = "'"$version"'"/' $1
}

write_hversion() {
  echo " creating file: $1"
  cat <<EOF >"$1"
// vim: et:ts=8:sw=2:sts=2

// Copyright (c) 2012 dgvncsz0f
// Copyright (c) 2012 pothix
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the <ORGANIZATION> nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __LOCASBEROS_VERSION__
#define __LOCASBEROS_VERSION__

#define CASLIB_MAJOR $major

#define CASLIB_MINOR $minor

#define CASLIB_PATCH $patch

#define CASLIB_BUILD "$build"

// The string: ("%d.%d.%d%s", MAJOR, MINOR, PATCH, BUILD)
#define CASLIB_VERSION "$version"

#endif

EOF
}

read_version
check_environ
echo "version: $version"
update_version "README.rst"
update_version "doc/source/conf.py"
write_hversion "src/caslib/version.h"
