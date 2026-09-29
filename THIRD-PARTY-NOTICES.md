# Third-Party Notices

## ps5-webkit-autoloader (itsPLK)
* Source: https://github.com/itsPLK/ps5-webkit-autoloader
* Revision: `137f065d2ba8022ba8e796895e7cb05c204f2ce9`
* License: GPL-3.0 — see `wkal-build/LICENSE` / `PS5-Autoloader-13x-Test/LICENSE-autoloader.txt`

Bundled submodules / derived work:
* `third_party/ps5-unified-autoloader` — https://github.com/itsPLK/ps5-unified-autoloader (GPL-3.0)
* `third_party/slopkit` — https://github.com/itsPLK/slopkit (fork of https://github.com/jordyidk/slopkit)
* `third_party/umtx2` — https://github.com/idlesauce/umtx2
* `third_party/ps5-elfldr` — https://github.com/itsPLK/ps5-elfldr (fork of https://github.com/ps5-payload-dev/elfldr)
* `frontend/autoloader/slopkit/`, `frontend/autoloader/umtx2/`, `frontend/autoloader/shared/` generated from the above at build time
* puff.c by Mark Adler — https://github.com/madler/zlib/tree/master/contrib/puff

## Relapse-Exploit (ntfargo et al.)
* Source: https://github.com/ntfargo/Relapse-Exploit (snapshot `Relapse-Exploit-main/`)
* Revision: `254df04dd58c67ad6f3bb30e76eea40f79bffe51`
* License: MIT © 2026 Nathan Fargo — see `Relapse-Exploit-main/LICENSE` / `PS5-Autoloader-13x-Test/LICENSE-relapse.txt` / `wkal-build/frontend/autoloader/relapse/LICENSE`
* Authors: ntfargo, ufm42, Sonic-Iso, Jordy, Dr. Yenyen, TheFlow, SlidyBat, Flatz, cow, nhk, bollarz, Sleirsgoevy, EchoStretch, EarthOnion
* Vendored binaries (hashes in `wkal-build/frontend/autoloader/relapse/PROVENANCE.json`):
  * `payloads/kexp_2026_05_25.bin` — sha256 `7cfb3a8cb86db67893c360ae3531f460f800e9f24039797038e6e81fc50f18a9`
  * `payloads/elfldr-ps5-1360.elf` — sha256 `de5dd480d12637527ba2d75e35de851adbb59c455293fa32588c492ef6b44b81`
* Upstream README retained as reference at `wkal-build/frontend/autoloader/relapse/README.md`; its R2 optional-payload menu is replaced by the WKAL handoff in this port.
