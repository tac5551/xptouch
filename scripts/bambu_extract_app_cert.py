#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


DEFAULT_ROAMING_DIR = Path.home() / "AppData" / "Roaming" / "Bambu Connect"
DEFAULT_MAINJS_FILE = Path(__file__).resolve().parent.parent / ".." / "trush" / "main.js.txt"
DEFAULT_OUT_DIR = Path(__file__).resolve().parent / "certs_out"

RE_PEM_KEY = re.compile(
    rb"-----BEGIN (?:RSA )?PRIVATE KEY-----\r?\n.*?\r?\n-----END (?:RSA )?PRIVATE KEY-----",
    re.DOTALL,
)
RE_PEM_CERT = re.compile(
    rb"-----BEGIN CERTIFICATE-----\r?\n.*?\r?\n-----END CERTIFICATE-----",
    re.DOTALL,
)
RE_ENC_BLOB = re.compile(rb"\b([0-9a-f]{32}):([0-9a-f]{64,})\b", re.IGNORECASE)


def load_mainjs_encryption_key(mainjs_path: Path) -> str | None:
    try:
        txt = mainjs_path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return None
    m = re.search(r"encryptionKey:\s*'([^']+)'", txt)
    return m.group(1) if m else None


def decrypt_conf_hex_blob(iv_hex: bytes, ct_hex: bytes, encryption_key: str) -> str | None:
    try:
        iv = bytes.fromhex(iv_hex.decode("ascii"))
        ct = bytes.fromhex(ct_hex.decode("ascii"))
    except Exception:
        return None
    if len(iv) != 16 or not ct:
        return None
    key = hashlib.sha256(encryption_key.encode("utf-8")).digest()
    try:
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv))
        dec = cipher.decryptor().update(ct) + cipher.decryptor().finalize()
    except Exception:
        return None
    if not dec:
        return None
    pad = dec[-1]
    if pad <= 0 or pad > 16:
        return None
    if dec[-pad:] != bytes([pad]) * pad:
        return None
    plain = dec[:-pad]
    try:
        return plain.decode("utf-8")
    except Exception:
        return None


def extract_from_json_text(text: str) -> tuple[str | None, str | None]:
    key = None
    cert = None
    try:
        obj = json.loads(text)
        if isinstance(obj, dict):
            ac = obj.get("appCert")
            if isinstance(ac, dict):
                k = ac.get("privateKey")
                c = ac.get("cert")
                if isinstance(k, str) and "BEGIN" in k:
                    key = k
                if isinstance(c, str) and "BEGIN CERTIFICATE" in c:
                    cert = c
    except Exception:
        pass
    return key, cert


def scan_files(base_dir: Path, encryption_key: str | None) -> tuple[str | None, str | None]:
    found_key: str | None = None
    found_cert: str | None = None
    for p in base_dir.rglob("*"):
        if not p.is_file():
            continue
        try:
            raw = p.read_bytes()
        except Exception:
            continue

        # 1) Direct PEM match in binary files
        if found_key is None:
            m_key = RE_PEM_KEY.search(raw)
            if m_key:
                found_key = m_key.group(0).decode("utf-8", errors="ignore")
                print(f"[HIT] private key PEM in {p}")
        if found_cert is None:
            m_cert = RE_PEM_CERT.search(raw)
            if m_cert:
                found_cert = m_cert.group(0).decode("utf-8", errors="ignore")
                print(f"[HIT] cert PEM in {p}")
        if found_key and found_cert:
            return found_key, found_cert

        # 2) Try encrypted electron-store style blobs iv_hex:cipher_hex
        if encryption_key:
            for iv_hex, ct_hex in RE_ENC_BLOB.findall(raw):
                plain = decrypt_conf_hex_blob(iv_hex, ct_hex, encryption_key)
                if not plain:
                    continue
                k, c = extract_from_json_text(plain)
                if k and not found_key:
                    found_key = k
                    print(f"[HIT] decrypted privateKey from {p}")
                if c and not found_cert:
                    found_cert = c
                    print(f"[HIT] decrypted cert from {p}")
                if found_key and found_cert:
                    return found_key, found_cert

        # 3) Fallback: parse text portions as JSON
        try:
            txt = raw.decode("utf-8", errors="ignore")
        except Exception:
            txt = ""
        if txt:
            k, c = extract_from_json_text(txt)
            if k and not found_key:
                found_key = k
                print(f"[HIT] json privateKey in {p}")
            if c and not found_cert:
                found_cert = c
                print(f"[HIT] json cert in {p}")
            if found_key and found_cert:
                return found_key, found_cert
    return found_key, found_cert


def main() -> int:
    ap = argparse.ArgumentParser(description="Extract appCert/privateKey from Bambu Connect storage")
    ap.add_argument("--roaming-dir", default=str(DEFAULT_ROAMING_DIR))
    ap.add_argument("--mainjs", default=str(DEFAULT_MAINJS_FILE))
    ap.add_argument("--out-dir", default=str(DEFAULT_OUT_DIR))
    args = ap.parse_args()

    base_dir = Path(args.roaming_dir)
    mainjs = Path(args.mainjs)
    out_dir = Path(args.out_dir)

    if not base_dir.exists():
        print(f"[ERR] roaming dir not found: {base_dir}")
        return 1

    enc_key = load_mainjs_encryption_key(mainjs)
    if enc_key:
        print("[INFO] encryptionKey loaded from main.js")
    else:
        print("[WARN] encryptionKey not found in main.js")

    key_pem, cert_pem = scan_files(base_dir, enc_key)
    if not key_pem:
        print("[ERR] private key not found")
        return 2
    if not cert_pem:
        print("[ERR] cert not found")
        return 3

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "app_private_key.pem").write_text(key_pem, encoding="utf-8")
    (out_dir / "app_cert.pem").write_text(cert_pem, encoding="utf-8")
    print(f"[OK] wrote {(out_dir / 'app_private_key.pem')}")
    print(f"[OK] wrote {(out_dir / 'app_cert.pem')}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
