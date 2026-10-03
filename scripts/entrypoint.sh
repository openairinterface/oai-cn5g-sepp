#!/bin/bash
# SPDX-License-Identifier: MIT

set -euo pipefail

# The configuration given to the container (often a bind-mounted file) is never
# modified: the interfaces detected at start-up are written to a runtime copy,
# and the SEPP is started with that copy.
CONFIG_FILE="${SEPP_CONFIG_FILE:-/openair-sepp/etc/config.yaml}"
RUNTIME_CONFIG="${SEPP_RUNTIME_CONFIG:-/openair-sepp/etc/config.runtime.yaml}"
# The N32 interface towards the peer SEPP (NBI; 3GPP TS 29.573 clause 4.2,
# 3GPP TS 23.501 clause 6.2.17) is the one with an address in this prefix,
# unless SEPP_NBI_INTERFACE names it directly.
NBI_IP_PREFIX="${SEPP_NBI_IP_PREFIX:-192.168.72.}"

# Print the first interface with an IPv4 address starting with $1
interface_with_ip_prefix() {
    ifconfig | awk -v prefix="$1" '
        /^[^ \t]/ { iface = $1; sub(/:$/, "", iface) }
        $1 == "inet" && index($2, prefix) == 1 { print iface; exit }'
}

cp "$CONFIG_FILE" "$RUNTIME_CONFIG"

echo "[+] Detecting network interfaces..."
NBI_IFACE="${SEPP_NBI_INTERFACE:-$(interface_with_ip_prefix "$NBI_IP_PREFIX")}"
if [ -z "$NBI_IFACE" ]; then
    echo "[-] Warning: no interface with a ${NBI_IP_PREFIX} address; using the configured interfaces"
else
    # The SBI interface is the other ethX interface, unless set directly
    SBI_IFACE="${SEPP_SBI_INTERFACE:-$(ifconfig | awk '/^eth[0-9]+/ { sub(/:$/, "", $1); print $1 }' | grep -vx "$NBI_IFACE" | head -n1)}"
    echo "[+] Target NBI Interface: $NBI_IFACE"
    echo "[+] Target SBI Interface: $SBI_IFACE"
    NBI_IFACE="$NBI_IFACE" SBI_IFACE="$SBI_IFACE" yq -i '
        .nfs.sepp.nbi.interface_name = strenv(NBI_IFACE) |
        .nfs.sepp.sbi.interface_name = strenv(SBI_IFACE)' "$RUNTIME_CONFIG"
    echo "[+] Configuration updated successfully: $RUNTIME_CONFIG"
fi

# Execute the main process passed via CMD, with the runtime configuration
args=()
for arg in "$@"; do
    if [ "$arg" = "$CONFIG_FILE" ]; then args+=("$RUNTIME_CONFIG"); else args+=("$arg"); fi
done
exec "${args[@]}"
