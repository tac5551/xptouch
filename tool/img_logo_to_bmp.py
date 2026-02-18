#!/usr/bin/env python3
"""
LVGLの ui_image_*.c から画像データを抽出しBMPに変換する。

使い方:
  python img_logo_to_bmp.py [input.c] [output.bmp]
  省略時: src/ui/ui_image_logo.c -> tool/img_logo.bmp

対応フォーマット: LV_IMG_CF_RGB565A8 (3 bytes/pixel: RGB565 LE + A8)
"""

import re
import struct
import sys
from pathlib import Path

# デフォルトパス（リポジトリルート基準）
REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_INPUT = REPO_ROOT / "src" / "ui" / "ui_image_logo.c"
DEFAULT_OUTPUT = REPO_ROOT / "tool" / "img_logo.bmp"


def extract_bytes_from_c(content: str) -> list[int]:
    """Cソースから 0xXX 形式のバイト列を出現順に抽出"""
    pattern = re.compile(r"0x([0-9a-fA-F]{2})\s*")
    return [int(m.group(1), 16) for m in pattern.finditer(content)]


def parse_header_from_c(content: str) -> dict:
    """lv_img_dsc_t の .header.w, .header.h, .header.cf を簡易パース"""
    w = re.search(r"\.header\.w\s*=\s*(\d+)", content)
    h = re.search(r"\.header\.h\s*=\s*(\d+)", content)
    cf = re.search(r"\.header\.cf\s*=\s*(\w+)", content)
    return {
        "w": int(w.group(1)) if w else 0,
        "h": int(h.group(1)) if h else 0,
        "cf": cf.group(1) if cf else "",
    }


def rgb565a8_to_bgr(data: bytes, width: int, height: int) -> bytes:
    """LVGL RGB565A8: 平面レイアウト [全ピクセルRGB565(2B)] [全ピクセルA8(1B)] を BGR 24bit に変換"""
    n = width * height
    color_size = n * 2
    alpha_size = n
    if len(data) < color_size + alpha_size:
        raise ValueError(f"data length {len(data)} < {color_size + alpha_size} (w={width} h={height})")
    color_buf = data[:color_size]
    alpha_buf = data[color_size : color_size + alpha_size]
    out = bytearray()
    for i in range(n):
        lo, hi = color_buf[i * 2], color_buf[i * 2 + 1]
        rgb565 = lo | (hi << 8)
        a = alpha_buf[i]
        # RGB565 -> R5 G6 B5
        r = (rgb565 >> 11) & 0x1F
        g = (rgb565 >> 5) & 0x3F
        b = rgb565 & 0x1F
        # 8bitに拡張
        r = (r << 3) | (r >> 2)
        g = (g << 2) | (g >> 4)
        b = (b << 3) | (b >> 2)
        # アルファでブレンド（透過は黒で表示）
        if a < 255:
            r = (r * a + 0 * (255 - a)) // 255
            g = (g * a + 0 * (255 - a)) // 255
            b = (b * a + 0 * (255 - a)) // 255
        out.append(b)
        out.append(g)
        out.append(r)
    return bytes(out)


def write_bmp(path: Path, width: int, height: int, bgr_data: bytes) -> None:
    """24bit BGR で BMP を書き出す（下から上の行順）"""
    row_bytes = (width * 3 + 3) & ~3
    pad = row_bytes - width * 3
    pixel_offset = 54
    file_size = pixel_offset + row_bytes * height

    with open(path, "wb") as f:
        # BITMAPFILEHEADER
        f.write(b"BM")
        f.write(struct.pack("<I", file_size))
        f.write(struct.pack("<HH", 0, 0))
        f.write(struct.pack("<I", pixel_offset))
        # BITMAPINFOHEADER (40 bytes)
        f.write(struct.pack("<I", 40))
        f.write(struct.pack("<ii", width, height))
        f.write(struct.pack("<HH", 1, 24))
        f.write(struct.pack("<IIiiII", 0, 0, 0, 0, 0, 0))
        # ピクセルデータ（BMPは下から上）
        for y in range(height - 1, -1, -1):
            row_start = y * width * 3
            f.write(bgr_data[row_start : row_start + width * 3])
            if pad:
                f.write(b"\x00" * pad)


def main() -> None:
    input_path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_INPUT
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_OUTPUT

    if not input_path.is_file():
        print(f"Error: not found: {input_path}", file=sys.stderr)
        sys.exit(1)

    content = input_path.read_text(encoding="utf-8", errors="replace")
    header = parse_header_from_c(content)
    w, h, cf = header["w"], header["h"], header["cf"]

    if w <= 0 or h <= 0:
        print("Error: could not parse width/height from .c file", file=sys.stderr)
        sys.exit(1)

    if "RGB565A8" not in cf and "LV_IMG_CF_RGB565A8" not in cf:
        print(f"Warning: unsupported format '{cf}', assuming RGB565A8", file=sys.stderr)

    raw = extract_bytes_from_c(content)
    data = bytes(raw)

    if len(data) < w * h * 3:
        print(
            f"Error: expected at least {w * h * 3} bytes, got {len(data)}",
            file=sys.stderr,
        )
        sys.exit(1)

    bgr = rgb565a8_to_bgr(data, w, h)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    write_bmp(output_path, w, h, bgr)
    print(f"Wrote: {output_path} ({w}x{h})")


if __name__ == "__main__":
    main()
