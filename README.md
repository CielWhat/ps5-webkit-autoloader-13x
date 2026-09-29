# PS5 WebKit Autoloader — 13.x Port (13.00 / 13.20 / 13.40 / 13.42 / 13.60)

Experimental port of the PS5 WebKit Autoloader to 13.x using the Relapse exploit chain.
Built and tested locally; PS5 hardware validation pending (see `PS5-Autoloader-13x-Test/READ-ME-FIRST.txt`).

## Contents

* `wkal-build/` — ported autoloader source + successful `0.4.0-relapse-13x` build (`installer.elf`, PC host). See `wkal-build/PORT-13X.md` and `wkal-build/BUILD-STATUS.md`.
* `ps5-webkit-autoloader-main/` — upstream autoloader source snapshot (rev `137f065d2ba8022ba8e796895e7cb05c204f2ce9`) with 13.x integration.
* `Relapse-Exploit-main/` — upstream Relapse exploit snapshot (rev `254df04dd58c67ad6f3bb30e76eea40f79bffe51`).
* `PS5-Autoloader-13x-Test/` — ready-to-use deliverables for 13.40: `webkit-autoloader-installer_v0.4.0-relapse-13x.elf` + `webkit-autoloader-host_v0.4.0-relapse-13x.py`. See `BUILD-INFO.json`.
* Original ZIPs + `autoloader-13x-existing-files.diff` retained for provenance.

## Setup Instructions

There are two ways to set up the autoloader, depending on whether you're already jailbroken.

### Already jailbroken? Just load the installer ELF

1. Get `webkit-autoloader-installer_v0.4.0-relapse-13x.elf` from `PS5-Autoloader-13x-Test/`.
2. Send it to your PS5 with `elfldr` (port `9021`), or launch it from Payload Manager:
   `nc <PS5-IP> 9021 < webkit-autoloader-installer_v0.4.0-relapse-13x.elf`
3. The installer opens the browser once to cache the autoloader page (Relapse chain for 13.x), then creates the **WebKit Autoloader** app on the homescreen and exits. Look for `Detected firmware 13.40, caching Relapse exploit` and `Launcher app installed successfully` in the log.
4. **Reboot once**, then launch **WebKit Autoloader** from the homescreen — no PC or DNS needed anymore.

### Not jailbroken yet

Host the exploit locally on your PC for the initial setup:

1. Run `webkit-autoloader-host_v0.4.0-relapse-13x.py` from `PS5-Autoloader-13x-Test/` on a PC on your network (or double-click `Start Host.command` on macOS).
2. On your PS5, set your network's Primary DNS to your PC's IP address.
3. Open the **User's Guide** from Settings to run the installer, which adds the **WebKit Autoloader** app to your homescreen.
4. **Reboot once**, then launch **WebKit Autoloader** from the homescreen.

## How to Use

After the Relapse chain runs, your payloads are sent via **Payload Manager**, or a custom `autoload.txt` — same as the upstream WebKit autoloader.

### 🟢 Option 1: Payload Manager

If no `autoload.txt` config is found, the autoloader will automatically launch **[Payload Manager](https://github.com/itsPLK/ps5-payload-manager)** — a fully-featured PS5 payload manager with a web UI. This lets you configure and send payloads directly from your browser, without needing to manually set up config files or transfer ELF files ahead of time.

Just run the autoloader — if there's nothing configured, Payload Manager starts automatically.

> **Note:** Payload Manager also has its own built-in autoload feature, which lets you configure payloads to load automatically on startup — all managed through its web UI. This is separate from the `autoload.txt` mechanism described below.

### ⚙️ Option 2: Manual Config (`autoload.txt`)

For a fixed, automated payload chain, you can configure payloads manually:

- Create a directory named `ps5_autoloader`.
- Inside this directory, place your `.elf` / `.bin` files, and an `autoload.txt` file.
  - In `autoload.txt`, list the files you want to load, one filename per line.
  - Filenames are case-sensitive — ensure each name exactly matches the file.
  - You can add lines like `!1000` to make the loader wait 1000 ms before sending the next payload.
- Put the `ps5_autoloader` directory in one of these locations (priority order - highest first):
  - Root of a USB drive
  - Internal drive: `/data/ps5_autoloader`

> **Note:** When an `autoload.txt` config is found, Payload Manager is **not** launched automatically. If you also want Payload Manager available, place `pldmgr.elf` in your `ps5_autoloader` directory and add it to `autoload.txt`.

## Additional Info

<Details>
<Summary><i>How to update the autoloader?</i></Summary>

The autoloader content is cached on the console, so updating is exactly the same as the initial install. Simply follow the **[Setup Instructions](#setup-instructions)** using the new release files.

The latest installer payload will re-create the homescreen app and refresh the cached page for you. Your payloads and `autoload.txt` on USB / internal storage are never touched.
</Details>

<Details>
<Summary><i>How to use a custom ELF Loader?</i></Summary>

On 13.x (Relapse) the autoloader boots `elfldr-ps5-1360.elf` and sends the payload to elfldr on `localhost:9021`.

If you want to use a "normal" ELF Loader that allows sending payloads from any device, you can simply load it through **Payload Manager**.

Alternatively, if you are using a manual config file (`autoload.txt`):
1. Place your custom ELF Loader (e.g. `elfldr.elf`) in the `ps5_autoloader` directory.
2. Add `elfldr.elf` to your `autoload.txt`.
3. **Note**: If you are loading other payloads right after `elfldr.elf` in your `autoload.txt`, add a sleep command immediately after it (like `!4000` to sleep for 4 seconds) to give the new ELF Loader time to start up and listen before subsequent payloads are sent.

Example `autoload.txt`:
```text
# Load custom ELF Loader
elfldr.elf
# Give it 4 seconds to start up (only needed if sending more payloads)
!4000
# Send other payloads
etaHEN.elf
```
</Details>

## Stability notes (13.x / Relapse)

* WebKit may need several attempts — reload the page if the browser stalls.
* The kernel exploit may hang or panic the console — reboot before trying again if that happens.
* The installer log reporting success means the cache + homescreen app were created; payload compatibility beyond the installer (etaHEN/kstuff/ShadowMount) is still being validated on 13.x.

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
