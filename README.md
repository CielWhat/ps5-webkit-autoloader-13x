# PS5 WebKit Autoloader — 13.x Port (13.00 / 13.20 / 13.40 / 13.42 / 13.60)

Experimental port of the PS5 WebKit Autoloader to 13.x using the Relapse exploit chain.
Built and tested locally; PS5 hardware validation pending (see `PS5-Autoloader-13x-Test/READ-ME-FIRST.txt`).

## Contents

* `wkal-build/` — ported autoloader source + successful `0.4.0-relapse-13x` build (`installer.elf`, PC host). See `wkal-build/PORT-13X.md` and `wkal-build/BUILD-STATUS.md`.
* `ps5-webkit-autoloader-main/` — upstream autoloader source snapshot (rev `137f065d2ba8022ba8e796895e7cb05c204f2ce9`) with 13.x integration.
* `Relapse-Exploit-main/` — upstream Relapse exploit snapshot (rev `254df04dd58c67ad6f3bb30e76eea40f79bffe51`).
* `PS5-Autoloader-13x-Test/` — ready-to-use deliverables for 13.40: `webkit-autoloader-installer_v0.4.0-relapse-13x.elf` + `webkit-autoloader-host_v0.4.0-relapse-13x.py`. See `BUILD-INFO.json`.
* Original ZIPs + `autoloader-13x-existing-files.diff` retained for provenance.

## Credits

Full credits in [`CREDITS.md`](CREDITS.md) and [`THIRD-PARTY-NOTICES.md`](THIRD-PARTY-NOTICES.md).

* Autoloader: [itsPLK](https://github.com/itsPLK/ps5-webkit-autoloader)
* Relapse exploit: ntfargo, ufm42, Sonic-Iso, Jordy, Dr. Yenyen, TheFlow, SlidyBat, Flatz, cow, nhk, bollarz, Sleirsgoevy, EchoStretch, EarthOnion
* umtx2: [idlesauce](https://github.com/idlesauce/umtx2) & contributors
* slopkit: [jordyidk](https://github.com/jordyidk/slopkit) & contributors
* pooP2JB: [soniciso1](https://github.com/soniciso1/pooP2JB)
* ps5-payload-sdk / elfldr: [john-tornblom](https://github.com/john-tornblom) / [ps5-payload-dev](https://github.com/ps5-payload-dev/sdk/)
* puff.c: [Mark Adler](https://github.com/madler)

## Licenses

* Autoloader: GPL-3.0 (`wkal-build/LICENSE`, `PS5-Autoloader-13x-Test/LICENSE-autoloader.txt`)
* Relapse: MIT © 2026 Nathan Fargo (`Relapse-Exploit-main/LICENSE`, `PS5-Autoloader-13x-Test/LICENSE-relapse.txt`)
