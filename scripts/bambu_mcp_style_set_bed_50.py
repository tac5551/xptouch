#!/usr/bin/env python3
from __future__ import annotations

import base64
import json
import re
import socket
import ssl
import time
from pathlib import Path

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

REGION = "global"
DEVICE_ID = "31B8AP611301485"
PROVISIONING_FILE = "scripts/provisioning.json"
CA_BUNDLE_FILE = "scripts/certs_out/ca_bundle.pem"
MAINJS_FILE = "../trush/main.js.txt"

# bambu-mcp default app cert id
APP_CERT_ID = "GLOF3813734089-524a37c80000c6a6a274a47b3281"
TARGET_BED_TEMP = 50
WAIT_SECONDS = 20


def build_mqtt_host(region: str) -> str:
    return "cn.mqtt.bambulab.com" if region.lower() == "cn" else "us.mqtt.bambulab.com"


def load_provisioning(path: Path) -> tuple[str | None, str | None]:
    try:
        obj = json.loads(path.read_text(encoding="utf-8", errors="ignore"))
        if not isinstance(obj, dict):
            return None, None
        u = obj.get("cloud-username")
        p = obj.get("cloud-authToken")
        return (u if isinstance(u, str) and u else None, p if isinstance(p, str) and p else None)
    except Exception:
        return None, None


def load_mainjs_private_key(path: Path) -> str | None:
    try:
        txt = path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return None
    m = re.search(r"privateKey:\s*'((?:\\.|[^'])*)'", txt, re.DOTALL)
    if not m:
        return None
    return m.group(1).replace("\\n", "\n").replace("\\r", "\r")


def mqtt_utf8(s: str) -> bytes:
    b = s.encode("utf-8")
    return len(b).to_bytes(2, "big") + b


def mqtt_rem_len(n: int) -> bytes:
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


def mqtt_publish(topic: str, payload_text: str) -> bytes:
    t = mqtt_utf8(topic)
    p = payload_text.encode("utf-8")
    return b"\x30" + mqtt_rem_len(len(t) + len(p)) + t + p


def read_exact(sock: ssl.SSLSocket, n: int) -> bytes:
    out = bytearray()
    while len(out) < n:
        c = sock.recv(n - len(out))
        if not c:
            break
        out.extend(c)
    return bytes(out)


def read_remaining(sock: ssl.SSLSocket) -> int:
    mul = 1
    val = 0
    for _ in range(4):
        b = read_exact(sock, 1)
        if not b:
            return -1
        c = b[0]
        val += (c & 0x7F) * mul
        if (c & 0x80) == 0:
            return val
        mul *= 128
    return -1


def sign_b64(private_key_pem: str, payload_text: str) -> str:
    key = serialization.load_pem_private_key(private_key_pem.encode("utf-8"), password=None)
    sig = key.sign(payload_text.encode("utf-8"), padding.PKCS1v15(), hashes.SHA256())
    return base64.b64encode(sig).decode("ascii")


def build_signed_payload(user_id: str, private_key: str, bed_temp: int) -> tuple[str, str]:
    sequence_id = str(int(time.time() * 1000))
    payload = {
        "print": {
            "sequence_id": sequence_id,
            "command": "set_bed_temper",
            "timestamp": int(time.time() * 1000),
            "bed_target_temper": bed_temp,
        },
        "user_id": user_id,
    }
    payload_str = json.dumps(payload, separators=(",", ":"))
    signed = {
        **payload,
        "header": {
            "sign_ver": "v1.0",
            "sign_alg": "RSA_SHA256",
            "sign_string": sign_b64(private_key, payload_str),
            "cert_id": APP_CERT_ID,
            "payload_len": len(payload_str.encode("utf-8")),
        },
    }
    return sequence_id, json.dumps(signed, separators=(",", ":"))


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    prov = root / PROVISIONING_FILE
    ca_file = root / CA_BUNDLE_FILE
    mainjs = (root / MAINJS_FILE).resolve()
    username, password = load_provisioning(prov)
    if not username or not password:
        print(f"[ERR] missing cloud credentials in {prov}")
        return 1
    private_key = load_mainjs_private_key(mainjs)
    if not private_key:
        print(f"[ERR] private key not found in {mainjs}")
        return 1

    user_id = username[2:] if username.startswith("u_") else username
    host = build_mqtt_host(REGION)
    request_topic = f"device/{DEVICE_ID}/request"
    report_topic = f"device/{DEVICE_ID}/report"

    if ca_file.exists():
        ctx = ssl.create_default_context()
        ctx.check_hostname = True
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(cadata=ca_file.read_text(encoding="utf-8", errors="ignore"))
    else:
        ctx = ssl.create_default_context()

    with socket.create_connection((host, 8883), timeout=10) as sock:
        with ctx.wrap_socket(sock, server_hostname=host) as ssock:
            ssock.settimeout(1.0)

            client_id = f"mcp-style-{int(time.time())}"
            vh = mqtt_utf8("MQTT") + b"\x04"
            flags = 0x02 | 0x80 | 0x40
            keep_alive = (30).to_bytes(2, "big")
            payload = mqtt_utf8(client_id) + mqtt_utf8(username) + mqtt_utf8(password)
            ssock.sendall(
                b"\x10" + mqtt_rem_len(len(vh) + 1 + 2 + len(payload)) + vh + bytes([flags]) + keep_alive + payload
            )
            connack = read_exact(ssock, 4)
            if len(connack) < 4 or connack[0] != 0x20 or connack[3] != 0:
                print(f"[ERR] connack={connack.hex() if connack else 'empty'}")
                return 1

            sub_payload = mqtt_utf8(report_topic) + b"\x00"
            sub = b"\x82" + mqtt_rem_len(2 + len(sub_payload)) + b"\x00\x01" + sub_payload
            ssock.sendall(sub)
            _ = read_exact(ssock, 1)
            rl = read_remaining(ssock)
            _ = read_exact(ssock, rl if rl > 0 else 0)

            sequence_id, req_text = build_signed_payload(user_id, private_key, TARGET_BED_TEMP)
            print(f"[SEC] cert_id={APP_CERT_ID}")
            print(f"[SEND_JSON] {req_text}")
            ssock.sendall(mqtt_publish(request_topic, req_text))
            print(f"[SEND] set_bed_temper {TARGET_BED_TEMP}")

            deadline = time.time() + WAIT_SECONDS
            while time.time() < deadline:
                try:
                    fh = read_exact(ssock, 1)
                except socket.timeout:
                    continue
                if not fh:
                    continue
                typ = fh[0] >> 4
                rem = read_remaining(ssock)
                body = read_exact(ssock, rem if rem > 0 else 0)
                if typ != 3 or len(body) < 2:
                    continue
                tlen = int.from_bytes(body[:2], "big")
                topic = body[2 : 2 + tlen].decode("utf-8", errors="replace")
                msg = body[2 + tlen :].decode("utf-8", errors="replace")
                if topic != report_topic:
                    continue
                print(f"[MSG] {msg}")
                try:
                    obj = json.loads(msg)
                except Exception:
                    continue
                pr = obj.get("print") if isinstance(obj, dict) else None
                if isinstance(pr, dict) and pr.get("command") == "set_bed_temper" and pr.get("sequence_id") == sequence_id:
                    return 0
            print("[ERR] no matching response")
            return 1


if __name__ == "__main__":
    raise SystemExit(main())
