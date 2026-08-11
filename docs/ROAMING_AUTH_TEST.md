# 5G Core Roaming & 5G-AKA Authentication Tutorial

This tutorial provides end-to-end instructions for deploying an OpenAirInterface (OAI) 5G Core inter-PLMN roaming topology using Docker Compose, running automated 5G-AKA authentication across visited and home networks via SEPP, and analyzing execution logs across all core network functions.

---

## 1. Network Topology & Architecture

* **Visited Network (Partner A):**
  * PLMN ID: `20822` (MCC: 208, MNC: 22)
  * Core NFs: `nrf.5gc.mnc22.mcc208.3gppnetwork.org` (`192.168.71.136`), `sepp.5gc.mnc22.mcc208.3gppnetwork.org` (`192.168.71.132`)
* **Home Network (Partner B):**
  * PLMN ID: `26210` (MCC: 262, MNC: 10)
  * Core NFs: `nrf` (`192.168.73.132`), `ausf` (`192.168.73.133`), `udm` (`192.168.73.135`), `udr` (`192.168.73.134`), `sepp` (`192.168.73.136`), `mysql-B` (`192.168.73.131`)
* **Inter-PLMN Roaming Network:**
  * Subnet: `192.168.72.0/24` (N32 interface connecting SEPP-A `192.168.72.111` to SEPP-B `192.168.72.222`)

---

## 2. Deploy the 5G Core Stack
Pre-requisite: Build required docker images by running `build_images.sh`

Start the Docker Compose roaming topology

NOTE: TLS is disabled by default for Wireshark trace debugging. Set disable_tls: no to enable TLS. Configure TLS keys in Wireshark if decryption is required.

```bash
cd ~/oai-cn5g-sepp/scripts/test
docker-compose -f docker-compose-basic-nrf-roaming.yaml up -d
Creating network "oai-public-netA" with driver "bridge"
Creating network "oai-public-roam" with the default driver
Creating network "oai-public-netB" with driver "bridge"
Creating mysql-B                                ... done
Creating nrf.5gc.mnc22.mcc208.3gppnetwork.org  ... done
Creating nrf.5gc.mnc10.mcc262.3gppnetwork.org ... done
Creating ausf.5gc.mnc10.mcc262.3gppnetwork.org ... done
Creating sepp.5gc.mnc10.mcc262.3gppnetwork.org ... done
Creating udm.5gc.mnc10.mcc262.3gppnetwork.org  ... done
Creating udr.5gc.mnc10.mcc262.3gppnetwork.org  ... done
Creating sepp.5gc.mnc22.mcc208.3gppnetwork.org ... done
```
## 3. Verify Container Health

```bash 
docker ps -a
CONTAINER ID   IMAGE             COMMAND                  CREATED          STATUS                    PORTS                               NAMES
50364c769145   oai-sepp:latest   "/openair-sepp/etc/e…"   31 seconds ago   Up 31 seconds (healthy)   80/tcp, 443/tcp, 8080/tcp           sepp.5gc.mnc22.mcc208.3gppnetwork.org
cfa69e293599   oai-udr:latest    "/openair-udr/bin/oa…"   31 seconds ago   Up 31 seconds (healthy)   80/tcp, 8080/tcp                    udr.5gc.mnc10.mcc262.3gppnetwork.org
0c4406172b92   oai-sepp:latest   "/openair-sepp/etc/e…"   31 seconds ago   Up 31 seconds (healthy)   80/tcp, 443/tcp, 8080/tcp           sepp.5gc.mnc10.mcc262.3gppnetwork.org
519448f704c0   oai-udm:latest    "/openair-udm/bin/oa…"   31 seconds ago   Up 31 seconds (healthy)   80/tcp, 5342-5344/tcp, 8080/tcp    udm.5gc.mnc10.mcc262.3gppnetwork.org
612e2640b510   oai-ausf:latest   "/openair-ausf/bin/o…"   31 seconds ago   Up 31 seconds (healthy)   80/tcp, 5342-5344/tcp, 8080/tcp    ausf.5gc.mnc10.mcc262.3gppnetwork.org
3005e211993e   oai-nrf:test      "/openair-nrf/bin/oa…"   32 seconds ago   Up 32 seconds (healthy)   80/tcp, 5342-5344/tcp, 8080/tcp    nrf.5gc.mnc10.mcc262.3gppnetwork.org
e71c06b9dbba   oai-nrf:test      "/openair-nrf/bin/oa…"   32 seconds ago   Up 32 seconds (healthy)   80/tcp, 5342-5344/tcp, 8080/tcp    nrf.5gc.mnc22.mcc208.3gppnetwork.org
080f95610f1c   mysql:9.6.0       "docker-entrypoint.s…"   32 seconds ago   Up 32 seconds (healthy)   3306/tcp, 33060/tcp                 mysql-B
```
## 4. Execute 5G-AKA Authentication with Home AUSF over SEPP
Run the test execution script from your visited client environment:

```bash
./auth_test.sh

[+] Step 1: Querying Home NRF for AUSF and UDM NF instances...

==================================================
[+] Step 2: Discovered Network Function FQDNs
--------------------------------------------------
    AUSF FQDN : ausf.5gc.mnc10.mcc262.3gppnetwork.org
    UDM  FQDN : udm.5gc.mnc10.mcc262.3gppnetwork.org
    UDR  FQDN : udr.5gc.mnc10.mcc262.3gppnetwork.org
==================================================

[+] Step 3.1: Requesting 5G authentication vector from AUSF (ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080)...
{
  "5gAuthData": {
    "autn": "fd627428095380006a671d86c194278b",
    "hxresStar": "fcca4f94b62ebfb7b16eab85e7fa523c",
    "rand": "1e16332bd324fd90419d8b33f662152d"
  },
  "_links": {
    "5g-aka": {
      "href": "[http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)"
    }
  },
  "authType": "5G_AKA"
}
[+] Received RAND:        1e16332bd324fd90419d8b33f662152d
[+] Received HXRES*:      fcca4f94b62ebfb7b16eab85e7fa523c
[+] Received Confirm URI: [http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)
[+] Step 3.2: Computing Milenage RES* locally...
[+] Computed RES*:        a82a33f6d22b446cb35de4c0098a6f5e
[+] Step 3.3: Sending 5G-AKA Confirmation via HTTP/2 PUT...
[+] Final Response from AUSF:
{
  "authResult": "AUTHENTICATION_SUCCESS",
  "kseaf": "49634a832f3bf11935aa2be549dfdc915b4b3548b9764869a120a2544b0a5c44",
  "supi": "imsi-262100000000031"
}
```

## 4. Component Execution Logs
Home SEPP Log (sepp.5gc.mnc10.mcc262.3gppnetwork.org)
```bash
$ docker logs sepp.5gc.mnc10.mcc262.3gppnetwork.org
[+] Detecting network interfaces...
[+] Target NBI Interface: eth0
[+] Target SBI Interface: eth1
[+] Configuration updated successfully!
[2026-09-15 23:45:24.004] [sepp_app] [debug] Reading SEPP configuration from file: /openair-sepp/etc/config.yaml
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF amf in configuration. Ignored
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF smf in configuration. Ignored
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF upf in configuration. Ignored
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF udm in configuration. Ignored
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF udr in configuration. Ignored
[2026-09-15 23:45:24.024] [config ] [debug] Unknown NF ausf in configuration. Ignored
[2026-09-15 23:45:24.046] [config ] [info] Reading NF configuration from /openair-sepp/etc/config.yaml
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF amf in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF smf in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF upf in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF udm in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF udr in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Unknown NF ausf in configuration. Ignored
[2026-09-15 23:45:24.067] [config ] [debug] Validating configuration of log_level
[2026-09-15 23:45:24.067] [config ] [debug] Validating configuration of register_nf
[2026-09-15 23:45:24.067] [config ] [debug] Validating configuration of http_version
[2026-09-15 23:45:24.067] [config ] [debug] Validating configuration of http_request_timeout
[2026-09-15 23:45:24.067] [config ] [debug] Validating configuration of nrf
[2026-09-15 23:45:24.069] [config ] [debug] Validating configuration of sepp
[2026-09-15 23:45:24.072] [config ] [debug] Validating configuration of database
[2026-09-15 23:45:24.072] [config ] [debug] Validating configuration of enable_roaming
[2026-09-15 23:45:24.072] [config ] [info] ==== OPENAIRINTERFACE sepp vBranch: main Abrev. Hash: 815b3d2 Date: Sat Sep 12 23:45:10 2026 +0200 ====
[2026-09-15 23:45:24.072] [config ] [info] Basic Configuration:
[2026-09-15 23:45:24.072] [config ] [info]   - log_level..................................: debug
[2026-09-15 23:45:24.072] [config ] [info]   - register_nf................................: Yes
[2026-09-15 23:45:24.072] [config ] [info]   - http_version...............................: 2
[2026-09-15 23:45:24.072] [config ] [info]   TLS:
[2026-09-15 23:45:24.072] [config ] [info]     - Enable TLS.................................: No
[2026-09-15 23:45:24.072] [config ] [info]   - HTTP Request Timeout.......................: 3000 (ms)
[2026-09-15 23:45:24.072] [config ] [info]   Roaming:
[2026-09-15 23:45:24.072] [config ] [info]     - Enable Roaming.............................: Yes
[2026-09-15 23:45:24.072] [config ] [info]     - Roaming Partners:
[2026-09-15 23:45:24.072] [config ] [info]       MCC: 208, MNC: 22
[2026-09-15 23:45:24.072] [config ] [info]       MCC: 228, MNC: 06
[2026-09-15 23:45:24.072] [config ] [info]   sepp:
[2026-09-15 23:45:24.072] [config ] [info]     - host.....................................: sepp.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:24.072] [config ] [info]     - SBI
[2026-09-15 23:45:24.072] [config ] [info]       + URL....................................: sepp.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:24.072] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:24.072] [config ] [info]       + IPv4 Address ..........................: 192.168.73.136
[2026-09-15 23:45:24.072] [config ] [info]     - NBI
[2026-09-15 23:45:24.072] [config ] [info]       + URL....................................: sepp.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:24.072] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:24.072] [config ] [info]       + IPv4 Address ..........................: 192.168.72.222
[2026-09-15 23:45:24.072] [config ] [info] Peer NF Configuration:
[2026-09-15 23:45:24.072] [config ] [info]   nrf:
[2026-09-15 23:45:24.072] [config ] [info]     - host.....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:24.072] [config ] [info]     - SBI
[2026-09-15 23:45:24.072] [config ] [info]       + URL....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:24.072] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:24.072] [config ] [info]       + IPv4 Address ..........................: 192.168.72.222
[2026-09-15 23:45:24.072] [sepp_client] [info] HTTP Client successfully initiated on interface eth1 with timeout 1000 ms, HTTP version 2
[2026-09-15 23:45:24.072] [sepp_client] [info] HTTP Client successfully initiated on interface eth0 with timeout 1000 ms, HTTP version 2
[2026-09-15 23:45:24.072] [sepp_app] [start] Starting SEPP Application...
[2026-09-15 23:45:24.072] [sepp_app] [debug] - NF instance info
[2026-09-15 23:45:24.072] [sepp_app] [debug]     Instance ID: 25633505-db3d-458e-aaee-5c34dc6573d2
[2026-09-15 23:45:24.072] [sepp_app] [debug]     Instance name: OAI-SEPP
[2026-09-15 23:45:24.072] [sepp_app] [debug]     Instance type: SEPP
[2026-09-15 23:45:24.073] [sepp_app] [debug]     Instance fqdn: sepp.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:24.073] [sepp_app] [debug]     Status: REGISTERED
[2026-09-15 23:45:24.073] [sepp_app] [debug]     HeartBeat timer: 50
[2026-09-15 23:45:24.073] [sepp_app] [debug]     Priority: 1
[2026-09-15 23:45:24.073] [sepp_app] [debug]     Capacity: 100
[2026-09-15 23:45:24.073] [sepp_app] [debug]     IPv4 Addr:
[2026-09-15 23:45:24.073] [sepp_app] [debug]          192.168.73.136
[2026-09-15 23:45:24.073] [sepp_app] [debug] 	SEPP Info
[2026-09-15 23:45:24.073] [sepp_app] [debug] 		 SeppPrefix: oai-sepp-prefix
[2026-09-15 23:45:24.073] [sepp_app] [debug] 		 SeppPorts:
[2026-09-15 23:45:24.073] [sepp_app] [debug] 			 Interface: n32c - Port: 443
[2026-09-15 23:45:24.073] [sepp_app] [debug] 			 Interface: n32f - Port: 443
[2026-09-15 23:45:24.073] [sepp_app] [debug] 		 SeppPorts:
[2026-09-15 23:45:24.073] [sepp_app] [debug] sepp profile to json:
 {"capacity":100,"custom_info":null,"fqdn":"sepp.5gc.mnc10.mcc262.3gppnetwork.org","heartBeatTimer":50,"ipv4Addresses":["192.168.73.136"],"nfInstanceId":"25633505-db3d-458e-aaee-5c34dc6573d2","nfInstanceName":"OAI-SEPP","nfServices":[{"ipEndPoints":[{"ipv4Address":"192.168.73.136","port":8080,"transport":"TCP"}],"nfServiceStatus":"REGISTERED","scheme":"http","serviceInstanceId":"nsepp-telescopic","serviceName":"nsepp-telescopic","versions":[{"apiFullVersion":"1.0.0","apiVersionInUri":"v1"}]}],"nfStatus":"REGISTERED","nfType":"SEPP","priority":1,"sNssais":[],"seppInfo":{"n32Purposes":["ROAMING"],"remotePlmnList":[{"mcc":"262","mnc":"10"}],"seppPorts":{"n32c":443,"n32f":443},"seppPrefix":"oai-sepp-prefix"}}
[2026-09-15 23:45:24.073] [sepp_sbi] [info] Sending NF registration request to NRF: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-nfm/v1/nf-instances/25633505-db3d-458e-aaee-5c34dc6573d2](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-nfm/v1/nf-instances/25633505-db3d-458e-aaee-5c34dc6573d2)
[2026-09-15 23:45:24.073] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:24.079] [sepp_sbi] [debug] NF registration successful
[2026-09-15 23:45:24.079] [sepp_app] [info] NRF Task Created
[2026-09-15 23:45:24.079] [sepp_nbi] [info] Starting NBI HTTP2 server...
[2026-09-15 23:45:24.079] [sepp_sbi] [info] +++++++ SBI HTTP2 TLS server being started +++++++
[2026-09-15 23:45:24.183] [sepp_app] [info] Processing N32-c capability exchange request
[2026-09-15 23:45:24.183] [sepp_app] [info] Peer indicated 3GppSbiTargetApiRootSupported: true
[2026-09-15 23:45:24.183] [sepp_app] [info] Received capability exchange request from peer SEPP: sepp.5gc.mnc22.mcc208.3gppnetwork.org
[2026-09-15 23:45:24.183] [sepp_app] [info] Successfully processed N32-c capability exchange (Selected Security: PRINS, Purpose: ROAMING, TargetApiRoot: true)
[2026-09-15 23:45:24.184] [sepp_app] [info] Handling N32-c parameter exchange request using model
[2026-09-15 23:45:24.184] [sepp_app] [info] Processed N32-c parameter exchange. N32F Context ID: e23fc02c-f0e0-48e7-bd2d-129fb1230e81, JWE: A128GCM, JWS: ES256
[2026-09-15 23:45:34.105] [sepp_sbi] [info] Sending NF heartbeat request
[2026-09-15 23:45:34.105] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:34.158] [sepp_sbi] [debug] NF heartbeat request successful
[2026-09-15 23:45:44.267] [sepp_sbi] [info] Sending NF heartbeat request
[2026-09-15 23:45:44.267] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:44.270] [sepp_sbi] [debug] NF heartbeat request successful
[2026-09-15 23:45:54.273] [sepp_sbi] [info] Sending NF heartbeat request
[2026-09-15 23:45:54.273] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:54.276] [sepp_sbi] [debug] NF heartbeat request successful
[2026-09-15 23:46:04.277] [sepp_sbi] [info] Sending NF heartbeat request
[2026-09-15 23:46:04.277] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:04.281] [sepp_sbi] [debug] NF heartbeat request successful
[2026-09-15 23:46:08.472] [sepp_app] [info] Sending HTTP request to target: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D)
[2026-09-15 23:46:08.472] [sepp_app] [info] HTTP method: GET
[2026-09-15 23:46:08.472] [sepp_app] [info] Prepared HTTP request: HTTP Request to URI: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=AUSF&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D)
[2026-09-15 23:46:08.472] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.478] [sepp_app] [info] Received HTTP response with status code: 200
[2026-09-15 23:46:08.525] [sepp_app] [info] Sending HTTP request to target: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D)
[2026-09-15 23:46:08.525] [sepp_app] [info] HTTP method: GET
[2026-09-15 23:46:08.525] [sepp_app] [info] Prepared HTTP request: HTTP Request to URI: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-disc/v1/nf-instances?target-nf-type=UDM&requester-nf-type=AMF&plmn-id=%7B%22mcc%22%3A%22262%22%2C%22mnc%22%3A%2210%22%7D)
[2026-09-15 23:46:08.525] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.530] [sepp_app] [info] Received HTTP response with status code: 200
[2026-09-15 23:46:08.580] [sepp_app] [info] Decrypted JOSE incoming payload: {
    "supiOrSuci": "imsi-262100000000031",
    "servingNetworkName": "5G:mnc022.mcc208.3gppnetwork.org"
  }
[2026-09-15 23:46:08.580] [sepp_app] [info] Sending HTTP request to target: [http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications](http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications)
[2026-09-15 23:46:08.580] [sepp_app] [info] HTTP method: POST
[2026-09-15 23:46:08.580] [sepp_app] [info] Prepared HTTP request: HTTP Request to URI: [http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications](http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications)
[2026-09-15 23:46:08.580] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.620] [sepp_app] [info] Received HTTP response with status code: 201
[2026-09-15 23:46:08.790] [sepp_app] [info] Decrypted JOSE incoming payload: {
    "resStar": "a82a33f6d22b446cb35de4c0098a6f5e"
  }
[2026-09-15 23:46:08.790] [sepp_app] [info] Sending HTTP request to target: [http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)
[2026-09-15 23:46:08.790] [sepp_app] [info] HTTP method: PUT
[2026-09-15 23:46:08.790] [sepp_app] [info] Prepared HTTP request: HTTP Request to URI: [http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)
[2026-09-15 23:46:08.790] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.821] [sepp_app] [info] Received HTTP response with status code: 200
[2026-09-15 23:46:14.282] [sepp_sbi] [info] Sending NF heartbeat request
[2026-09-15 23:46:14.282] [sepp_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:14.284] [sepp_sbi] [debug] NF heartbeat request successful
```
Home AUSF Log (ausf.5gc.mnc10.mcc262.3gppnetwork.org)

```bash
$ docker logs ausf.5gc.mnc10.mcc262.3gppnetwork.org
Trying to read .yaml configuration file: /openair-ausf/etc/config.yaml
LTTNG Tracing disabled at build-time!
[2026-09-15 23:45:23.628] [ausf_server] [start] Options parsed
[2026-09-15 23:45:23.628] [ausf_server] [debug] Parsing the configuration file, file type YAML.
[2026-09-15 23:45:23.628] [config ] [info] Reading NF configuration from /openair-ausf/etc/config.yaml
[2026-09-15 23:45:23.665] [config ] [debug] Unknown NF amf in configuration. Ignored
[2026-09-15 23:45:23.665] [config ] [debug] Unknown NF smf in configuration. Ignored
[2026-09-15 23:45:23.665] [config ] [debug] Unknown NF upf in configuration. Ignored
[2026-09-15 23:45:23.665] [config ] [debug] Unknown NF udr in configuration. Ignored
[2026-09-15 23:45:23.665] [config ] [debug] Unknown NF sepp in configuration. Ignored
[2026-09-15 23:45:23.666] [config ] [debug] Validating configuration of log_level
[2026-09-15 23:45:23.666] [config ] [debug] Validating configuration of register_nf
[2026-09-15 23:45:23.666] [config ] [debug] Validating configuration of http_version
[2026-09-15 23:45:23.666] [config ] [debug] Validating configuration of http_request_timeout
[2026-09-15 23:45:23.666] [config ] [debug] Validating configuration of NRF
[2026-09-15 23:45:23.668] [config ] [debug] Validating configuration of UDM
[2026-09-15 23:45:23.669] [config ] [debug] Validating configuration of AUSF
[2026-09-15 23:45:23.767] [config ] [debug] Validating configuration of database
[2026-09-15 23:45:23.767] [config ] [info] ==== OPENAIRINTERFACE ausf vBranch: develop Abrev. Hash: 8bbf78a Date: Thu Sep 3 22:47:25 2026 +0200 ====
[2026-09-15 23:45:23.767] [config ] [info] Basic Configuration:
[2026-09-15 23:45:23.767] [config ] [info]   - log_level..................................: debug
[2026-09-15 23:45:23.767] [config ] [info]   - register_nf................................: Yes
[2026-09-15 23:45:23.767] [config ] [info]   - http_version...............................: 2
[2026-09-15 23:45:23.767] [config ] [info]   TLS:
[2026-09-15 23:45:23.767] [config ] [info]     - Enable TLS.................................: No
[2026-09-15 23:45:23.767] [config ] [info]   - HTTP Request Timeout.......................: 3000 (ms)
[2026-09-15 23:45:23.767] [config ] [info]     AUSF:
[2026-09-15 23:45:23.767] [config ] [info]     - host.....................................: ausf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.767] [config ] [info]     - SBI
[2026-09-15 23:45:23.767] [config ] [info]       + URL....................................: ausf.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:23.767] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:23.767] [config ] [info]       + IPv4 Address ..........................: 192.168.73.133
[2026-09-15 23:45:23.767] [config ] [info]     - Instance ID..............................: 0
[2026-09-15 23:45:23.767] [config ] [info]     - PID Directory............................: 
[2026-09-15 23:45:23.767] [config ] [info]     - AUSF Name................................: 
[2026-09-15 23:45:23.767] [config ] [info] Peer NF Configuration:
[2026-09-15 23:45:23.767] [config ] [info]   NRF:
[2026-09-15 23:45:23.767] [config ] [info]     - host.....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.767] [config ] [info]     - SBI
[2026-09-15 23:45:23.767] [config ] [info]       + URL....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:23.767] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:23.767] [ausf_client] [info] HTTP Client successfully initiated on interface eth0 with timeout 3000 ms, HTTP version 2
[2026-09-15 23:45:23.767] [ausf_app] [start] Starting...
[2026-09-15 23:45:23.767] [ausf_nrf] [info] Create NRF TASK
[2026-09-15 23:45:23.767] [ausf_app] [debug] - NF instance info
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Instance ID: 78e097f9-476a-45a8-a58a-e312d0e4e6d4
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Instance name: OAI-AUSF
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Instance type: AUSF
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Instance fqdn: ausf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Status: REGISTERED
[2026-09-15 23:45:23.767] [ausf_app] [debug]     HeartBeat timer: 50
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Priority: 1
[2026-09-15 23:45:23.767] [ausf_app] [debug]     Capacity: 100
[2026-09-15 23:45:23.767] [ausf_app] [debug]     IPv4 Addr:
[2026-09-15 23:45:23.767] [ausf_app] [debug]          192.168.73.133
[2026-09-15 23:45:23.767] [ausf_app] [debug] 	AUSF Info
[2026-09-15 23:45:23.767] [ausf_app] [debug] 		GroupId: oai-ausf-testgroupid
[2026-09-15 23:45:23.767] [ausf_app] [debug] 		 SupiRanges: Start - q0930j0c80283ncjf, End - , Pattern - 209238210938
[2026-09-15 23:45:23.767] [ausf_app] [debug] 		 Routing Indicators: 0210
[2026-09-15 23:45:23.767] [ausf_app] [debug] 		 Routing Indicators: 9876
[2026-09-15 23:45:23.767] [ausf_nrf] [info] NRF TASK created
[2026-09-15 23:45:23.768] [ausf_app] [debug] AUSF profile to JSON:
 {"ausfInfo":{"groupId":"oai-ausf-testgroupid","routingIndicators":["0210","9876"],"supiRanges":[{"end":"","pattern":"209238210938","start":"q0930j0c80283ncjf"}]},"capacity":100,"fqdn":"ausf.5gc.mnc10.mcc262.3gppnetwork.org","heartBeatTimer":50,"ipv4Addresses":["192.168.73.133"],"nfInstanceId":"78e097f9-476a-45a8-a58a-e312d0e4e6d4","nfInstanceName":"OAI-AUSF","nfStatus":"REGISTERED","nfType":"AUSF","priority":1,"sNssais":[]}
[2026-09-15 23:45:23.768] [ausf_nrf] [info] Sending NF registration request
[2026-09-15 23:45:23.768] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:23.773] [ausf_app] [start] Started
[2026-09-15 23:45:23.776] [ausf_server] [info] HTTP2 server being started
[2026-09-15 23:45:33.837] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:33.838] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:43.963] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:43.964] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:53.969] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:53.969] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:03.974] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:03.975] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.583] [ausf_server] [info] Received ue_authentications_post Request
[2026-09-15 23:46:08.583] [ausf_app] [info] Handle UE Authentication Request
[2026-09-15 23:46:08.583] [ausf_app] [info] ServingNetworkName 5G:mnc022.mcc208.3gppnetwork.org
[2026-09-15 23:46:08.583] [ausf_app] [info] supiOrSuci imsi-262100000000031
[2026-09-15 23:46:08.584] [ausf_app] [debug] UDM's URI [http://udm.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudm-ueau/v1/imsi-262100000000031/security-information/generate-auth-data](http://udm.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudm-ueau/v1/imsi-262100000000031/security-information/generate-auth-data)
[2026-09-15 23:46:08.584] [ausf_app] [info] Received authInfo from AMF without ResynchronizationInfo IE
[2026-09-15 23:46:08.584] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.618] [ausf_app] [info] Response from UDM: {"authType":"5G_AKA","authenticationVector":{"autn":"fd627428095380006a671d86c194278b","avType":"5G_HE_AKA","kausf":"a554cc28ee2ab0d1cefddfd7dcbefcd014c34ad3ed529cee8c80b434ad5391ea","rand":"1e16332bd324fd90419d8b33f662152d","xresStar":"a82a33f6d22b446cb35de4c0098a6f5e"},"supi":"imsi-262100000000031"}
[2026-09-15 23:46:08.619] [ausf_app] [debug] authType 5G_AKA
[2026-09-15 23:46:08.619] [ausf_app] [debug] autn_udm fd627428095380006a671d86c194278b
[2026-09-15 23:46:08.619] [ausf_app] [debug] av_type_udm 5G_HE_AKA
[2026-09-15 23:46:08.619] [ausf_app] [debug] kausf_udm a554cc28ee2ab0d1cefddfd7dcbefcd014c34ad3ed529cee8c80b434ad5391ea
[2026-09-15 23:46:08.619] [ausf_app] [debug] rand_udm 1e16332bd324fd90419d8b33f662152d
[2026-09-15 23:46:08.619] [ausf_app] [debug] xres*_udm a82a33f6d22b446cb35de4c0098a6f5e
[2026-09-15 23:46:08.619] [ausf_app] [debug] Generating 5G AV
[2026-09-15 23:46:08.619] [ausf_app] [debug] HXresStar calculated:
 fcca4f94b62ebfb7b16eab85e7fa523c
[2026-09-15 23:46:08.619] [ausf_app] [debug] Derive_kseaf ...
[2026-09-15 23:46:08.619] [ausf_app] [debug] SNN: 5G:mnc022.mcc208.3gppnetwork.org
[2026-09-15 23:46:08.619] [common] [debug] [ausf_app]derive_kseaf Kausf
a5 54 cc 28 ee 2a b0 d1 ce fd df d7 dc be fc d0 14 c3 4a d3 ed 52 9c ee 8c 80 b4 34 ad 53 91 ea 
[2026-09-15 23:46:08.619] [common] [debug] [ausf_app]derive_kseaf Kseaf
49 63 4a 83 2f 3b f1 19 35 aa 2b e5 49 df dc 91 5b 4b 35 48 b9 76 48 69 a1 20 a2 54 4b 0a 5c 44 
[2026-09-15 23:46:08.619] [ausf_app] [debug] Kseaf calculated:
 49634a832f3bf11935aa2be549dfdc915b4b3548b9764869a120a2544b0a5c44
[2026-09-15 23:46:08.619] [ausf_app] [debug] Create a new security context with SUPI imsi-262100000000031
[2026-09-15 23:46:08.619] [ausf_app] [debug] Auth Response:
 {"5gAuthData":{"autn":"fd627428095380006a671d86c194278b","hxresStar":"fcca4f94b62ebfb7b16eab85e7fa523c","rand":"1e16332bd324fd90419d8b33f662152d"},"_links":{"5g-aka":{"href":"[http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)"}},"authType":"5G_AKA"}
[2026-09-15 23:46:08.619] [ausf_server] [debug] Auth response:
 {"5gAuthData":{"autn":"fd627428095380006a671d86c194278b","hxresStar":"fcca4f94b62ebfb7b16eab85e7fa523c","rand":"1e16332bd324fd90419d8b33f662152d"},"_links":{"5g-aka":{"href":"[http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation](http://192.168.73.133:8080/nausf-auth/v1/ue-authentications/fd627428095380006a671d86c194278b/5g-aka-confirmation)"}},"authType":"5G_AKA"}
[2026-09-15 23:46:08.619] [ausf_server] [info] Send Auth response to SEAF (Code 201)
[2026-09-15 23:46:08.790] [ausf_server] [info] Received 5g_aka_confirmation Request
[2026-09-15 23:46:08.790] [ausf_server] [info] 5gaka confirmation received with authctxID fd627428095380006a671d86c194278b
[2026-09-15 23:46:08.790] [ausf_app] [debug] Handling 5g-aka-confirmation
[2026-09-15 23:46:08.790] [ausf_app] [debug] Retrieve security context with authCtxId: fd627428095380006a671d86c194278b
[2026-09-15 23:46:08.790] [ausf_app] [info] Received authCtxId fd627428095380006a671d86c194278b
[2026-09-15 23:46:08.790] [ausf_app] [info] Received res* a82a33f6d22b446cb35de4c0098a6f5e
[2026-09-15 23:46:08.790] [ausf_app] [debug] authCtxId in AUSF: fd627428095380006a671d86c194278b
[2026-09-15 23:46:08.790] [ausf_app] [info] AV is up to date, handling received res*...
[2026-09-15 23:46:08.790] [ausf_app] [debug] xres* in AUSF: a82a33f6d22b446cb35de4c0098a6f5e
[2026-09-15 23:46:08.790] [ausf_app] [debug] xres in AMF: a82a33f6d22b446cb35de4c0098a6f5e
[2026-09-15 23:46:08.790] [ausf_app] [info] Authentication successful by home network!
[2026-09-15 23:46:08.791] [ausf_app] [debug] UDM's URI: [http://udm.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudm-ueau/v1/imsi-262100000000031/auth-events](http://udm.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudm-ueau/v1/imsi-262100000000031/auth-events)
[2026-09-15 23:46:08.791] [ausf_app] [debug] confirmResultInfo: {"authRemovalInd":false,"authType":"5G_AKA","nfInstanceId":"78e097f9-476a-45a8-a58a-e312d0e4e6d4","servingNetworkName":"5G:mnc022.mcc208.3gppnetwork.org","success":true,"timeStamp":"2026-09-15T21:46:08Z"}
[2026-09-15 23:46:08.791] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.820] [ausf_server] [debug] 5g-aka-confirmation response:
 {"authResult":"AUTHENTICATION_SUCCESS","kseaf":"49634a832f3bf11935aa2be549dfdc915b4b3548b9764869a120a2544b0a5c44","supi":"imsi-262100000000031"}
[2026-09-15 23:46:08.820] [ausf_server] [info] Send 5g-aka-confirmation response to SEAF (Code 200)
[2026-09-15 23:46:13.983] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:13.983] [ausf_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:24.004] [ausf_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:24.005] [ausf_client] [debug] Send a simple HTTP request
```

Home UDM Log (udm.5gc.mnc10.mcc262.3gppnetwork.org)
```bash
$ docker logs udm.5gc.mnc10.mcc262.3gppnetwork.org
Trying to read .yaml configuration file: /openair-udm/etc/config.yaml
LTTNG Tracing disabled at build-time!
[2026-09-15 23:45:23.935] [system ] [start] Options parsed
[2026-09-15 23:45:23.935] [system ] [debug] Parsing the configuration file (YAML).
[2026-09-15 23:45:23.935] [config ] [info] Reading NF configuration from /openair-udm/etc/config.yaml
[2026-09-15 23:45:23.979] [config ] [debug] Unknown NF amf in configuration. Ignored
[2026-09-15 23:45:23.980] [config ] [debug] Unknown NF smf in configuration. Ignored
[2026-09-15 23:45:23.980] [config ] [debug] Unknown NF upf in configuration. Ignored
[2026-09-15 23:45:23.980] [config ] [debug] Unknown NF ausf in configuration. Ignored
[2026-09-15 23:45:23.980] [config ] [debug] Unknown NF sepp in configuration. Ignored
[2026-09-15 23:45:23.980] [config ] [debug] Validating configuration of log_level
[2026-09-15 23:45:23.980] [config ] [debug] Validating configuration of register_nf
[2026-09-15 23:45:23.980] [config ] [debug] Validating configuration of http_version
[2026-09-15 23:45:23.980] [config ] [debug] Validating configuration of http_request_timeout
[2026-09-15 23:45:23.980] [config ] [debug] Validating configuration of NRF
[2026-09-15 23:45:23.983] [config ] [debug] Validating configuration of UDR
[2026-09-15 23:45:23.983] [config ] [debug] Validating configuration of UDM
[2026-09-15 23:45:23.987] [config ] [debug] Validating configuration of database
[2026-09-15 23:45:23.987] [config ] [info] ==== OPENAIRINTERFACE udm vBranch: udm-profile-fix Abrev. Hash: 1479d20 Date: Sun Sep 13 21:26:51 2026 +0200 ====
[2026-09-15 23:45:23.987] [config ] [info] Basic Configuration:
[2026-09-15 23:45:23.987] [config ] [info]   - log_level..................................: debug
[2026-09-15 23:45:23.987] [config ] [info]   - register_nf................................: Yes
[2026-09-15 23:45:23.987] [config ] [info]   - http_version...............................: 2
[2026-09-15 23:45:23.987] [config ] [info]   TLS:
[2026-09-15 23:45:23.987] [config ] [info]     - Enable TLS.................................: No
[2026-09-15 23:45:23.987] [config ] [info]   - HTTP Request Timeout.......................: 3000 (ms)
[2026-09-15 23:45:23.987] [config ] [info]     UDM:
[2026-09-15 23:45:23.987] [config ] [info]     - host.....................................: udm.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.987] [config ] [info]     - SBI
[2026-09-15 23:45:23.987] [config ] [info]       + URL....................................: udm.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:23.987] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:23.987] [config ] [info]       + IPv4 Address ..........................: 192.168.73.135
[2026-09-15 23:45:23.987] [config ] [info]     - Instance ID..............................: 0
[2026-09-15 23:45:23.987] [config ] [info]     - PID Directory............................: 
[2026-09-15 23:45:23.987] [config ] [info]     - UDM Name.................................: 
[2026-09-15 23:45:23.987] [config ] [info] Peer NF Configuration:
[2026-09-15 23:45:23.987] [config ] [info]   NRF:
[2026-09-15 23:45:23.987] [config ] [info]     - host.....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.987] [config ] [info]     - SBI
[2026-09-15 23:45:23.987] [config ] [info]       + URL....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:23.987] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:23.987] [config ] [info]   UDR:
[2026-09-15 23:45:23.987] [config ] [info]     - host.....................................: udr.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.987] [config ] [info]     - SBI
[2026-09-15 23:45:23.987] [config ] [info]       + URL....................................: udr.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:23.987] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:23.987] [udm_client] [info] HTTP Client successfully initiated on interface eth0 with timeout 3000 ms, HTTP version 2
[2026-09-15 23:45:23.987] [udm_app] [start] Starting...
[2026-09-15 23:45:23.987] [udm_app] [debug] - NF instance info
[2026-09-15 23:45:23.987] [udm_app] [debug]     Instance ID: b74756e0-e645-4dd0-9dcd-782d9a026ab3
[2026-09-15 23:45:23.987] [udm_app] [debug]     Instance name: OAI-UDM
[2026-09-15 23:45:23.987] [udm_app] [debug]     Instance type: UDM
[2026-09-15 23:45:23.987] [udm_app] [debug]     Instance fqdn: udm.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:23.987] [udm_app] [debug]     Status: REGISTERED
[2026-09-15 23:45:23.987] [udm_app] [debug]     HeartBeat timer: 50
[2026-09-15 23:45:23.987] [udm_app] [debug]     Priority: 1
[2026-09-15 23:45:23.987] [udm_app] [debug]     Capacity: 100
[2026-09-15 23:45:23.987] [udm_app] [debug]     IPv4 Addr:
[2026-09-15 23:45:23.987] [udm_app] [debug]          192.168.73.135
[2026-09-15 23:45:23.987] [udm_app] [debug] 	UDM Info
[2026-09-15 23:45:23.987] [udm_app] [debug] 		GroupId: 
[2026-09-15 23:45:23.987] [udm_app] [debug] udm profile to JSON:
 {"capacity":100,"fqdn":"udm.5gc.mnc10.mcc262.3gppnetwork.org","heartBeatTimer":50,"ipv4Addresses":["192.168.73.135"],"nfInstanceId":"b74756e0-e645-4dd0-9dcd-782d9a026ab3","nfInstanceName":"OAI-UDM","nfStatus":"REGISTERED","nfType":"UDM","priority":1,"sNssais":[],"udmInfo":{"externalGroupIdentifiersRanges":[],"gpsiRanges":[],"groupId":"","internalGroupIdentifiersRanges":[],"routingIndicators":[],"supiRanges":[]}}
[2026-09-15 23:45:23.987] [udm_nrf] [info] Sending NF registration request to NRF, NRF's URI: [http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-nfm/v1/nf-instances/b74756e0-e645-4dd0-9dcd-782d9a026ab3](http://nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080/nnrf-nfm/v1/nf-instances/b74756e0-e645-4dd0-9dcd-782d9a026ab3)
[2026-09-15 23:45:23.987] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:23.991] [udm_app] [info] NRF TASK Created 
[2026-09-15 23:45:23.991] [udm_app] [start] Started
[2026-09-15 23:45:23.992] [udm_server] [info] HTTP2 server being started
[2026-09-15 23:45:34.022] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:34.022] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:44.153] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:44.153] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:45:54.156] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:45:54.157] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:04.164] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:04.165] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.587] [udm_ueau] [info] Handle generate_auth_data()
[2026-09-15 23:46:08.587] [udm_ueau] [info] Handle Generate Auth Data Request
[2026-09-15 23:46:08.587] [udm_ueau] [debug] 5GS mobile identity type: SUPI
[2026-09-15 23:46:08.587] [udm_ueau] [debug] Remote URI: [http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription](http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription)
[2026-09-15 23:46:08.587] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.594] [common] [debug] [udm_ueau]Result For F1-Alg Key
0c 0a 34 60 1d 4f 07 67 73 03 65 2c 04 62 53 5b 
[2026-09-15 23:46:08.594] [common] [debug] [udm_ueau]Result For F1-Alg OPC
63 bf a5 0e e6 52 33 65 ff 14 c1 f4 5f 88 73 7d 
[2026-09-15 23:46:08.594] [common] [debug] [udm_ueau]Result For F1-Alg AMF
80 00 
[2026-09-15 23:46:08.594] [common] [debug] [udm_ueau]Result For F1-Alg SQN: 
00 00 00 00 00 20 
[2026-09-15 23:46:08.595] [udm_ueau] [info] Current SQN 000000000040
71 a3 a2 45 28 39 c9 92 cd 8a 2f eb ad 88 35 b 
72 4c e6 19 37 2c 39 27 50 a7 1f c3 b9 8f df 88 
71 a3 a2 45 28 39 c9 92 cd 8a 2f eb ad 88 35 b 
ee e7 82 e3 8e b6 f9 3f cc c 7b 39 0 15 1f 91 
9f f3 ca 38 a2 40 19 72 77 74 29 e7 e6 8f e5 da 
b3 5 2a 75 19 38 4b ac 91 ee d3 af 97 9b ad 7 
[2026-09-15 23:46:08.596] [common] [debug] [udm_ueau]XRES*(new)
c1 b1 a6 95 3e cc 79 33 74 49 94 1b ba b5 24 63 a8 2a 33 f6 d2 2b 44 6c b3 5d e4 c0 09 8a 6f 5e 
[2026-09-15 23:46:08.596] [udm_ueau] [debug] derive_kausf ...
[2026-09-15 23:46:08.596] [common] [debug] [udm_ueau]derive_kausf key
07 a7 25 00 fd cb c3 10 f5 18 7f 29 80 b9 a7 44 63 82 c9 29 1c c4 3f b0 c8 80 1f 0c d2 13 18 a3 
[2026-09-15 23:46:08.596] [common] [debug] [udm_ueau]derive_kausf kausf
a5 54 cc 28 ee 2a b0 d1 ce fd df d7 dc be fc d0 14 c3 4a d3 ed 52 9c ee 8c 80 b4 34 ad 53 91 ea 
[2026-09-15 23:46:08.597] [udm_ueau] [info] New SQN (for next round) = 000000000060
[2026-09-15 23:46:08.597] [udm_ueau] [debug] Remote URI: [http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription](http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription)
[2026-09-15 23:46:08.597] [udm_ueau] [info] Update UDR with PATCH message, body:  [{"from":"","op":"replace","path":"","value":"{\"lastIndexes\":{\"ausf\":0},\"sqn\":\"000000000060\",\"sqnScheme\":\"NON_TIME_BASED\"}"}]
[2026-09-15 23:46:08.597] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.617] [udm_ueau] [info] Send 200 OK response to AUSF
[2026-09-15 23:46:08.618] [udm_ueau] [info] AuthInfoResult {"authType":"5G_AKA","authenticationVector":{"autn":"fd627428095380006a671d86c194278b","avType":"5G_HE_AKA","kausf":"a554cc28ee2ab0d1cefddfd7dcbefcd014c34ad3ed529cee8c80b434ad5391ea","rand":"1e16332bd324fd90419d8b33f662152d","xresStar":"a82a33f6d22b446cb35de4c0098a6f5e"},"supi":"imsi-262100000000031"}
[2026-09-15 23:46:08.618] [udm_ueau] [info] Send response to AUSF
[2026-09-15 23:46:08.618] [udm_ueau] [info] Update sqn in Database
[2026-09-15 23:46:08.791] [udm_ueau] [info] Handle Authentication Confirmation
[2026-09-15 23:46:08.791] [udm_ueau] [debug] Remote URI: [http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription](http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-subscription)
[2026-09-15 23:46:08.791] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.793] [udm_ueau] [debug] Remote URI:[http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-status](http://udr.5gc.mnc10.mcc262.3gppnetwork.org:8080/nudr-dr/v1/subscription-data/imsi-262100000000031/authentication-data/authentication-status)
[2026-09-15 23:46:08.793] [udm_ueau] [debug] Request body = {"authRemovalInd":false,"authType":"5G_AKA","nfInstanceId":"78e097f9-476a-45a8-a58a-e312d0e4e6d4","servingNetworkName":"5G:mnc022.mcc208.3gppnetwork.org","success":true,"timeStamp":"2026-09-15T21:46:08Z"}
[2026-09-15 23:46:08.793] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.819] [udm_ueau] [debug] authEventId=1
[2026-09-15 23:46:08.819] [udm_ueau] [info] Send 201 Created response to AUSF
[2026-09-15 23:46:08.819] [udm_ueau] [info] Send response to AUSF
[2026-09-15 23:46:14.175] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:14.175] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:24.202] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:24.203] [udm_client] [debug] Send a simple HTTP request
[2026-09-15 23:46:34.213] [udm_nrf] [info] Sending NF heartbeat request
[2026-09-15 23:46:34.214] [udm_client] [debug] Send a simple HTTP request
```

Home UDR Log (udr.5gc.mnc10.mcc262.3gppnetwork.org)
```bash
docker logs udr.5gc.mnc10.mcc262.3gppnetwork.org
Trying to read .yaml configuration file: /openair-udr/etc/config.yaml
LTTNG Tracing disabled at build-time!
[2026-09-15 23:45:23.971] [common] [start] Options parsed
[2026-09-15 23:45:23.971] [common] [debug] Parsing the configuration file, file type YAML.
[2026-09-15 23:45:23.971] [config ] [info] Reading NF configuration from /openair-udr/etc/config.yaml
[2026-09-15 23:45:23.996] [config ] [debug] Unknown NF amf in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Unknown NF smf in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Unknown NF upf in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Unknown NF udm in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Unknown NF ausf in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Unknown NF sepp in configuration. Ignored
[2026-09-15 23:45:23.997] [config ] [debug] Validating configuration of log_level
[2026-09-15 23:45:23.997] [config ] [debug] Validating configuration of register_nf
[2026-09-15 23:45:23.997] [config ] [debug] Validating configuration of http_version
[2026-09-15 23:45:23.997] [config ] [debug] Validating configuration of http_request_timeout
[2026-09-15 23:45:23.997] [config ] [debug] Validating configuration of NRF
[2026-09-15 23:45:23.999] [config ] [debug] Validating configuration of UDR
[2026-09-15 23:45:24.001] [config ] [debug] Validating configuration of database
[2026-09-15 23:45:24.001] [config ] [info] ==== OPENAIRINTERFACE udr vBranch: develop Abrev. Hash: 7c018ea Date: Thu Sep 3 22:22:36 2026 +0200 ====
[2026-09-15 23:45:24.001] [config ] [info] Basic Configuration:
[2026-09-15 23:45:24.001] [config ] [info]   - log_level..................................: debug
[2026-09-15 23:45:24.001] [config ] [info]   - register_nf................................: Yes
[2026-09-15 23:45:24.001] [config ] [info]   - http_version...............................: 2
[2026-09-15 23:45:24.001] [config ] [info]   TLS:
[2026-09-15 23:45:24.001] [config ] [info]     - Enable TLS.................................: No
[2026-09-15 23:45:24.001] [config ] [info]   - HTTP Request Timeout.......................: 3000 (ms)
[2026-09-15 23:45:24.001] [config ] [info]     UDR:
[2026-09-15 23:45:24.001] [config ] [info]     - host.....................................: udr.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:24.001] [config ] [info]     - sbi
[2026-09-15 23:45:24.001] [config ] [info]       + URL....................................: udr.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:24.001] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:24.001] [config ] [info]       + IPv4 Address ..........................: 192.168.73.134
[2026-09-15 23:45:24.001] [config ] [info]     - UDR Name.................................: oai-udr
[2026-09-15 23:45:24.001] [config ] [info]   Database:
[2026-09-15 23:45:24.001] [config ] [info]     - Host.....................................: mysql-B
[2026-09-15 23:45:24.001] [config ] [info]     - Port.....................................: 3306
[2026-09-15 23:45:24.001] [config ] [info]     - Database Type............................: mysql
[2026-09-15 23:45:24.001] [config ] [info]     - User.....................................: test
[2026-09-15 23:45:24.001] [config ] [info]     - Password.................................: test
[2026-09-15 23:45:24.001] [config ] [info]     - Database Name............................: oai_db
[2026-09-15 23:45:24.002] [config ] [info]     - Connection Timeout.......................: 300
[2026-09-15 23:45:24.002] [config ] [info] Peer NF Configuration:
[2026-09-15 23:45:24.002] [config ] [info]   NRF:
[2026-09-15 23:45:24.002] [config ] [info]     - host.....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:24.002] [config ] [info]     - sbi
[2026-09-15 23:45:24.002] [config ] [info]       + URL....................................: nrf.5gc.mnc10.mcc262.3gppnetwork.org:8080
[2026-09-15 23:45:24.002] [config ] [info]       + API Version............................: v1
[2026-09-15 23:45:24.002] [udr_nrf] [info] HTTP Client successfully initiated on interface eth0 with timeout 3000 ms, HTTP version 2
[2026-09-15 23:45:24.002] [udr_app] [start] Starting...
[2026-09-15 23:45:24.002] [udr_db] [debug] Initializing MySQL DB ...
[2026-09-15 23:45:24.002] [udr_db] [debug] Done!
[2026-09-15 23:45:24.002] [udr_db] [debug] Connecting to MySQL DB
[2026-09-15 23:45:24.003] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:24.504] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:25.005] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:25.506] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:26.007] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:26.508] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:27.008] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:27.510] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:28.010] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:28.512] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:29.013] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:29.514] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:30.016] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:30.517] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:31.017] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:31.518] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:32.019] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:32.519] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:33.020] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:33.521] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:34.022] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:34.523] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:35.024] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:35.524] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:36.026] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:36.526] [udr_db] [error] An error occurred when connecting to MySQL DB (Can't connect to MySQL server on 'mysql-B:3306' (111)), retry ...
[2026-09-15 23:45:37.067] [udr_db] [info] Connected to MySQL DB
[2026-09-15 23:45:37.067] [udr_app] [debug] - NF instance info
[2026-09-15 23:45:37.067] [udr_app] [debug]     Instance ID: f253bb2b-31eb-46f9-96d8-e09eb2c2e421
[2026-09-15 23:45:37.067] [udr_app] [debug]     Instance name: OAI-UDR
[2026-09-15 23:45:37.067] [udr_app] [debug]     Instance type: UDR
[2026-09-15 23:45:37.067] [udr_app] [debug]     Instance fqdn: udr.5gc.mnc10.mcc262.3gppnetwork.org
[2026-09-15 23:45:37.067] [udr_app] [debug]     Status: REGISTERED
[2026-09-15 23:45:37.067] [udr_app] [debug]     HeartBeat timer: 50
[2026-09-15 23:45:37.068] [udr_app] [debug]     Priority: 1
[2026-09-15 23:45:37.068] [udr_app] [debug]     Capacity: 100
[2026-09-15 23:45:37.068] [udr_app] [debug]     IPv4 Addr:
[2026-09-15 23:45:37.068] [udr_app] [debug]          192.168.73.134
[2026-09-15 23:45:37.068] [udr_app] [debug] 	UDR Info
[2026-09-15 23:45:37.068] [udr_app] [debug] 		GroupId: oai-udr-testgroupid
[2026-09-15 23:45:37.068] [udr_app] [debug] 		 SupiRanges: Start - 208950000000131, End - , Pattern - ^imsi-20895[31-131]{6}$[2026-09-15 23:45:37.068] [udr_app] [debug] 		 GpsiRanges: Start - 752740000, End - 752749999, Pattern - ^gpsi-75274[0-9]{4}$
[2026-09-15 23:45:37.068] [udr_app] [debug] 		 Data Set Id: 0210
[2026-09-15 23:45:37.068] [udr_app] [debug] 		 Data Set Id: 9876
[2026-09-15 23:45:37.068] [udr_app] [info] NRF TASK Created 
[2026-09-15 23:45:37.068] [udr_server] [info] HTTP2 server being started 
[2026-09-15 23:45:37.069] [udr_app] [debug] udr profile to JSON:
 {"capacity":100,"fqdn":"udr.5gc.mnc10.mcc262.3gppnetwork.org","heartBeatTimer":50,"ipv4Addresses":["192.168.73.134"],"nfInstanceId":"f253bb2b-31eb-46f9-96d8-e09eb2c2e421","nfInstanceName":"OAI-UDR","nfStatus":"REGISTERED","nfType":"UDR","priority":1,"sNssais":[],"udrInfo":{"DataSetId":["0210","9876"],"externalGroupIdentifiersRanges":[],"gpsiRanges":[{"end":"752749999","pattern":"^gpsi-75274[0-9]{4}$","start":"752740000"}],"groupId":"oai-udr-testgroupid","supiRanges":[{"end":"","pattern":"^imsi-20895[31-131]{6}$","start":"208950000000131"}]}}
[2026-09-15 23:45:37.069] [udr_nrf] [info] Sending NF Registration request
[2026-09-15 23:45:37.069] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:45:47.153] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:45:47.154] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:45:57.209] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:45:57.210] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:46:07.220] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:46:07.221] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:46:08.591] [udr_server] [info] Received response: 
[2026-09-15 23:46:08.591] [udr_app] [info] [UE Id imsi-262100000000031] Retrieve the Authentication Subscription data of an UE
[2026-09-15 23:46:08.591] [udr_db] [info] [UE Id 262100000000031] Query Authentication Subscription
[2026-09-15 23:46:08.591] [udr_db] [info] [UE Id 262100000000031] MySQL Query: SELECT * FROM AuthenticationSubscription WHERE ueid='262100000000031'
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [0]: ueid 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [1]: authenticationMethod 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [2]: encPermanentKey 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [3]: protectionParameterId 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [4]: sequenceNumber 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [5]: authenticationManagementField 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [6]: algorithmId 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [7]: encOpcKey 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [8]: encTopcKey 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [9]: vectorGenerationInHss 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [10]: n5gcAuthMethod 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [11]: rgAuthenticationInd 
[2026-09-15 23:46:08.593] [udr_db] [debug] [UE Id 262100000000031] Row [12]: supi 
[2026-09-15 23:46:08.593] [udr_app] [info] [UE Id imsi-262100000000031] AuthenticationSubscription: {"algorithmId":"milenage","authenticationManagementField":"8000","authenticationMethod":"5G_AKA","encOpcKey":"63bfa50ee6523365ff14c1f45f88737d","encPermanentKey":"0C0A34601D4F07677303652C0462535B","protectionParameterId":"0C0A34601D4F07677303652C0462535B","sequenceNumber":{"lastIndexes":{"ausf":0},"sqn":"000000000020","sqnScheme":"NON_TIME_BASED"},"supi":"262100000000031"}
[2026-09-15 23:46:08.593] [udr_server] [info] HTTP Response code 200 (HTTP Version 2).

[2026-09-15 23:46:08.600] [udr_app] [info] [UE Id imsi-262100000000031] Modify the Authentication Subscription data of an UE
[2026-09-15 23:46:08.600] [udr_db] [info] [UE Id 262100000000031] MySQL Query: SELECT * from AuthenticationSubscription WHERE ueid='262100000000031'
[2026-09-15 23:46:08.600] [udr_db] [debug] [UE Id 262100000000031] Patch value: "{\"lastIndexes\":{\"ausf\":0},\"sqn\":\"000000000060\",\"sqnScheme\":\"NON_TIME_BASED\"}"
[2026-09-15 23:46:08.601] [udr_db] [info] [UE Id 262100000000031] MySQL Update command UPDATE AuthenticationSubscription SET sequenceNumber='{"lastIndexes":{"ausf":0},"sqn":"000000000060","sqnScheme":"NON_TIME_BASED"}' WHERE ueid='262100000000031'
[2026-09-15 23:46:08.617] [udr_db] [info] [UE Id 262100000000031] AuthenticationSubscription PATCH: [{"from":"","op":"replace","path":"","value":"{\"lastIndexes\":{\"ausf\":0},\"sqn\":\"000000000060\",\"sqnScheme\":\"NON_TIME_BASED\"}"}]
[2026-09-15 23:46:08.617] [udr_app] [info] [UE Id imsi-262100000000031] Successful modified the Authentication subscription data
[2026-09-15 23:46:08.617] [udr_server] [debug] HTTP Response code 204 (HTTP Version 2).

[2026-09-15 23:46:08.792] [udr_server] [info] Received response: 
[2026-09-15 23:46:08.792] [udr_app] [info] [UE Id imsi-262100000000031] Retrieve the Authentication Subscription data of an UE
[2026-09-15 23:46:08.792] [udr_db] [info] [UE Id 262100000000031] Query Authentication Subscription
[2026-09-15 23:46:08.792] [udr_db] [info] [UE Id 262100000000031] MySQL Query: SELECT * FROM AuthenticationSubscription WHERE ueid='262100000000031'
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [0]: ueid 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [1]: authenticationMethod 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [2]: encPermanentKey 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [3]: protectionParameterId 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [4]: sequenceNumber 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [5]: authenticationManagementField 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [6]: algorithmId 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [7]: encOpcKey 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [8]: encTopcKey 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [9]: vectorGenerationInHss 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [10]: n5gcAuthMethod 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [11]: rgAuthenticationInd 
[2026-09-15 23:46:08.793] [udr_db] [debug] [UE Id 262100000000031] Row [12]: supi 
[2026-09-15 23:46:08.793] [udr_app] [info] [UE Id imsi-262100000000031] AuthenticationSubscription: {"algorithmId":"milenage","authenticationManagementField":"8000","authenticationMethod":"5G_AKA","encOpcKey":"63bfa50ee6523365ff14c1f45f88737d","encPermanentKey":"0C0A34601D4F07677303652C0462535B","protectionParameterId":"0C0A34601D4F07677303652C0462535B","sequenceNumber":{"lastIndexes":{"ausf":0},"sqn":"000000000060","sqnScheme":"NON_TIME_BASED"},"supi":"262100000000031"}
[2026-09-15 23:46:08.793] [udr_server] [info] HTTP Response code 200 (HTTP Version 2).

[2026-09-15 23:46:08.794] [udr_app] [info] [UE Id imsi-262100000000031] Store the Authentication Status data of an UE
[2026-09-15 23:46:08.794] [udr_db] [info] [UE Id 262100000000031] MySQL query: SELECT * FROM AuthenticationStatus WHERE ueid='262100000000031'
[2026-09-15 23:46:08.810] [udr_db] [info] [UE Id 262100000000031] MySQL query: INSERT INTO AuthenticationStatus SET ueid='262100000000031',nfInstanceId='78e097f9-476a-45a8-a58a-e312d0e4e6d4',success=1,timeStamp='2026-09-15T21:46:08Z',authType='5G_AKA',servingNetworkName='5G:mnc022.mcc208.3gppnetwork.org',authRemovalInd=0
[2026-09-15 23:46:08.818] [udr_db] [info] [UE Id 262100000000031] Authentication Status PUT: {"authRemovalInd":false,"authType":"5G_AKA","nfInstanceId":"78e097f9-476a-45a8-a58a-e312d0e4e6d4","servingNetworkName":"5G:mnc022.mcc208.3gppnetwork.org","success":true,"timeStamp":"2026-09-15T21:46:08Z"}
[2026-09-15 23:46:08.818] [udr_app] [info] [UE Id imsi-262100000000031] Successful stored the Authentication Status data
[2026-09-15 23:46:08.818] [udr_server] [debug] HTTP Response code 204 (HTTP Version 2).

[2026-09-15 23:46:17.250] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:46:17.250] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:46:27.256] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:46:27.257] [udr_nrf] [debug] Send a simple HTTP request
[2026-09-15 23:46:37.272] [udr_nrf] [info] Sending NF Heartbeat Request
[2026-09-15 23:46:37.273] [udr_nrf] [debug] Send a simple HTTP request
```