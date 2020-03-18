#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -ex

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	libtirpc \
	libtirpc-devel \
	pkg-config \
	redhat-lsb-core
