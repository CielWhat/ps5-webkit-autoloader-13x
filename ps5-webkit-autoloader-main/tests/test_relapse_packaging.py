"""Verify offline URL coverage and PC-host embedding without console execution."""
import io
from pathlib import Path
import sys
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from gen_file_registry import build_manifest, apply_exploit_mode_placeholder
from build_host import build_zip
from unittest.mock import patch


class PackagingTests(unittest.TestCase):
    def test_offline_and_host_assets(self):
        frontend = ROOT / "frontend/autoloader"
        files = [("/app/test/" + p.relative_to(frontend).as_posix(), str(p))
                 for p in frontend.rglob("*") if p.is_file()]
        manifest = build_manifest(files, "test", "test", "/app/test",
                                  "/app/index.html", "/app/test/__complete__")
        cached = manifest.split("CACHE:\n", 1)[1].split("\nNETWORK:", 1)[0].splitlines()
        self.assertIn("/app/test/relapse/index.html?autoload=payload.elf", cached)
        self.assertEqual(cached[-2:], ["/app/index.html", "/app/test/__complete__"])
        with tempfile.TemporaryDirectory() as overrides:
            archive, _ = build_zip(str(frontend), overrides, "test", "test")
        with zipfile.ZipFile(io.BytesIO(archive)) as z:
            for rel in ["relapse/src/utils/rop_slave.js",
                        "relapse/payloads/elfldr-ps5-1360.elf",
                        "relapse/payloads/kexp_2026_05_25.bin",
                        *[f"relapse/offsets/{v}.js" for v in
                          ("13.00", "13.20", "13.40", "13.42", "13.60")]]:
                self.assertIn("/app/test/" + rel, cached)
                self.assertEqual(z.read(rel), (frontend / rel).read_bytes())
            self.assertNotIn("relapse/payloads/etaHEN.elf", z.namelist())

    def test_forced_mode(self):
        with patch.dict("os.environ", {"FORCE_EXPLOIT": "relapse"}):
            result = apply_exploit_mode_placeholder("/app/test/app.js",
                                                    b"[[EXPLOIT_MODE]]", "/app/test")
        self.assertEqual(result, b"relapse")


if __name__ == "__main__":
    unittest.main()
