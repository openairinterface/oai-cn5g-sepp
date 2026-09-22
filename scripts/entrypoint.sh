#!/bin/bash
set -e

CONFIG_FILE="/openair-sepp/etc/config.yaml"
TMP_FILE="/tmp/config.yaml.tmp"

echo "[+] Detecting network interfaces..."

# Detect interface with 192.168.72.x IP
NBI_IFACE=$(ifconfig | grep -B1 'inet 192\.168\.72\.' | head -n1 | awk '{print $1}' | tr -d ':')

if [ -z "$NBI_IFACE" ]; then
    echo "[-] Warning: 192.168.72.x IP not found. Defaulting NBI=eth1, SBI=eth0"
    NBI_IFACE="eth1"
    SBI_IFACE="eth0"
else
    if [ "$NBI_IFACE" == "eth1" ]; then
        SBI_IFACE="eth0"
    elif [ "$NBI_IFACE" == "eth0" ]; then
        SBI_IFACE="eth1"
    else
        # Fallback to get the other eth interface
        SBI_IFACE=$(ifconfig | grep -E '^eth[0-9]+' | awk '{print $1}' | tr -d ':' | grep -v "^${NBI_IFACE}$" | head -n1)
    fi
fi

echo "[+] Target NBI Interface: $NBI_IFACE"
echo "[+] Target SBI Interface: $SBI_IFACE"

# Precise line-by-line replacement using AWK
awk -v nbi="$NBI_IFACE" -v sbi="$SBI_IFACE" '
BEGIN { in_sepp=0; section="" }

# Track when we are inside the nfs -> sepp block
$1 == "sepp:" && NR < 100 { in_sepp=1 }
in_sepp && $1 == "database:" { in_sepp=0 }

# Track section inside sepp
in_sepp && $1 == "sbi:" { section="sbi" }
in_sepp && $1 == "nbi:" { section="nbi" }

# Perform precise replacement
in_sepp && section == "sbi" && $1 == "interface_name:" {
    sub(/interface_name: .*/, "interface_name: " sbi)
}
in_sepp && section == "nbi" && $1 == "interface_name:" {
    sub(/interface_name: .*/, "interface_name: " nbi)
}

{ print }
' "$CONFIG_FILE" > "$TMP_FILE"

# Copy content back without breaking file inodes (fixes "Device or resource busy")
cat "$TMP_FILE" > "$CONFIG_FILE"
rm -f "$TMP_FILE"

echo "[+] Configuration updated successfully!"

# Execute main process passed via CMD
exec "$@"