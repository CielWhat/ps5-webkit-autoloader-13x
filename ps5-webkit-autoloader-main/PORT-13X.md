# Experimental Relapse integration (7.00–13.60 default)

## Status

Source integration completed; local integration tests pass. The installer ELF
has been built from the wkal-build copy and install-tested on a 13.40 console
(cache + homescreen app OK). Offline launch / payload runs on other firmwares
remain unverified.

Automatic routing selects Relapse for every firmware it ships offsets for:
7.00–13.60 (33 versions, including **12.60** and **13.40** — no more 50-minute
poops/p2jb waits). umtx2 still serves 1.00–5.50. poops/p2jb remain only as
legacy fallbacks: 9.05/11.40 have no Relapse offsets, and any chain can still
be forced via `?force=` or build-time `FORCE_EXPLOIT`.

## Changes

- Bundled the supplied Relapse source, offsets, kernel-stage binary and its
  elfldr-ps5-1360.elf. No new exploit offsets or primitives were invented.
- Replaced Relapse's R2 optional-payload menu with the existing WKAL handoff:
  ../payloads/payload.elf, sent through the supplied sender to localhost:9021.
  During installation this is the installer; in the installed cache it is the
  unified-autoloader payload. The included optional kstuff/etaHEN/ShadowMount
  files from the reference archive are not bundled or automatically sent.
- Added exact firmware routing, forced Relapse mode, log mirroring, and parent
  success/failure reporting. The iframe remains alive after the transfer.
- Removed the timestamp from offset URLs and wait for offset-script readiness
  before running the chain, so those files can load from AppCache.
- Added the exact iframe query URL to AppCache and updated native cache
  filtering to retain Relapse assets on 7.00+ (slopkit files are still cached
  on 7.00–12.70 as legacy fallback) and omit unrelated chains.
- Restricted completion messages to the same-origin exploit iframe.
- Auto-routing order is now umtx2 → Relapse → poops → p2jb, so 7.00–12.70
  consoles (e.g. 12.60) run Relapse instead of the slow slopkit chains.

The WebKit and kernel exploit implementations are unchanged. Successful byte
transfer means elfldr received the payload, not that the native installer or
all subsequently configured payloads executed successfully.

## Build requirements

The provided GitHub source ZIP has empty third_party directories; it does not
include the git submodules. This Mac also has neither Docker nor the PS5 SDK
compiler at /opt/ps5-payload-sdk/bin/prospero-clang. These prevent a native build
here. The source port deliberately does not substitute an old installer ELF,
which would install the original cache without Relapse.

On a machine with Docker, first get a full upstream checkout at the supplied
Autoloader revision, then overlay this source directory (including the vendored
Relapse payloads) onto that checkout, preserving its .git and submodule folders:

```sh
git clone https://github.com/itsPLK/ps5-webkit-autoloader.git wkal-build
cd wkal-build
git checkout 137f065d2ba8022ba8e796895e7cb05c204f2ce9
git submodule update --init --recursive
# Overlay this source package onto wkal-build before continuing.
CUSTOM_VERSION=relapse-13x bash build_release.sh
```

The existing build script builds the SDK container, installer and bundled PC
host. Use the resulting host/installer from this build, not the old upstream
release. SDK, unified-autoloader and Payload Manager firmware compatibility
still need validation; browser offsets alone do not prove those components
work on 13.40 or 13.60. The native installer also uses firmware-sensitive system
interfaces and process structure offsets that are unverified on 13.x.

## Local checks

Run from the source directory:

```sh
node tests/relapse.test.cjs
python3 -m unittest discover -s tests -p 'test_*.py'
cc -Wall -Wextra -Werror -Iinclude tests/cache_filter.c -o /tmp/wkal-cache-filter-test
/tmp/wkal-cache-filter-test
```

Checked: all 45 JavaScript files parse; exact firmware routing and unsupported
versions; offset-load readiness/errors; partial payload writes, invalid/missing
payloads and socket cleanup; native cache selection; offline manifest URLs;
and PC-host inclusion of the required offset, worker and binary assets. The
sender tests use mocks and never run the exploit.

Still required on hardware: native installer execution, complete cache fill,
homescreen app creation, reboot/offline launch, unified-autoloader handoff and
configured payload compatibility. Start with your 13.40 console; each other
firmware remains unverified until separately tested.

## Source provenance

Autoloader ZIP revision: 137f065d2ba8022ba8e796895e7cb05c204f2ce9.
Relapse ZIP revision: 254df04dd58c67ad6f3bb30e76eea40f79bffe51.

Both original ZIPs remain unchanged alongside this project. See
frontend/autoloader/relapse/PROVENANCE.json for archive and binary SHA-256
hashes, and its LICENSE for the upstream license. Its README.md is retained
as upstream reference; its original R2 usage does not describe this integration.
