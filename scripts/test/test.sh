#!/usr/bin/env bash
set -euo pipefail

# Script location: <workspace>/oai-cn5g-sepp/scripts/test/
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMPOSE_FILE="$SCRIPT_DIR/docker-compose-basic-nrf-roaming.yaml"

if [ ! -f "$COMPOSE_FILE" ]; then
    echo "Error: Compose file not found at $COMPOSE_FILE"
    exit 1
fi

# Detect whether to use 'docker compose' or legacy 'docker-compose'
if docker compose version >/dev/null 2>&1; then
    DOCKER_COMPOSE="docker compose"
elif command -v docker-compose >/dev/null 2>&1; then
    DOCKER_COMPOSE="docker-compose"
else
    echo "Error: Neither 'docker compose' nor 'docker-compose' was found."
    exit 1
fi

echo "Using compose command: $DOCKER_COMPOSE"

echo "=== Starting Inter-PLMN Roaming Topology ==="
$DOCKER_COMPOSE -f "$COMPOSE_FILE" down -v --remove-orphans || true
$DOCKER_COMPOSE -f "$COMPOSE_FILE" up -d

echo "Waiting for services to initialize..."
sleep 15
$DOCKER_COMPOSE -f "$COMPOSE_FILE" ps

echo "=== Test 1: Discover UDR in PLMN B via SEPP-A ==="
RESPONSE_UDR=$(curl --silent --show-error --http2-prior-knowledge -w "\nHTTP_STATUS:%{http_code}\n" -X GET \
    -H "3gpp-Sbi-Target-apiRoot: http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080" \
    -H "Authority: sepp.5gc.mnc22.mcc208.3gppnetwork.org:8080" \
    --resolve sepp.5gc.mnc22.mcc208.3gppnetwork.org:8080:192.168.71.132 \
    "http://sepp.5gc.mnc22.mcc208.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D")

echo "$RESPONSE_UDR"

if echo "$RESPONSE_UDR" | grep -q "HTTP_STATUS:200"; then
    echo ">> Test 1 Passed: Successfully discovered UDR via SEPP."
else
    echo ">> Test 1 Failed: Received non-200 HTTP response."
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" logs sepp.5gc.mnc22.mcc208.3gppnetwork.org
    exit 1
fi

echo "=== Test 2: Discover AUSF in PLMN B via NRF ==="
RESPONSE_AUSF=$(curl --silent --show-error --http2-prior-knowledge -w "\nHTTP_STATUS:%{http_code}\n" -X GET \
    -H "Authority: nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080" \
    --resolve nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080:192.168.71.132 \
    "http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D")

echo "$RESPONSE_AUSF"

if echo "$RESPONSE_AUSF" | grep -q "HTTP_STATUS:200"; then
    echo ">> Test 2 Passed: Successfully discovered AUSF."
else
    echo ">> Test 2 Failed: Received non-200 HTTP response."
    $DOCKER_COMPOSE -f "$COMPOSE_FILE" logs sepp.5gc.mnc10.mcc262.3gppnetwork.org
    exit 1
fi

echo "All tests passed successfully!"
