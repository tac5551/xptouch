#!/usr/bin/env python3
from __future__ import annotations

import argparse
import base64
import json
import os
import sys
import time
from pathlib import Path
from urllib import error, parse, request

from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

FIXED_ARE = "GLOF3813734089-524a37c80000c6a6a274a47b3281"
FIXED_APP_PRIVATE_KEY = """-----BEGIN PRIVATE KEY-----
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDQNp2NfkajwcWH
PIqosa08P1ZwETPr1veZCMqieQxWtYw97wp+JCxX4yBrBcAwid7o7PHI9KQVzPRM
f0uXspaDUdSljrfJ/YwGEz7+GJz4+ml1UbWXBePyzXW1+N2hIGGn7BcNuA0v8rMY
uvVgiIIQNjLErgGcCWmMHLwsMMQ7LNprUZZKsSNB4HaQDH7cQZmYBN/O45np6l+K
VuLdzXdDpZcOM7bNO6smev822WPGDuKBo1iVfQbUe10X4dCNwkBR3QGpScVvg8gg
tRYZDYue/qc4Xaj806RZPttknWfxdvfZgoOmAiwnyQ5K3+mzNYHgQZAOC2ydkK4J
s+ZizK3lAgMBAAECggEAKwEcyXyrWmdLRQNcIDuSbD8ouzzSXIOp4BHQyH337nDQ
5nnY0PTns79VksU9TMktIS7PQZJF0brjOmmQU2SvcbAVG5y+mRmlMhwHhrPOuB4A
ahrWRrsQubV1+n/MRttJUEWS/WJmVuDp3NHAnI+VTYPkOHs4GeJXynik5PutjAr3
tYmr3kaw0Wo/hYAXTKsI/R5aenC7jH8ZSyVcZ/j+bOSH5sT5/JY122AYmkQOFE7s
JA0EfYJaJEwiuBWKOfRLQVEHhOFodUBZdGQcWeW3uFb88aYKN8QcKTO8/f6e4r8w
QojgK3QMj1zmfS7xid6XCOVa17ary2hZHAEPnjcigQKBgQDQnm4TlbVTsM+CbFUS
1rOIJRzPdnH3Y7x3IcmVKZt81eNktsdu56A4U6NEkFQqk4tVTT4TYja/hwgXmm6w
J+w0WwZd445Bxj8PmaEr6Z/NSMYbCsi8pRelKWmlIMwD2YhtY/1xXD37zpOgN8oQ
ryTKZR2gljbPxdfhKS7YerLp2wKBgQD/gJt3Ds69j1gMDLnnPctjmhsPRXh7PQ0e
E9lqgFkx/vNuCuyRs6ymic2rBZmkdlpjsTJFmz1bwOzIvSRoH6kp0Mfyo6why5kr
upDf7zz+hlvaFewme8aDeV3ex9Wvt73D66nwAy5ABOgn+66vZJeo0Iq/tnCwK3a/
evTL9BOzPwKBgEUi7AnziEc3Bl4Lttnqa08INZcPgs9grzmv6dVUF6J0Y8qhxFAd
1Pw1w5raVfpSMU/QrGzSFKC+iFECLgKVCHOFYwPEgQWNRKLP4BjkcMAgiP63QTU7
ZS2oHsnJp7Ly6YKPK5Pg5O3JVSU4t+91i7TDc+EfRwTuZQ/KjSrS5u4XAoGBAP06
v9reSDVELuWyb0Yqzrxm7k7ScbjjJ28aCTAvCTguEaKNHS7DP2jHx5mrMT35N1j7
NHIcjFG2AnhqTf0M9CJHlQR9B4tvON5ISHJJsNAq5jpd4/G4V2XTEiBNOxKvL1tQ
5NrGrD4zHs0R+25GarGcDwg3j7RrP4REHv9NZ4ENAoGAY7Nuz6xKu2XUwuZtJP7O
kjsoDS7bjP95ddrtsRq5vcVjJ04avnjsr+Se9WDA//t7+eSeHjm5eXD7u0NtdqZo
WtSm8pmWySOPXMn9QQmdzKHg1NOxer//f1KySVunX1vftTStjsZH7dRCtBEePcqg
z5Av6MmEFDojtwTqvEZuhBM=
-----END PRIVATE KEY-----"""


def b64url_keep_padding(raw: bytes) -> str:
    return base64.b64encode(raw).decode("ascii").replace("+", "-").replace("/", "_")


def build_hosts(region: str) -> str:
    return "api.bambulab.cn" if region.lower() == "cn" else "api.bambulab.com"


def rsa_private_encrypt_pkcs1_v15_raw(private_key_pem: str, plain: bytes) -> bytes:
    key = serialization.load_pem_private_key(private_key_pem.encode("utf-8"), password=None)
    nums = key.private_numbers()
    n = nums.public_numbers.n
    d = nums.d
    k = (n.bit_length() + 7) // 8
    em = b"\x00\x01" + (b"\xff" * (k - len(plain) - 3)) + b"\x00" + plain
    m = int.from_bytes(em, "big")
    c = pow(m, d, n)
    return c.to_bytes(k, "big")


class CertManager:
    def __init__(self, are_plain: bytes, server_cert_pem: str, app_private_key_pem: str, app_cert_pem: str) -> None:
        self.are_plain = are_plain
        self.server_cert_pem = server_cert_pem
        self.random_key = os.urandom(32)
        self.app_private_key_pem = app_private_key_pem
        self.app_cert_pem = app_cert_pem

    def encrypt_app_key(self) -> tuple[str, str]:
        iv = os.urandom(12)
        enc = AESGCM(self.random_key).encrypt(iv, self.are_plain, None)
        enc_app_key = b64url_keep_padding(iv + enc)
        cert = x509.load_pem_x509_certificate(self.server_cert_pem.encode("utf-8"))
        encrypted_random_key = cert.public_key().encrypt(self.random_key, padding.PKCS1v15())
        aes256 = b64url_keep_padding(encrypted_random_key)
        return enc_app_key, aes256

    def get_sign_cert_id(self) -> str:
        cert = x509.load_pem_x509_certificate(self.app_cert_pem.encode("utf-8"))
        return f"{cert.issuer.rfc4514_string()}:{format(cert.serial_number, 'x')}"

    def private_encrypt_timestamp_b64(self) -> str:
        stamp = str(int(time.time() * 1000)).encode("utf-8")
        encrypted = rsa_private_encrypt_pkcs1_v15_raw(self.app_private_key_pem, stamp)
        return base64.b64encode(encrypted).decode("ascii")


def call_bind_api(region: str, bearer_token: str | None, user_agent: str, timeout: float) -> tuple[int, object]:
    url = f"https://{build_hosts(region)}/v1/iot-service/api/user/bind"
    req = request.Request(url, method="GET")
    if bearer_token:
        req.add_header("Authorization", f"Bearer {bearer_token}")
    req.add_header("User-Agent", user_agent)
    req.add_header("Accept", "application/json")
    try:
        with request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            return resp.status, json.loads(body) if body else {}
    except error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        try:
            return exc.code, json.loads(body) if body else {}
        except json.JSONDecodeError:
            return exc.code, {"raw": body}


def print_bind_devices(payload: object, only_device_id: str | None = None) -> None:
    if not isinstance(payload, dict):
        return
    devices = payload.get("devices")
    if not isinstance(devices, list):
        return
    print("[BIND] device summary:")
    for d in devices:
        if not isinstance(d, dict):
            continue
        dev_id = str(d.get("dev_id", ""))
        if only_device_id and dev_id != only_device_id:
            continue
        print(
            f"- {dev_id} | {d.get('name','')} | {d.get('dev_product_name', d.get('dev_model_name',''))} "
            f"| online={bool(d.get('online', False))} | access_code={d.get('dev_access_code','')}"
        )


def resolve_cert_paths(server_cert_file: str, app_cert_file: str) -> tuple[Path, Path]:
    repo_root = Path(__file__).resolve().parent.parent
    server = Path(server_cert_file)
    app = Path(app_cert_file)
    if not server.is_absolute():
        server = repo_root / server
    if not app.is_absolute():
        app = repo_root / app
    return server, app


def main() -> int:
    parser = argparse.ArgumentParser(description="Bambu Cloud connect test")
    parser.add_argument("--region", choices=["global", "cn"], default="global")
    parser.add_argument("--bearer-token", default=None)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--device-id", default=None)
    parser.add_argument("--cert-first", action="store_true")
    parser.add_argument("--user-agent", default="Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36")
    parser.add_argument("--server-cert-file", default="scripts/certs_out/server_cert.pem")
    parser.add_argument("--app-cert-file", default="scripts/certs_out/app_cert_candidate.pem")
    parser.add_argument("--app-private-key-file", default=None, help="Optional. If omitted, built-in test key is used.")
    args = parser.parse_args()

    print(f"region={args.region}")

    cert_ok = False
    if args.cert_first:
        server_path, app_path = resolve_cert_paths(args.server_cert_file, args.app_cert_file)
        if not server_path.exists() or not app_path.exists():
            print("[API] skipped: cert files not found")
        else:
            server_cert = server_path.read_text(encoding="utf-8", errors="ignore")
            app_cert = app_path.read_text(encoding="utf-8", errors="ignore")
            app_key = (
                Path(args.app_private_key_file).read_text(encoding="utf-8", errors="ignore")
                if args.app_private_key_file and Path(args.app_private_key_file).exists()
                else FIXED_APP_PRIVATE_KEY
            )
            cm = CertManager(FIXED_ARE.encode("utf-8"), server_cert, app_key, app_cert)
            enc_app_key, aes256 = cm.encrypt_app_key()
            url = f"https://{build_hosts(args.region)}/v1/iot-service/api/user/applications/{enc_app_key}/cert?{parse.urlencode({'aes256': aes256})}"
            req = request.Request(url, method="GET")
            if args.bearer_token:
                req.add_header("Authorization", f"Bearer {args.bearer_token}")
            req.add_header("x-bbl-app-certification-id", cm.get_sign_cert_id())
            req.add_header("x-bbl-device-security-sign", cm.private_encrypt_timestamp_b64())
            req.add_header("User-Agent", args.user_agent)
            try:
                with request.urlopen(req, timeout=args.timeout) as resp:
                    cert_ok = 200 <= resp.status < 300
            except Exception:
                cert_ok = False
            print(f"[API] cert_first success={cert_ok}")

    b_status, b_payload = call_bind_api(args.region, args.bearer_token, args.user_agent, args.timeout)
    print(f"[BIND] status={b_status}")
    print_bind_devices(b_payload, args.device_id)
    bind_ok = 200 <= b_status < 300
    print(f"RESULT success={cert_ok or bind_ok} cert_api={cert_ok} bind_fallback={bind_ok}")
    return 0 if (cert_ok or bind_ok) else 1


if __name__ == "__main__":
    sys.exit(main())

