# Credits

This repo combines the PS5 WebKit Autoloader with the Relapse exploit chain for 13.x.
No original authorship is claimed here — all credit belongs to the upstream creators below.

## PS5 WebKit Autoloader

* **[itsPLK](https://github.com/itsPLK/ps5-webkit-autoloader)** — autoloader design, installer, offline AppCache hosting, Payload Manager / `autoload.txt` flow
  * Related: [ps5-unified-autoloader](https://github.com/itsPLK/ps5-unified-autoloader), [ps5-payload-manager](https://github.com/itsPLK/ps5-payload-manager), [ps5-y2jb-autoloader](https://github.com/itsPLK/ps5-y2jb-autoloader), [ps5-bdjb-autoloader](https://github.com/itsPLK/ps5-bdjb-autoloader), [ps5-lua-autoloader](https://github.com/itsPLK/ps5-lua-autoloader), [ps5-elfldr](https://github.com/itsPLK/ps5-elfldr) fork
  * Upstream revision in this repo: `137f065d2ba8022ba8e796895e7cb05c204f2ce9`
  * License: GPL-3.0

## Relapse Exploit (7.00–13.60, used here for 13.00 / 13.20 / 13.40 / 13.42 / 13.60)

* **ntfargo, ufm42, Sonic-Iso, Jordy, Dr. Yenyen, TheFlow, SlidyBat, Flatz, cow, nhk, bollarz, Sleirsgoevy, EchoStretch, EarthOnion**
  * Upstream: `ntfargo/Relapse-Exploit`, revision `254df04dd58c67ad6f3bb30e76eea40f79bffe51`
  * Copyright (c) 2026 Nathan Fargo, License: MIT
  * Community: PS5 Research & Development (Discord)
  * In this repo: `Relapse-Exploit-main/`, vendored runtime under `wkal-build/frontend/autoloader/relapse/` (see `PROVENANCE.json`). Exploit primitives and offsets unchanged; `main.js` / `site.js` / `kexp.js` / `index.html` adapted for WKAL handoff.

## Exploit / SDK foundations (via Autoloader)

* **[idlesauce](https://github.com/idlesauce/umtx2)** & contributors — [umtx2](https://github.com/idlesauce/umtx2) (1.00–5.50 route)
* **[jordyidk](https://github.com/jordyidk)** & contributors — [slopkit](https://github.com/jordyidk/slopkit) (poops / p2jb base, 7.00–12.70 routes)
* **[soniciso1](https://github.com/soniciso1)** — [pooP2JB](https://github.com/soniciso1/pooP2JB)
* **[john-tornblom](https://github.com/john-tornblom)** — [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk/) and [elfldr](https://github.com/ps5-payload-dev/elfldr)
* **[Mark Adler](https://github.com/madler)** — [puff.c](https://github.com/madler/zlib/tree/master/contrib/puff) (embedded frontend decompression)
* Everyone else contributing to the PS5 homebrew scene.

## This 13.x port

* Integration only (routing, AppCache/cache-filter, iframe handoff, PC-host packaging). No new exploit offsets or primitives invented.
* See `wkal-build/PORT-13X.md`, `wkal-build/BUILD-STATUS.md`, `PS5-Autoloader-13x-Test/BUILD-INFO.json`.
