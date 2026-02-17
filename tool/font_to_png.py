#!/usr/bin/env python3
"""
LVGL フォント C ファイル (.c) からグリフを抽出し無圧縮 BMP または SVG として出力する。
使い方:
  python font_to_png.py [path/to/ui_font_xlcd.c]     # 既定: src/ui/fonts/ui_font_xlcd.c → BMP
  python font_to_png.py ui_font_xlcd48.c            # 相対パス可（fonts 基準）
  python font_to_png.py ui_font_xlcd48.c --svg      # シンプルなSVG
  python font_to_png.py ui_font_xlcd.c --flip
出力先: tool/icon_export/ に icon_{文字}_{Unicode}.bmp / .svg
2bpp は 2bit(1px) 単位でストリームデコード、1行 box_w ピクセルで区切る。
"""
import re
import sys
import array
from pathlib import Path

# 2bpp: 1バイト = 4ピクセル (2bitずつ、MSB first)、値 0-3 を 0-255 にスケール
def decode_2bpp_row(data, width):
    pixels = []
    for i in range(0, len(data)):
        b = data[i]
        for shift in (6, 4, 2, 0):
            if len(pixels) >= width:
                break
            v = (b >> shift) & 3
            pixels.append(int(v * 255 / 3))
    return pixels[:width]

def decode_2bpp_stream(data):
    """バイト列を 2bit(1px) 単位のストリームにデコード。端数も次のピクセルとして続く。"""
    pixels = []
    for b in data:
        for shift in (6, 4, 2, 0):
            v = (b >> shift) & 3
            pixels.append(int(v * 255 / 3))
    return pixels

def parse_font_c(filepath):
    with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    # Bpp 取得
    bpp_match = re.search(r"Bpp:\s*(\d+)", content)
    bpp = int(bpp_match.group(1)) if bpp_match else 2

    # glyph_bitmap 配列を抽出 (コメント行を除き 0x00, 0x1f 形式のみ)
    bitmap_match = re.search(
        r"static\s+LV_ATTRIBUTE_LARGE_CONST\s+const\s+uint8_t\s+glyph_bitmap\[\]\s*=\s*\{",
        content,
    )
    if not bitmap_match:
        return None
    start = bitmap_match.end()
    depth = 1
    pos = start
    while pos < len(content) and depth > 0:
        if content[pos] == "{":
            depth += 1
        elif content[pos] == "}":
            depth -= 1
        pos += 1
    hex_str = content[start : pos - 1]
    hex_bytes = re.findall(r"0x([0-9A-Fa-f]+)", hex_str)
    bitmap = bytes(int(h, 16) for h in hex_bytes)

    # glyph_dsc を抽出 (bitmap_index, adv_w, box_w, box_h, ofs_x, ofs_y)
    glyphs = []
    for m in re.finditer(
        r"\.bitmap_index\s*=\s*(\d+)\s*,.*?\.adv_w\s*=\s*(\d+)\s*,.*?\.box_w\s*=\s*(\d+)\s*,.*?\.box_h\s*=\s*(\d+)\s*,.*?\.ofs_x\s*=\s*(-?\d+)\s*,.*?\.ofs_y\s*=\s*(-?\d+)\s*[,}]",
        content,
        re.DOTALL,
    ):
        glyphs.append(
            {
                "bitmap_index": int(m.group(1)),
                "adv_w": int(m.group(2)),
                "box_w": int(m.group(3)),
                "box_h": int(m.group(4)),
                "ofs_x": int(m.group(5)),
                "ofs_y": int(m.group(6)),
            }
        )
    if not glyphs:
        return None

    # 文字マッピング: xlcd は unicode_list_0(sparse)+97-122。xlcd48 は 48-52 と 97-122 のみ
    unicode_to_glyph = {}
    unicode_list_0 = re.search(r"unicode_list_0\[\]\s*=\s*\{([^}]+)\}", content)
    if unicode_list_0:
        ofs = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]+)", unicode_list_0.group(1))]
        for i, o in enumerate(ofs):
            unicode_to_glyph[32 + o] = 1 + i
    # cmaps の連続範囲。range_start=32 で sparse がある場合は上書きしない
    for m in re.finditer(
        r"\.range_start\s*=\s*(\d+)\s*,.*?\.range_length\s*=\s*(\d+)\s*,.*?\.glyph_id_start\s*=\s*(\d+)",
        content,
        re.DOTALL,
    ):
        start = int(m.group(1))
        length = int(m.group(2))
        gid_start = int(m.group(3))
        if start == 32 and unicode_list_0:
            continue  # xlcd の sparse を維持
        for i in range(length):
            unicode_to_glyph[start + i] = gid_start + i

    return {
        "bpp": bpp,
        "bitmap": bitmap,
        "glyphs": glyphs,
        "unicode_to_glyph": unicode_to_glyph,
    }

def glyph_to_image(data, glyph_index, bpp=2):
    glyphs = data["glyphs"]
    g = glyphs[glyph_index]
    box_w = g["box_w"]
    box_h = g["box_h"]
    if box_w == 0 or box_h == 0:
        return None
    start = g["bitmap_index"]
    # フォントの実バイト数（次グリフの開始 - この開始）
    if glyph_index + 1 < len(glyphs):
        total = glyphs[glyph_index + 1]["bitmap_index"] - start
    else:
        total = (box_w * bpp + 7) // 8 * box_h
    if total <= 0:
        return None
    chunk = data["bitmap"][start : start + total]
    if len(chunk) == 0:
        return None
    # 2bit(1px)単位でストリームデコード。1行 box_w ピクセルで区切り、端数は次の行の先頭として扱う
    stream = decode_2bpp_stream(chunk)
    try:
        img = array.array("B")
        for y in range(box_h):
            row_start = y * box_w
            row_end = row_start + box_w
            row_pixels = stream[row_start:row_end] if row_end <= len(stream) else stream[row_start:]
            img.extend(row_pixels)
            if len(row_pixels) < box_w:
                img.extend([0] * (box_w - len(row_pixels)))
        n = box_w * box_h
        if len(img) < n:
            img.extend([0] * (n - len(img)))
        return (box_w, box_h, img)
    except Exception:
        return None

def save_bmp(w, h, pixels, path):
    """無圧縮 BMP (24bpp, 下から上の行順)。グレースケールを BGR で出力。"""
    try:
        import struct
        # 行は 4 バイト境界にパディング
        row_bytes = ((w * 3 + 3) // 4) * 4
        pixel_data_size = row_bytes * h
        file_size = 14 + 40 + pixel_data_size
        pixel_offset = 14 + 40

        with open(path, "wb") as f:
            # ファイルヘッダ (14 bytes)
            f.write(b"BM")
            f.write(struct.pack("<I", file_size))
            f.write(struct.pack("<HH", 0, 0))
            f.write(struct.pack("<I", pixel_offset))
            # BITMAPINFOHEADER (40 bytes)
            f.write(struct.pack("<I", 40))
            f.write(struct.pack("<ii", w, h))
            f.write(struct.pack("<HH", 1, 24))
            f.write(struct.pack("<I", 0))  # BI_RGB
            f.write(struct.pack("<I", pixel_data_size))
            f.write(struct.pack("<i", 0))
            f.write(struct.pack("<i", 0))
            f.write(struct.pack("<II", 0, 0))
            # ピクセルデータ: BMP は下の行から。グレー v を BGR で
            for y in range(h - 1, -1, -1):
                row = bytearray(row_bytes)
                for x in range(w):
                    v = pixels[y * w + x]
                    row[x * 3 : x * 3 + 3] = (v, v, v)
                f.write(row)
        return True
    except Exception as e:
        print("BMP save error:", e)
        return False

def save_svg(w, h, pixels, path, simple=True):
    """ビットマップを SVG で出力。simple=True なら行ごとに連続ピクセルを1つの rect にまとめる。"""
    try:
        lines = [
            '<?xml version="1.0" encoding="UTF-8"?>',
            f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" height="{h}">',
        ]
        for y in range(h):
            x = 0
            while x < w:
                a = pixels[y * w + x]
                if a == 0:
                    x += 1
                    continue
                if simple:
                    # 同じ行で連続する非0を1本の rect にまとめる
                    x0 = x
                    run_op = a
                    while x < w and pixels[y * w + x] != 0:
                        run_op = max(run_op, pixels[y * w + x])
                        x += 1
                    op = round(run_op / 255.0, 4)
                    lines.append(f'  <rect x="{x0}" y="{y}" width="{x - x0}" height="1" fill="black" opacity="{op}"/>')
                else:
                    op = round(a / 255.0, 4)
                    lines.append(f'  <rect x="{x}" y="{y}" width="1" height="1" fill="black" opacity="{op}"/>')
                    x += 1
        lines.append("</svg>")
        with open(path, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
        return True
    except Exception as e:
        print("SVG save error:", e)
        return False

def main():
    tool_dir = Path(__file__).resolve().parent
    fonts_dir = tool_dir.parent / "src" / "ui" / "fonts"
    if len(sys.argv) >= 2:
        c_file = Path(sys.argv[1])
        if not c_file.is_absolute():
            c_file = fonts_dir / c_file.name
    else:
        c_file = fonts_dir / "ui_font_xlcd.c"

    if not c_file.exists():
        print("Usage: python font_to_png.py [path/to/ui_font_xlcd.c]")
        print("File not found:", c_file)
        sys.exit(1)

    data = parse_font_c(c_file)
    if not data:
        print("Failed to parse font C file")
        sys.exit(1)

    # row_bytes と adv_w の一致チェック（理論上は一致するか）
    bpp = data["bpp"]
    mismatches = []
    for i, g in enumerate(data["glyphs"]):
        box_w, box_h = g["box_w"], g["box_h"]
        if box_w == 0 and box_h == 0:
            continue
        row_bytes = (box_w * bpp + 7) // 8
        adv_w = g.get("adv_w")
        if adv_w is not None and row_bytes != adv_w:
            mismatches.append((i, row_bytes, adv_w, box_w, box_h))
    if mismatches:
        print("row_bytes vs adv_w (一致しないグリフ):")
        for i, rb, aw, bw, bh in mismatches[:20]:
            print(f"  glyph_index={i} row_bytes={rb} adv_w={aw} box_w={bw} box_h={bh}")
        if len(mismatches) > 20:
            print(f"  ... 他 {len(mismatches) - 20} 件")
        print(f"合計 {len(mismatches)}/{len([g for g in data['glyphs'] if g['box_w'] or g['box_h']])} で不一致")
    else:
        print("row_bytes と adv_w は全グリフで一致しています。")

    out_dir = tool_dir / "icon_export"
    out_dir.mkdir(exist_ok=True)

    # unicode -> 表示用ラベル
    def unicode_name(u):
        if 0x20 <= u <= 0x7E:
            c = chr(u)
            return c if c.isalnum() or c in " _-" else f"U+{u:04X}"
        return f"U+{u:04X}"

    exported = 0
    for unicode_val, glyph_id in sorted(data["unicode_to_glyph"].items()):
        if glyph_id >= len(data["glyphs"]):
            continue
        g = data["glyphs"][glyph_id]
        if g["box_w"] == 0 and g["box_h"] == 0:
            continue
        result = glyph_to_image(data, glyph_id, data["bpp"])
        if not result:
            continue
        w, h, pixels = result
        # フォントは通常 Y 上向きなので、画像は上下反転すると自然になることがある
        args = sys.argv[2:]
        if "--flip" in args:
            pix_list = list(pixels)
            for y in range(h // 2):
                for x in range(w):
                    i1, i2 = y * w + x, (h - 1 - y) * w + x
                    pix_list[i1], pix_list[i2] = pix_list[i2], pix_list[i1]
            pixels = array.array("B", pix_list)
        use_svg = "--svg" in args
        svg_simple = "--svg-pixel" not in args  # 未指定ならシンプル（連続を1rectに）
        name = unicode_name(unicode_val)
        safe = "".join(c if c.isalnum() or c in " _-" else "_" for c in name)
        ext = "svg" if use_svg else "bmp"
        path = out_dir / f"icon_{safe}_{unicode_val:04X}.{ext}"
        ok = save_svg(w, h, pixels, path, simple=svg_simple) if use_svg else save_bmp(w, h, pixels, path)
        if ok:
            print(f"{path.name}  {w}x{h}")
            exported += 1

    print(f"Exported {exported} icons to {out_dir}")

if __name__ == "__main__":
    main()
