#!/bin/bash
# SPDX-License-Identifier: MIT

set -eo pipefail

STATUS=0
# The entrypoint starts the SEPP with a runtime copy of the configuration
CONFIG_FILE="${SEPP_RUNTIME_CONFIG:-/openair-sepp/etc/config.runtime.yaml}"
[ -f "$CONFIG_FILE" ] || CONFIG_FILE="${SEPP_CONFIG_FILE:-/openair-sepp/etc/config.yaml}"
SEPP_INTERFACE_NAME_FOR_SBI=$(yq '.nfs.sepp.sbi.interface_name' "$CONFIG_FILE")
SEPP_INTERFACE_PORT_FOR_SBI=$(yq '.nfs.sepp.sbi.port' "$CONFIG_FILE")

SEPP_IP_SBI_INTERFACE=$(ifconfig $SEPP_INTERFACE_NAME_FOR_SBI | grep inet | grep -v inet6 | awk {'print $2'})
#Check if entrypoint properly configured the conf file and no parameter is unset(optional)
SEPP_SBI_PORT_STATUS=$(netstat -tnpl | grep -o "$SEPP_IP_SBI_INTERFACE:$SEPP_INTERFACE_PORT_FOR_SBI")

if [[ -z $SEPP_SBI_PORT_STATUS ]]; then
	STATUS=-1
	echo "Healthcheck error: UNHEALTHY SBI TCP/HTTP port $SEPP_INTERFACE_PORT_FOR_SBI is not listening."
fi

exit $STATUS
