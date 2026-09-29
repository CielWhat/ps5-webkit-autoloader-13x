# Build status: 2026-09-29 (rebuilt: Relapse default for 7.00–13.60)

The experimental 0.4.0-relapse-13x installer and standalone host have been
rebuilt with Relapse as the automatic route for 7.00–13.60 (poops/p2jb kept
as legacy fallback). The deliverables are in ../PS5-Autoloader-13x-Test/.
See its BUILD-INFO.json for revisions, checks and checksums.

Rebuild notes: Docker bind-mounts of paths containing spaces come up empty
under this Colima/virtiofs setup, so the source was staged via tar into a
`wkalsrc` Docker volume, built there with `make clean all`
(CUSTOM_VERSION=relapse-13x), and the artifacts copied back with
`docker cp`. Throwaway git repos were initialised for the third_party
submodule checkouts inside the volume only (the flat tree carries no
submodule `.git` dirs); ps5-unified-autoloader was tagged v0.1.4-955249d to
match the pinned payload sidecar. Local integration, packaging, binary
consistency and HTTP-serving checks (now including 12.60) pass.

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
