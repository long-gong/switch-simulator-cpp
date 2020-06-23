#!/usr/bin/env bash
set -e

install_dir=$(pwd)/build_dependencies

if [ ! -d "${install_dir}" ]; then
  mkdir -p "${install_dir}"
fi

echo "Install dependencies: lemon c++ to ${install_dir}"
cd /tmp
rm -rf lemon-1.3.1.tar.gz lemon-1.3.1
wget http://lemon.cs.elte.hu/pub/sources/lemon-1.3.1.tar.gz
tar -zxvf lemon-1.3.1.tar.gz
cd lemon-1.3.1
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${install_dir}
make && make install
cd /tmp
rm -rf lemon-1.3.1.tar.gz lemon-1.3.1
