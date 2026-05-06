#!/usr/bin/env python3
from __future__ import annotations

import json
import base64
import hashlib
import re
import socket
import ssl
import time
from pathlib import Path
from collections import OrderedDict

from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding

DEFAULT_REGION = "global"
DEFAULT_PROVISIONING_FILE = "scripts/provisioning.json"
DEFAULT_CA_BUNDLE_FILE = "scripts/certs_out/ca_bundle.pem"
DEFAULT_DEVICE_ID = "31B8AP611301485"
DEFAULT_MAINJS_FILE = "../trush/main.js.txt"


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


def mqtt_publish_qos0(topic: str, payload_text: str) -> bytes:
    topic_b = mqtt_utf8(topic)
    payload_b = payload_text.encode("utf-8")
    return b"\x30" + mqtt_rem_len(len(topic_b) + len(payload_b)) + topic_b + payload_b


def load_mainjs_private_key(path: Path) -> str | None:
    try:
        txt = path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return None
    m = re.search(r"privateKey:\s*'((?:\\.|[^'])*)'", txt, re.DOTALL)
    if not m:
        return None
    return m.group(1).replace("\\n", "\n").replace("\\r", "\r")


def rsa_sha256_b64(private_key_pem: str, payload_text: str) -> str:
    key = serialization.load_pem_private_key(private_key_pem.encode("utf-8"), password=None)
    sig = key.sign(payload_text.encode("utf-8"), padding.PKCS1v15(), hashes.SHA256())
    return base64.b64encode(sig).decode("ascii")


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    prov = root / DEFAULT_PROVISIONING_FILE
    ca_file = root / DEFAULT_CA_BUNDLE_FILE
    username, password = load_provisioning(prov)
    if not username or not password:
        print(f"[ERR] missing cloud-username/cloud-authToken in {prov}")
        return 1
    host = build_mqtt_host(DEFAULT_REGION)
    topic = f"device/{DEFAULT_DEVICE_ID}/report"
    request_topic = f"device/{DEFAULT_DEVICE_ID}/request"
    mainjs_path = (root / DEFAULT_MAINJS_FILE).resolve()
    app_private_key = load_mainjs_private_key(mainjs_path)
    if not app_private_key:
        print(f"[ERR] app private key not found in {mainjs_path}")
        return 1

    if ca_file.exists():
        ctx = ssl.create_default_context()
        ctx.check_hostname = True
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(cadata=ca_file.read_text(encoding="utf-8", errors="ignore"))
    else:
        ctx = ssl.create_default_context()

    print(f"[WATCH] host={host} topic={topic}")
    with socket.create_connection((host, 8883), timeout=10) as sock:
        with ctx.wrap_socket(sock, server_hostname=host) as ssock:
            ssock.settimeout(1.0)
            client_id = f"xptouch-watch-{int(time.time())}"
            vh = mqtt_utf8("MQTT") + b"\x04"
            connect_flags = 0x02 | 0x80 | 0x40
            keep_alive = (30).to_bytes(2, "big")
            payload = mqtt_utf8(client_id) + mqtt_utf8(username) + mqtt_utf8(password)
            pkt = b"\x10" + mqtt_rem_len(len(vh) + 1 + 2 + len(payload)) + vh + bytes([connect_flags]) + keep_alive + payload
            ssock.sendall(pkt)
            connack = read_exact(ssock, 4)
            if len(connack) < 4 or connack[0] != 0x20 or connack[3] != 0:
                print(f"[ERR] connack={connack.hex() if connack else 'empty'}")
                return 1
            print("[WATCH] connected")

            sub_payload = mqtt_utf8(topic) + b"\x00"
            sub = b"\x82" + mqtt_rem_len(2 + len(sub_payload)) + b"\x00\x01" + sub_payload
            ssock.sendall(sub)
            h = read_exact(ssock, 1)
            rl = read_remaining(ssock)
            body = read_exact(ssock, rl if rl > 0 else 0)
            if not h or h[0] != 0x90 or len(body) < 3 or body[2] == 0x80:
                print(f"[ERR] suback hdr={h.hex() if h else 'empty'} body={body.hex()}")
                return 1
            print("[WATCH] subscribed")
            seq = int(time.time())
            cert_id = None

            # ask app_cert_list first to obtain active cert_id
            sec_seq = str(seq)
            sec_req = {
                "security": {
                    "sequence_id": sec_seq,
                    "command": "app_cert_list",
                    "timestamp": int(time.time() * 1000),
                    "type": "app",
                }
            }
            ssock.sendall(mqtt_publish_qos0(request_topic, json.dumps(sec_req, separators=(",", ":"))))
            print(f"[SEC] request app_cert_list sequence_id={sec_seq}")

            while True:
                try:
                    fh = read_exact(ssock, 1)
                except socket.timeout:
                    continue
                if not fh:
                    continue
                typ = fh[0] >> 4
                rem = read_remaining(ssock)
                if rem < 0:
                    print("[ERR] remaining length parse failed")
                    return 1
                payload = read_exact(ssock, rem)
                if typ == 3:  # PUBLISH
                    tlen = int.from_bytes(payload[:2], "big")
                    t = payload[2 : 2 + tlen].decode("utf-8", errors="replace")
                    msg = payload[2 + tlen :].decode("utf-8", errors="replace")
                    print(f"[MSG] topic={t} payload={msg}")
                    try:
                        obj = json.loads(msg)
                        sec = obj.get("security") if isinstance(obj, dict) else None
                        if isinstance(sec, dict) and sec.get("command") == "app_cert_list":
                            cert_ids = sec.get("cert_ids")
                            if isinstance(cert_ids, list) and cert_ids:
                                cert_id = str(cert_ids[0])
                                print(f"[SEC] active cert_id={cert_id}")
                    except Exception:
                        pass
                elif typ == 13:  # PINGRESP
                    print("[WATCH] pingresp")
                elif typ == 9:  # SUBACK
                    print(f"[WATCH] suback body={payload.hex()}")
                else:
                    print(f"[WATCH] packet type={typ} len={rem}")


if __name__ == "__main__":
    raise SystemExit(main())
