# Build status: 2026-09-29

The experimental 0.4.0-relapse-13x installer and standalone host have now been
built successfully. The deliverables are in ../PS5-Autoloader-13x-Test/.
See its BUILD-INFO.json for revisions, checks and checksums.

The missing submodules and payload dependencies have been restored. Docker CLI,
Buildx, Colima and Lima were installed with Homebrew. Docker Desktop installation
failed at its administrator step and was rolled back; the build instead used
the dedicated Colima profile `wkal`, which is being stopped after compilation.
The profile and compiler image are retained for debugging until the PS5 test
succeeds. No VM is required to run the generated Python host.

Built inside Linux/amd64 using the supplied Dockerfile.sdk. The build command
was `make all` with CUSTOM_VERSION=relapse-13x. The PC host was generated using
`tools/build_host.py --payload installer.elf` with that same version suffix.
Local integration, packaging, binary consistency and HTTP-serving checks pass.

Hardware validation remains outstanding: exploit execution on 13.40, installer
execution, homescreen shortcut creation, reboot/offline AppCache launch, and
subsequent unified-autoloader/payload compatibility. Successful compilation does
not establish those runtime results.
