#!/usr/bin/env python3
from __future__ import annotations

import json
import socket
import ssl
import time
from pathlib import Path

REGION = "global"
PROVISIONING_FILE = "scripts/provisioning.json"
CA_BUNDLE_FILE = "scripts/certs_out/ca_bundle.pem"
DEVICE_ID = "31B8AP611301485"
WAIT_SECONDS = 20


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


def host_for_region(region: str) -> str:
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


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    prov = root / PROVISIONING_FILE
    ca_file = root / CA_BUNDLE_FILE
    username, password = load_provisioning(prov)
    if not username or not password:
        print(f"[ERR] missing cloud credentials in {prov}")
        return 1

    host = host_for_region(REGION)
    report_topic = f"device/{DEVICE_ID}/report"
    request_topic = f"device/{DEVICE_ID}/request"

    if ca_file.exists():
        ctx = ssl.create_default_context()
        ctx.check_hostname = True
        ctx.verify_mode = ssl.CERT_REQUIRED
        ctx.load_verify_locations(cadata=ca_file.read_text(encoding="utf-8", errors="ignore"))
    else:
        ctx = ssl.create_default_context()

    seq = str(int(time.time()))
    req_obj = {
        "security": {
            "sequence_id": seq,
            "command": "app_cert_list",
            "timestamp": int(time.time() * 1000),
            "type": "app",
        }
    }
    req_payload = json.dumps(req_obj, separators=(",", ":"))

    print(f"[REQ] topic={request_topic} payload={req_payload}")
    with socket.create_connection((host, 8883), timeout=10) as sock:
        with ctx.wrap_socket(sock, server_hostname=host) as ssock:
            ssock.settimeout(1.0)
            client_id = f"xptouch-certlist-{int(time.time())}"
            vh = mqtt_utf8("MQTT") + b"\x04"
            flags = 0x02 | 0x80 | 0x40
            keep_alive = (30).to_bytes(2, "big")
            pl = mqtt_utf8(client_id) + mqtt_utf8(username) + mqtt_utf8(password)
            conn = b"\x10" + mqtt_rem_len(len(vh) + 1 + 2 + len(pl)) + vh + bytes([flags]) + keep_alive + pl
            ssock.sendall(conn)
            ack = read_exact(ssock, 4)
            if len(ack) < 4 or ack[0] != 0x20 or ack[3] != 0:
                print(f"[ERR] connack={ack.hex() if ack else 'empty'}")
                return 1

            sub_payload = mqtt_utf8(report_topic) + b"\x00"
            sub = b"\x82" + mqtt_rem_len(2 + len(sub_payload)) + b"\x00\x01" + sub_payload
            ssock.sendall(sub)
            h = read_exact(ssock, 1)
            rl = read_remaining(ssock)
            body = read_exact(ssock, rl if rl > 0 else 0)
            if not h or h[0] != 0x90 or len(body) < 3 or body[2] == 0x80:
                print(f"[ERR] suback hdr={h.hex() if h else 'empty'} body={body.hex()}")
                return 1

            pub_topic = mqtt_utf8(request_topic)
            pub = b"\x30" + mqtt_rem_len(len(pub_topic) + len(req_payload.encode("utf-8"))) + pub_topic + req_payload.encode("utf-8")
            ssock.sendall(pub)

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
                if rem < 0:
                    continue
                payload = read_exact(ssock, rem)
                if typ != 3 or len(payload) < 2:
                    continue
                tlen = int.from_bytes(payload[:2], "big")
                t = payload[2 : 2 + tlen].decode("utf-8", errors="replace")
                msg = payload[2 + tlen :].decode("utf-8", errors="replace")
                if t != report_topic:
                    continue
                try:
                    obj = json.loads(msg)
                except Exception:
                    continue
                sec = obj.get("security") if isinstance(obj, dict) else None
                if not isinstance(sec, dict):
                    continue
                if sec.get("command") == "app_cert_list":
                    print(f"[RES] {json.dumps(obj, ensure_ascii=False)}")
                    return 0

    print("[ERR] app_cert_list response not received within timeout")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
