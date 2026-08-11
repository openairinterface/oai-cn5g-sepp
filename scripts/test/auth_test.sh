#!/usr/bin/env bash
set -e

# --- Configuration & Credentials ---
SEPP_A_IP="192.168.71.132"
SEPP_A_PORT="8080"
SEPP_A_HOST="sepp.5gc.mnc22.mcc208.3gppnetwork.org:8080"
TARGET_NRF_APIROOT="http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080"

SUPI="imsi-262100000000031"
SN_NAME="5G:mnc022.mcc208.3gppnetwork.org"

# Subscriber Key and OPc from database
K_HEX="0C0A34601D4F07677303652C0462535B"
OPC_HEX="63bfa50ee6523365ff14c1f45f88737d"

# ==============================================================================
# STEP 1: Discover AUSF and UDM FQDNs from Home NRF via SEPP
# ==============================================================================
echo "[+] Step 1: Querying Home NRF for AUSF and UDM NF instances..."

# Query AUSF
AUSF_DISC_RESP=$(curl -s --http2-prior-knowledge \
  -H "3gpp-Sbi-Target-apiRoot: ${TARGET_NRF_APIROOT}" \
  -H "Authority: ${SEPP_A_HOST}" \
  --resolve "${SEPP_A_HOST}:${SEPP_A_IP}" \
  "http://${SEPP_A_HOST}/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D")

AUSF_FQDN=$(echo "${AUSF_DISC_RESP}" | jq -r '.nfInstances[0].fqdn // empty')

# Query UDM
UDM_DISC_RESP=$(curl -s --http2-prior-knowledge \
  -H "3gpp-Sbi-Target-apiRoot: ${TARGET_NRF_APIROOT}" \
  -H "Authority: ${SEPP_A_HOST}" \
  --resolve "${SEPP_A_HOST}:${SEPP_A_IP}" \
  "http://${SEPP_A_HOST}/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D")

UDM_FQDN=$(echo "${UDM_DISC_RESP}" | jq -r '.nfInstances[0].fqdn // empty')

if [ -z "${AUSF_FQDN}" ]; then
  echo "[-] Failed to discover AUSF FQDN from NRF."
  exit 1
fi

# ==============================================================================
# STEP 2: Display Obtained FQDNs
# ==============================================================================
echo ""
echo "=================================================="
echo "[+] Step 2: Discovered Network Function FQDNs"
echo "--------------------------------------------------"
echo "    AUSF FQDN : ${AUSF_FQDN}"
echo "    UDM FQDN  : ${UDM_FQDN}"
echo "=================================================="
echo ""

# ==============================================================================
# STEP 3: Perform 5G-AKA Authentication
# ==============================================================================
AUSF_HOST="${AUSF_FQDN}:${SEPP_A_PORT}"

echo "[+] Step 3.1: Requesting 5G authentication vector from AUSF (${AUSF_HOST})..."
INIT_RESP=$(curl -s --http2-prior-knowledge \
  -H "Content-Type: application/json" \
  -H "Accept: application/3gppHal+json, application/problem+json, application/json" \
  -H "Authority: ${AUSF_HOST}" \
  --resolve "${AUSF_HOST}:${SEPP_A_IP}" \
  -d "{
    \"supiOrSuci\": \"${SUPI}\",
    \"servingNetworkName\": \"${SN_NAME}\"
  }" \
  "http://${AUSF_HOST}/nausf-auth/v1/ue-authentications")

echo "${INIT_RESP}" | jq .

RAND_HEX=$(echo "${INIT_RESP}" | jq -r '.["5gAuthData"].rand')
AUTN_HEX=$(echo "${INIT_RESP}" | jq -r '.["5gAuthData"].autn')
HXRES_STAR_HEX=$(echo "${INIT_RESP}" | jq -r '.["5gAuthData"].hxresStar')
CONFIRM_HREF=$(echo "${INIT_RESP}" | jq -r '._links["5g-aka"].href')

if [ -z "${RAND_HEX}" ] || [ "${RAND_HEX}" = "null" ]; then
  echo "[-] Authentication vector generation failed."
  exit 1
fi

echo "[+] Received RAND:        ${RAND_HEX}"
echo "[+] Received HXRES*:      ${HXRES_STAR_HEX}"
echo "[+] Received Confirm URI: ${CONFIRM_HREF}"

echo "[+] Step 3.2: Computing Milenage RES* locally..."
RES_STAR_HEX=$(python3 - <<EOF
import hmac, hashlib
from Crypto.Cipher import AES

K = bytes.fromhex("${K_HEX}")
OPC = bytes.fromhex("${OPC_HEX}")
RAND = bytes.fromhex("${RAND_HEX}")
SN = "${SN_NAME}".encode('utf-8')

def xor(a, b): return bytes(x ^ y for x, y in zip(a, b))
def rot(b, r): return b[(r // 8) % 16:] + b[:(r // 8) % 16]

cipher = AES.new(K, AES.MODE_ECB)
tmp = cipher.encrypt(xor(RAND, OPC))

# Milenage f2 (RES)
f2_in = xor(rot(xor(tmp, OPC), 0), bytes.fromhex("00000000000000000000000000000001"))
res = xor(cipher.encrypt(f2_in), OPC)[8:16]

# Milenage f3 (CK)
f3_in = xor(rot(xor(tmp, OPC), 32), bytes.fromhex("00000000000000000000000000000002"))
ck = xor(cipher.encrypt(f3_in), OPC)

# Milenage f4 (IK)
f4_in = xor(rot(xor(tmp, OPC), 64), bytes.fromhex("00000000000000000000000000000004"))
ik = xor(cipher.encrypt(f4_in), OPC)

# 3GPP TS 33.501 Annex A.4 (RES*)
key = ck + ik
s = bytearray([0x6B])
s.extend(SN)
s.extend(len(SN).to_bytes(2, 'big'))
s.extend(RAND)
s.extend(len(RAND).to_bytes(2, 'big'))
s.extend(res)
s.extend(len(res).to_bytes(2, 'big'))

res_star = hmac.new(key, bytes(s), hashlib.sha256).digest()[16:32]
print(res_star.hex())
EOF
)

echo "[+] Computed RES*:        ${RES_STAR_HEX}"

CONFIRM_PATH=$(echo "${CONFIRM_HREF}" | sed -E 's|^http://[^/]+||')

echo "[+] Step 3.3: Sending 5G-AKA Confirmation via HTTP/2 PUT..."
CONFIRM_RESP=$(curl -s --http2-prior-knowledge -X PUT \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, application/problem+json" \
  -H "Authority: ${AUSF_HOST}" \
  --resolve "${AUSF_HOST}:${SEPP_A_IP}" \
  -d "{
    \"resStar\": \"${RES_STAR_HEX}\"
  }" \
  "http://${AUSF_HOST}${CONFIRM_PATH}")

echo "[+] Final Response from AUSF:"
echo "${CONFIRM_RESP}" | jq .
