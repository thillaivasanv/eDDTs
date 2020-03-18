# SPDX-License-Identifier: GPL-2.0-or-later
#!/bin/sh
# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2017-2020 Petr Vorel <pvorel@suse.cz>

SERVER=
CLIENT=
CLIENT_EXTRA_OPTS=
CLEANER=
# Program number to register the services to rpcbind
PROGNUMNOSVC=536875000

TST_TESTFUNC=do_test
TST_USAGE=usage
TST_OPTS="c:e:s:"
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_PARSE_ARGS=rpc_parse_args
TST_NEEDS_CMDS="pkill rpcinfo"
. rpc_lib.sh

usage()
{
	cat << EOF
USAGE: $0 [-s sprog] -c clprog [ -e extra ]

Connect to the remote host and start sprog.
Then execute clprog and passing it the remote host value.

-c clprog client program binary
-s sprog  server program binary
-e extra  extra client options
EOF
}

rpc_parse_args()
{
	case "$1" in
		c) CLIENT="$OPTARG" ;;
		e) CLIENT_EXTRA_OPTS="$OPTARG" ;;
		s) SERVER="$OPTARG" ;;
	esac
}

setup()
{
	check_portmap_rpcbind

	if [ -n "$SERVER" ]; then
		CLEANER="rpc_cleaner"
		if echo "$SERVER" | grep -q '^tirpc'; then
			CLEANER="tirpc_cleaner"
		fi
	fi

	[ -n "$CLIENT" ] || tst_brk "client program not set"
}

cleanup()
{
	if [ ! -z "$SERVER" ]; then
		pkill -9 $SERVER > /dev/null 2>&1
		$CLEANER $PROGNUMNOSVC
	fi
}

do_test()
{
	local i

	if [ -n "$SERVER" ]; then
		$SERVER $PROGNUMNOSVC &

		for i in $(seq 1 10); do
			rpcinfo -p localhost | grep -q $PROGNUMNOSVC && break
			[ "$i" -eq 30 ] && tst_brk TBROK "server not registered"
			tst_sleep 100ms
		done
	fi

	EXPECT_RHOST_PASS $CLIENT $(tst_ipaddr) $PROGNUMNOSVC $CLIENT_EXTRA_OPTS
}

tst_run
