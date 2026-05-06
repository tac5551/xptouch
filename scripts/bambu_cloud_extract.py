#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any

PEM_CERT_RE = re.compile(
    r"-----BEGIN CERTIFICATE-----\s+([A-Za-z0-9+/=\r\n]+?)\s+-----END CERTIFICATE-----",
    re.DOTALL,
)


def load_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def load_text_from_binary_strings(path: Path) -> str:
    data = path.read_bytes()
    chunks = re.findall(rb"[ -~]{8,}", data)
    return "\n".join(c.decode("latin1", errors="ignore") for c in chunks)


def extract_main_jsc_from_asar(asar_path: Path, out_jsc_path: Path) -> bool:
    out_jsc_path.parent.mkdir(parents=True, exist_ok=True)
    npx_cmd = "npx.cmd" if os.name == "nt" else "npx"
    tmp_dir = Path(tempfile.mkdtemp(prefix="xptouch_asar_"))
    try:
        cmd = [npx_cmd, "asar", "extract", str(asar_path), str(tmp_dir)]
        res = subprocess.run(cmd, capture_output=True, text=True, check=False)
        if res.returncode != 0:
            return False
        extracted = tmp_dir / ".vite" / "build" / "main.jsc"
        if not extracted.exists():
            return False
        out_jsc_path.write_bytes(extracted.read_bytes())
        return True
    except Exception:
        return False
    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)


def extract_pem_certs(text: str) -> list[str]:
    certs: list[str] = []
    seen: set[str] = set()
    for m in PEM_CERT_RE.finditer(text):
        body = "".join(line.strip() for line in m.group(1).splitlines() if line.strip())
        pem = "-----BEGIN CERTIFICATE-----\n"
        for i in range(0, len(body), 64):
            pem += body[i : i + 64] + "\n"
        pem += "-----END CERTIFICATE-----\n"
        if pem not in seen:
            seen.add(pem)
            certs.append(pem)
    return certs


def parse_cert_meta(pem: str) -> dict[str, Any]:
    meta: dict[str, Any] = {"valid": False}
    try:
        from cryptography import x509  # type: ignore

        cert = x509.load_pem_x509_certificate(pem.encode("utf-8"))
        meta["valid"] = True
        meta["subject"] = cert.subject.rfc4514_string()
        meta["issuer"] = cert.issuer.rfc4514_string()
        meta["serial_hex"] = format(cert.serial_number, "x")
        meta["not_before"] = cert.not_valid_before_utc.isoformat()
        meta["not_after"] = cert.not_valid_after_utc.isoformat()
    except Exception as e:
        meta["error"] = str(e)
    return meta


def main() -> int:
    parser = argparse.ArgumentParser(description="Extract certs from Bambu Connect app.asar/main.jsc/strings.txt")
    parser.add_argument(
        "--input",
        default=r"C:\Users\user\AppData\Local\Programs\bambu-connect\resources\app.asar",
        help="input file (.asar / .jsc / .txt)",
    )
    parser.add_argument("--out", default="scripts/certs_out", help="output directory")
    args = parser.parse_args()

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)
    in_path = Path(args.input)
    if not in_path.exists():
        print(f"[ERROR] input not found: {in_path}")
        print(
            "Bambu Connect is not installed (or installed in a custom path). "
            "Please install Bambu Connect and retry, or pass --input with a valid path."
        )
        return 1

    extracted_jsc = out_dir / "main.from.asar.jsc"
    strings_txt = out_dir / "main.jsc.strings.txt"

    if in_path.suffix.lower() == ".asar":
        if not extract_main_jsc_from_asar(in_path, extracted_jsc):
            print("[ERROR] failed to extract .vite/build/main.jsc from app.asar")
            print("Please ensure Node.js is installed and npx.cmd works.")
            return 1
        text = load_text_from_binary_strings(extracted_jsc)
        strings_txt.write_text(text, encoding="utf-8")
    elif in_path.suffix.lower() == ".jsc":
        text = load_text_from_binary_strings(in_path)
        strings_txt.write_text(text, encoding="utf-8")
    else:
        text = load_text(in_path)

    pems = extract_pem_certs(text)
    if not pems:
        print("[ERROR] no PEM certificate blocks found")
        return 1

    metas = [parse_cert_meta(p) for p in pems]
    all_dir = out_dir / "all"
    all_dir.mkdir(parents=True, exist_ok=True)
    for i, pem in enumerate(pems):
        (all_dir / f"cert_{i:02d}.pem").write_text(pem, encoding="utf-8")

    (out_dir / "server_cert.pem").write_text(pems[0], encoding="utf-8")
    if len(pems) > 1:
        (out_dir / "app_cert_candidate.pem").write_text(pems[1], encoding="utf-8")
    (out_dir / "report.json").write_text(
        json.dumps(
            {
                "input": str(in_path),
                "total_certificates": len(pems),
                "certificates": [{"index": i, **metas[i]} for i in range(len(metas))],
            },
            ensure_ascii=False,
            indent=2,
        ),
        encoding="utf-8",
    )

    print(f"[OK] extracted certs: {len(pems)}")
    print(f"[OK] output: {out_dir}")

    # Copy required certs to sibling repo: ../xptouch-bin/cert
    try:
        bin_cert_dir = Path(__file__).resolve().parent.parent.parent / "xptouch-bin" / "cert"
        bin_cert_dir.mkdir(parents=True, exist_ok=True)
        copy_targets = ["server_cert.pem", "app_cert_candidate.pem", "ca_bundle.pem"]
        copied = []
        for name in copy_targets:
            src = out_dir / name
            if src.exists():
                dst = bin_cert_dir / name
                dst.write_bytes(src.read_bytes())
                copied.append(name)
        if copied:
            print(f"[OK] copied to {bin_cert_dir}: {', '.join(copied)}")
        else:
            print(f"[WARN] no cert files copied to {bin_cert_dir}")
    except Exception as e:
        print(f"[WARN] cert copy skipped: {e}")

    for p in (extracted_jsc, strings_txt):
        try:
            if p.exists():
                p.unlink()
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

