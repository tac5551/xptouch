#!/usr/bin/env python3
from __future__ import annotations

import base64
import json
import os
import re
import socket
import ssl
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

# Parameter-less strict configuration
DEFAULT_REGION = "global"
DEFAULT_TIMEOUT = 8.0
DEFAULT_USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36"
# main.js faithful path: cert_api uses only materials from main.js.txt
DEFAULT_SERVER_CERT_FILE = "scripts/certs_out/server_cert.pem"
DEFAULT_APP_CERT_FILE = "scripts/certs_out/app_cert_candidate.pem"
DEFAULT_CA_BUNDLE_FILE = "scripts/certs_out/ca_bundle.pem"
DEFAULT_APP_PRIVATE_KEY_FILE = "scripts/certs_out/app_private_key_candidate.pem"
DEFAULT_PROVISIONING_FILE = "scripts/provisioning.json"
DEFAULT_MAINJS_FILE = "../trush/main.js.txt"
DEFAULT_MQTT_SUB_DEVICE_ID = "31B8AP611301485"
APP_ACCEPT_LANGUAGE = "ja,en-US;q=0.9,en;q=0.8"
APP_ORIGIN = "app://bambu-connect"
APP_REFERER = "app://bambu-connect/"
APP_SEC_FETCH_MODE = "cors"
APP_SEC_FETCH_DEST = "empty"


def b64url_keep_padding(raw: bytes) -> str:
    return base64.b64encode(raw).decode("ascii").replace("+", "-").replace("/", "_")


def build_hosts(region: str) -> str:
    return "api.bambulab.cn" if region.lower() == "cn" else "api.bambulab.com"


def build_mqtt_host(region: str) -> str:
    return "cn.mqtt.bambulab.com" if region.lower() == "cn" else "us.mqtt.bambulab.com"


def _unescape_js_string(s: str) -> str:
    return s.replace("\\n", "\n").replace("\\r", "\r")


def load_mainjs_materials(path: Path) -> dict[str, str] | None:
    try:
        txt = path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return None
    m_are = re.search(r"const\s+Are\s*=\s*'([^']+)'", txt)
    m_fre = re.search(r"const\s+fre\s*=\s*'((?:\\.|[^'])*)'", txt, re.DOTALL)
    m_pk = re.search(r"privateKey:\s*'((?:\\.|[^'])*)'", txt, re.DOTALL)
    m_cert = re.search(r"cert:\s*'((?:\\.|[^'])*)'", txt, re.DOTALL)
    if not (m_are and m_fre and m_pk and m_cert):
        return None
    return {
        "are": m_are.group(1),
        "server_cert": _unescape_js_string(m_fre.group(1)),
        "app_private_key": _unescape_js_string(m_pk.group(1)),
        "app_cert": _unescape_js_string(m_cert.group(1)),
    }


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
    req.add_header("Accept-Language", APP_ACCEPT_LANGUAGE)
    req.add_header("Origin", APP_ORIGIN)
    req.add_header("Referer", APP_REFERER)
    req.add_header("Sec-Fetch-Mode", APP_SEC_FETCH_MODE)
    req.add_header("Sec-Fetch-Dest", APP_SEC_FETCH_DEST)
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


def load_provisioning_credentials(path: Path) -> tuple[str | None, str | None]:
    try:
        obj = json.loads(path.read_text(encoding="utf-8", errors="ignore"))
        if not isinstance(obj, dict):
            return None, None
        username = obj.get("cloud-username")
        auth_token = obj.get("cloud-authToken")
        return (
            str(username) if isinstance(username, str) and username else None,
            str(auth_token) if isinstance(auth_token, str) and auth_token else None,
        )
    except Exception:
        return None, None


def load_provisioning_bearer_token(path: Path) -> str | None:
    try:
        obj = json.loads(path.read_text(encoding="utf-8", errors="ignore"))
        if not isinstance(obj, dict):
            return None
        token = obj.get("cloud-authToken")
        return str(token) if isinstance(token, str) and token else None
    except Exception:
        return None


def resolve_cert_paths(server_cert_file: str, app_cert_file: str) -> tuple[Path, Path]:
    repo_root = Path(__file__).resolve().parent.parent
    server = Path(server_cert_file)
    app = Path(app_cert_file)
    if not server.is_absolute():
        server = repo_root / server
    if not app.is_absolute():
        app = repo_root / app
    return server, app


def tls_handshake_mqtt(host: str, port: int, timeout: float, ca_pem_text: str | None) -> tuple[bool, str]:
    try:
        if ca_pem_text:
            ctx = ssl.create_default_context()
            ctx.check_hostname = True
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(cadata=ca_pem_text)
        else:
            ctx = ssl.create_default_context()
        with socket.create_connection((host, port), timeout=timeout) as sock:
            with ctx.wrap_socket(sock, server_hostname=host):
                return True, "handshake ok"
    except Exception as exc:
        return False, str(exc)


def _mqtt_encode_remaining_length(n: int) -> bytes:
    out = bytearray()
    while True:
        b = n % 128
        n //= 128
        if n > 0:
            b |= 0x80
        out.append(b)
        if n == 0:
            break
    return bytes(out)


def _mqtt_utf8(s: str) -> bytes:
    b = s.encode("utf-8")
    return len(b).to_bytes(2, "big") + b


def _mqtt_read_exact(sock: ssl.SSLSocket, n: int) -> bytes:
    out = bytearray()
    while len(out) < n:
        chunk = sock.recv(n - len(out))
        if not chunk:
            break
        out.extend(chunk)
    return bytes(out)


def _mqtt_read_remaining_length(sock: ssl.SSLSocket) -> int:
    mul = 1
    val = 0
    for _ in range(4):
        b = _mqtt_read_exact(sock, 1)
        if not b:
            return -1
        c = b[0]
        val += (c & 0x7F) * mul
        if (c & 0x80) == 0:
            return val
        mul *= 128
    return -1


def mqtt_connect_check(
    host: str,
    port: int,
    timeout: float,
    ca_pem_text: str | None,
    username: str,
    password: str,
    client_id: str,
) -> tuple[bool, str]:
    try:
        if ca_pem_text:
            ctx = ssl.create_default_context()
            ctx.check_hostname = True
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(cadata=ca_pem_text)
        else:
            ctx = ssl.create_default_context()
        with socket.create_connection((host, port), timeout=timeout) as sock:
            with ctx.wrap_socket(sock, server_hostname=host) as ssock:
                vh = _mqtt_utf8("MQTT") + b"\x04"  # protocol level 4 (3.1.1)
                connect_flags = 0x02 | 0x80 | 0x40  # clean session + username + password
                keep_alive = (30).to_bytes(2, "big")
                payload = _mqtt_utf8(client_id) + _mqtt_utf8(username) + _mqtt_utf8(password)
                remaining = len(vh) + 1 + 2 + len(payload)
                pkt = b"\x10" + _mqtt_encode_remaining_length(remaining) + vh + bytes([connect_flags]) + keep_alive + payload
                ssock.sendall(pkt)
                hdr = ssock.recv(4)
                if len(hdr) < 4 or hdr[0] != 0x20 or hdr[1] != 0x02:
                    return False, f"invalid CONNACK: {hdr.hex() if hdr else 'empty'}"
                rc = hdr[3]
                if rc != 0:
                    return False, f"CONNACK return_code={rc}"
                try:
                    ssock.sendall(b"\xE0\x00")  # DISCONNECT
                except Exception:
                    pass
                return True, "connack accepted"
    except Exception as exc:
        return False, str(exc)


def mqtt_subscribe_check(
    host: str,
    port: int,
    timeout: float,
    ca_pem_text: str | None,
    username: str,
    password: str,
    client_id: str,
    device_id: str,
) -> tuple[bool, str]:
    topic = f"device/{device_id}/report"
    try:
        if ca_pem_text:
            ctx = ssl.create_default_context()
            ctx.check_hostname = True
            ctx.verify_mode = ssl.CERT_REQUIRED
            ctx.load_verify_locations(cadata=ca_pem_text)
        else:
            ctx = ssl.create_default_context()
        with socket.create_connection((host, port), timeout=timeout) as sock:
            with ctx.wrap_socket(sock, server_hostname=host) as ssock:
                # CONNECT
                vh = _mqtt_utf8("MQTT") + b"\x04"
                connect_flags = 0x02 | 0x80 | 0x40
                keep_alive = (30).to_bytes(2, "big")
                payload = _mqtt_utf8(client_id) + _mqtt_utf8(username) + _mqtt_utf8(password)
                remaining = len(vh) + 1 + 2 + len(payload)
                pkt = b"\x10" + _mqtt_encode_remaining_length(remaining) + vh + bytes([connect_flags]) + keep_alive + payload
                ssock.sendall(pkt)
                connack = _mqtt_read_exact(ssock, 4)
                if len(connack) < 4 or connack[0] != 0x20 or connack[1] != 0x02 or connack[3] != 0:
                    return False, f"connect failed: {connack.hex() if connack else 'empty'}"

                # SUBSCRIBE packet id=1, qos0
                pid = b"\x00\x01"
                sub_payload = _mqtt_utf8(topic) + b"\x00"
                sub_var = pid
                sub_remaining = len(sub_var) + len(sub_payload)
                sub_pkt = b"\x82" + _mqtt_encode_remaining_length(sub_remaining) + sub_var + sub_payload
                ssock.sendall(sub_pkt)

                # SUBACK
                h = _mqtt_read_exact(ssock, 1)
                if not h:
                    return False, "suback header empty"
                remaining_len = _mqtt_read_remaining_length(ssock)
                if remaining_len < 0:
                    return False, "suback remaining length parse failed"
                body = _mqtt_read_exact(ssock, remaining_len)
                if h[0] != 0x90 or len(body) < 3:
                    return False, f"invalid suback: hdr={h.hex()} body={body.hex()}"
                if body[2] == 0x80:
                    return False, f"suback rejected topic={topic}"
                try:
                    ssock.sendall(b"\xE0\x00")
                except Exception:
                    pass
                return True, f"suback accepted topic={topic} qos={body[2]}"
    except Exception as exc:
        return False, str(exc)


def run_cert_api_check(
    label: str,
    region: str,
    timeout: float,
    user_agent: str,
    bearer_token: str | None,
    are_plain: str,
    server_cert: str,
    app_cert: str,
    app_key: str,
) -> bool:
    if not app_key.strip() or not app_cert.strip() or not server_cert.strip():
        print(f"[API:{label}] cert_api success=False (missing app/server cert or private key)")
        return False
    cm = CertManager(are_plain.encode("utf-8"), server_cert, app_key, app_cert)
    enc_app_key, aes256 = cm.encrypt_app_key()
    url = f"https://{build_hosts(region)}/v1/iot-service/api/user/applications/{enc_app_key}/cert?{parse.urlencode({'aes256': aes256})}"
    req = request.Request(url, method="GET")
    if bearer_token:
        req.add_header("Authorization", f"Bearer {bearer_token}")
    req.add_header("x-bbl-app-certification-id", cm.get_sign_cert_id())
    req.add_header("x-bbl-device-security-sign", cm.private_encrypt_timestamp_b64())
    req.add_header("User-Agent", user_agent)
    req.add_header("Accept", "application/json, text/plain, */*")
    req.add_header("Accept-Language", APP_ACCEPT_LANGUAGE)
    req.add_header("Origin", APP_ORIGIN)
    req.add_header("Referer", APP_REFERER)
    req.add_header("Sec-Fetch-Mode", APP_SEC_FETCH_MODE)
    req.add_header("Sec-Fetch-Dest", APP_SEC_FETCH_DEST)
    try:
        with request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("utf-8", errors="replace")
            ok = 200 <= resp.status < 300
            print(f"[API:{label}] http_status={resp.status}")
            if body:
                print(f"[API:{label}] response={body}")
    except Exception:
        ok = False
        try:
            if isinstance(sys.exc_info()[1], error.HTTPError):
                exc = sys.exc_info()[1]
                body = exc.read().decode("utf-8", errors="replace")
                print(f"[API:{label}] http_status={exc.code}")
                if body:
                    print(f"[API:{label}] response={body}")
        except Exception:
            pass
    print(f"[API:{label}] cert_api success={ok}")
    return ok


def main() -> int:
    region = DEFAULT_REGION
    timeout = DEFAULT_TIMEOUT
    user_agent = DEFAULT_USER_AGENT
    device_id = None
    mqtt_check = True
    mqtt_auth_check = True
    mqtt_username = os.environ.get("BAMBU_CLOUD_USERNAME")
    mqtt_password = os.environ.get("BAMBU_CLOUD_AUTHTOKEN")
    bearer_token = None
    print(f"region={region}")

    prov_path = Path(__file__).resolve().parent.parent / DEFAULT_PROVISIONING_FILE

    if not bearer_token and prov_path.exists():
        p_bearer = load_provisioning_bearer_token(prov_path)
        if p_bearer:
            bearer_token = p_bearer
            print(f"[API] bearer token loaded: {prov_path}")

    if mqtt_auth_check and (not mqtt_username or not mqtt_password):
        if prov_path.exists():
            p_user, p_pass = load_provisioning_credentials(prov_path)
            if not mqtt_username and p_user:
                mqtt_username = p_user
            if not mqtt_password and p_pass:
                mqtt_password = p_pass
            print(f"[MQTT] provisioning loaded: {prov_path}")

    mj = Path(__file__).resolve().parent.parent / DEFAULT_MAINJS_FILE

    mainjs_data = load_mainjs_materials(mj) if mj.exists() else None
    if mainjs_data:
        print(f"[API:mainjs] main.js materials loaded: {mj}")
        cert_ok = run_cert_api_check(
            "mainjs",
            region,
            timeout,
            user_agent,
            bearer_token,
            mainjs_data["are"],
            mainjs_data["server_cert"],
            mainjs_data["app_cert"],
            mainjs_data["app_private_key"],
        )
    else:
        print(f"[API:mainjs] main.js materials missing or parse failed: {mj}")
        cert_ok = False

    b_status, b_payload = call_bind_api(region, bearer_token, user_agent, timeout)
    print(f"[BIND] status={b_status}")
    print_bind_devices(b_payload, device_id)

    mqtt_ok = False
    mqtt_auth_ok = False
    mqtt_sub_ok = False
    if mqtt_check:
        ca_path = Path(__file__).resolve().parent.parent / DEFAULT_CA_BUNDLE_FILE
        ca_text = ca_path.read_text(encoding="utf-8", errors="ignore") if ca_path.exists() else None
        mqtt_host = build_mqtt_host(region)
        mqtt_ok, mqtt_msg = tls_handshake_mqtt(mqtt_host, 8883, timeout, ca_text)
        print(f"[MQTT] host={mqtt_host} tls_ok={mqtt_ok} detail={mqtt_msg}")
        if mqtt_auth_check:
            if not mqtt_username or not mqtt_password:
                print("[MQTT] auth_check skipped: set --mqtt-username/--mqtt-password or BAMBU_CLOUD_USERNAME/BAMBU_CLOUD_AUTHTOKEN")
            elif mqtt_ok:
                client_id = f"xptouch-test-{int(time.time())}"
                mqtt_auth_ok, auth_msg = mqtt_connect_check(
                    mqtt_host, 8883, timeout, ca_text, mqtt_username, mqtt_password, client_id
                )
                print(f"[MQTT] auth_ok={mqtt_auth_ok} detail={auth_msg}")
                if mqtt_auth_ok:
                    sub_id = f"xptouch-sub-{int(time.time())}"
                    mqtt_sub_ok, sub_msg = mqtt_subscribe_check(
                        mqtt_host,
                        8883,
                        timeout,
                        ca_text,
                        mqtt_username,
                        mqtt_password,
                        sub_id,
                        DEFAULT_MQTT_SUB_DEVICE_ID,
                    )
                    print(f"[MQTT] subscribe_ok={mqtt_sub_ok} detail={sub_msg}")

    mqtt_ready = True
    if mqtt_check:
        mqtt_ready = mqtt_ready and mqtt_ok
    if mqtt_auth_check:
        mqtt_ready = mqtt_ready and mqtt_auth_ok
    mqtt_ready = mqtt_ready and mqtt_sub_ok
    # cert_api は継続調査、統合判定は先に MQTT 成功を優先する。
    success = mqtt_ready
    print(
        f"RESULT success={success} cert_api={cert_ok} cert_api_mainjs={cert_ok} mqtt_ready={mqtt_ready} "
        f"mqtt_tls={mqtt_ok if mqtt_check else 'skip'} "
        f"mqtt_auth={mqtt_auth_ok if mqtt_auth_check else 'skip'}"
    )
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())

