#!/usr/bin/env bash
set -euo pipefail

# Script location: <workspace>/oai-cn5g-sepp/scripts/test/
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SEPP_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
ROOT_DIR="$(cd "$SEPP_DIR/.." && pwd)"

echo "Script directory: $SCRIPT_DIR"
echo "SEPP directory:   $SEPP_DIR"
echo "Workspace root:   $ROOT_DIR"

cd "$ROOT_DIR"

echo "=== Step 1: Cloning Remaining Network Functions ==="
NFS=("oai-cn5g-nrf" "oai-cn5g-ausf" "oai-cn5g-udm" "oai-cn5g-udr")

for nf in "${NFS[@]}"; do
    if [ ! -d "$ROOT_DIR/$nf" ]; then
        echo "Cloning https://github.com/openairinterface/${nf}.git ..."
        git clone "https://github.com/openairinterface/${nf}.git" "$ROOT_DIR/$nf"
    else
        echo "Repository $nf already exists in workspace, skipping clone."
    fi
done

echo "=== Step 2: Initializing and Updating Submodules ==="
ALL_REPOS=("${NFS[@]}" "oai-cn5g-sepp")

for repo in "${ALL_REPOS[@]}"; do
    echo "Updating submodules for $repo..."
    (
        cd "$ROOT_DIR/$repo"
        git submodule init
        git submodule update --init --recursive
    )
done

echo "=== Step 3: Checking out sepp-support branch for NRF and SEPP common-src ==="
if [ -d "$SEPP_DIR/src/common-src" ]; then
    echo "Checking out sepp-support in oai-cn5g-sepp/src/common-src..."
    (
        cd "$SEPP_DIR/src/common-src"
        git checkout sepp-support
    )
fi

if [ -d "$ROOT_DIR/oai-cn5g-nrf/src/common-src" ]; then
    echo "Checking out sepp-support in oai-cn5g-nrf/src/common-src..."
    (
        cd "$ROOT_DIR/oai-cn5g-nrf/src/common-src"
        git checkout sepp-support
    )
fi

echo "=== Step 4: Building All NF Docker Images with tag :latest ==="

echo ">> 1/5 Building oai-sepp:latest..."
(
    cd "$SEPP_DIR"
    docker build -f docker/Dockerfile.sepp.ubuntu \
        --build-arg TARGETPLATFORM=linux/amd64 \
        --build-arg BASE_IMAGE=ubuntu:jammy \
        --target oai-sepp \
        --tag oai-sepp:latest .
)

echo ">> 2/5 Building oai-nrf:latest (and oai-nrf:test)..."
(
    cd "$ROOT_DIR/oai-cn5g-nrf"
    docker build -f docker/Dockerfile.nrf.ubuntu \
        --build-arg TARGETPLATFORM=linux/amd64 \
        --build-arg BASE_IMAGE=ubuntu:jammy \
        --target oai-nrf \
        --tag oai-nrf:latest \
        --tag oai-nrf:test .
)

echo ">> 3/5 Building oai-ausf:latest..."
(
    cd "$ROOT_DIR/oai-cn5g-ausf"
    docker build -f docker/Dockerfile.ausf.ubuntu \
        --build-arg TARGETPLATFORM=linux/amd64 \
        --build-arg BASE_IMAGE=ubuntu:jammy \
        --target oai-ausf \
        --tag oai-ausf:latest .
)

echo ">> 4/5 Building oai-udm:latest..."
(
    cd "$ROOT_DIR/oai-cn5g-udm"
    docker build -f docker/Dockerfile.udm.ubuntu \
        --build-arg TARGETPLATFORM=linux/amd64 \
        --build-arg BASE_IMAGE=ubuntu:jammy \
        --target oai-udm \
        --tag oai-udm:latest .
)

echo ">> 5/5 Building oai-udr:latest..."
(
    cd "$ROOT_DIR/oai-cn5g-udr"
    docker build -f docker/Dockerfile.udr.ubuntu \
        --build-arg TARGETPLATFORM=linux/amd64 \
        --build-arg BASE_IMAGE=ubuntu:jammy \
        --target oai-udr \
        --tag oai-udr:latest .
)

echo "=== All NF Docker images built successfully with tag :latest! ==="
docker images | grep -E "oai-(sepp|nrf|ausf|udm|udr)"
