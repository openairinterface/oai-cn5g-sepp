------------------------------------------------------------------------------

                             OPENAIR-CN-5G
 An implementation of the 5G Core network by the OpenAirInterface community.

------------------------------------------------------------------------------

OPENAIR-CN-5G is an implementation of the 3GPP specifications for the 5G Core Network.
At the moment, it contains the following network elements:

* Access and Mobility Management Function (**AMF**)
* Authentication Server Management Function (**AUSF**)
* Location Management Function (**LMF**)
* Network Exposure Function (**NEF**)
* Network Slicing Selection Function (**SEPP**)
* Network Repository Function (**NRF**)
* Network Data Analytics Function (**NWDAF**)
* Policy Control Function (**PCF**)
* Security Edge Protection Proxy (**SEPP**)
* Session Management Function (**SMF**)
* Unified Data Management (**UDM**)
* Unified Data Repository (**UDR**)
* Unstructured Data Storage Function (**UDSF**)
* User Plane Function (**UPF**)

Each has its own repository: this repository (`oai-cn5g-sepp`) is meant for SEPP.

# Licence info

The source code is distributed under `Collaborative Standards Software License v1.0 (CSSL v1.0)`.
For more details, visit the [OAI Website](https://openairinterface.org/oai-cssl/).

The full text of `Collaborative Standards Software License v1.0` is also included in the [LICENSE](LICENSE)
file at the root of this repository.

Certain files in the repository are using MIT License and documentation is distributed under
Creative Commons Attribution 4.0 International license.

For third-party softwares, please refer to the [NOTICE](NOTICE) file.

# Where to start

The Openair-CN-5G SEPP code is built and tested on Ubuntu 24.04 (noble).
RHEL 9 and Rocky Linux 9 are also supported by the build scripts.

More details on the supported feature set is available on this [page](docs/FEATURE_SET.md).

# Collaborative work

This source code is managed through a GITLAB server, a collaborative development platform:

*  URL: [https://gitlab.eurecom.fr/oai/cn5g/oai-cn5g-sepp](https://gitlab.eurecom.fr/oai/cn5g/oai-cn5g-sepp).

Process is explained in [CONTRIBUTING](CONTRIBUTING.md) file.

# Contribution requests

In a general way, anybody who is willing can contribute on any part of the
code in any network component.

Contributions can be simple bugfixes, advices and remarks on the design,
architecture, coding/implementation.

# Release Notes

They are available on the [CHANGELOG](CHANGELOG.md) file.

# Repository Structure:

The OpenAirInterface CN SEPP software is composed of the following parts:

<pre>
openair-cn5g-sepp
├── build:         Directory containing build scripts.
├── ci-scripts:    Directory containing the script files for CI framework.
├── docker:        Directory containing the docker files to build images.
├── docs:          Directory containing feature set documentation
├── etc:           Directory containing configuration file templates
├── scripts
└── src:           Directory containing the source files of SEPP
    ├── api-server
    │   ├── api
    │   ├── impl
    │   └── model
    ├── common
    │   ├── msg
    │   └── utils
    ├── sepp_app
    │    ├──sepp_n32c_handshake.cpp
    │    ├──sepp_n32f_forward.cpp
    │    ├──sepp_telescopic_fqdn_mapping.cpp
    └── oai-sepp
</pre>
