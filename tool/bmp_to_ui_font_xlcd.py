#!/usr/bin/env python3
"""
BMP アイコン群から LVGL 用 font C を生成する。

想定:
- 入力: tool/icon_export/ui_font_xlcd/icon_XXXX_00YY.bmp
- 出力: src/ui/fonts/ui_font_xlcd.c

ファイル名末尾の 4 桁 HEX を Unicode コードポイントとして扱う。
例: icon_A_007B.bmp -> U+007B

注意:
- このスクリプトは TTF を介さず、LVGL のカスタムコールバック方式フォントを生成する。
- 生成フォントは 2bpp（lv_font_conv の既存設定に寄せる）。
"""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover
    raise SystemExit(
        "Pillow が必要です。`pip install pillow` を実行してください。"
    ) from exc


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_INPUT_DIR = REPO_ROOT / "tool" / "icon_export" / "ui_font_xlcd"
DEFAULT_OUTPUT = REPO_ROOT / "src" / "ui" / "fonts" / "ui_font_xlcd.c"

# 必須グリフ範囲（終端を含む）
REQUIRED_DIGIT_START = 0x30  # '0'
REQUIRED_DIGIT_END = 0x34    # '4'
REQUIRED_LOWER_START = 0x61  # 'a'
REQUIRED_LOWER_END = 0x7A    # 'z'
REQUIRED_UPPER_START = 0x41  # 'A'
REQUIRED_UPPER_END = 0x43    # 'C'


def format_required_range_label(start: int, end_inclusive: int) -> str:
    """例: 0x41,0x43 -> 'A-C' / 1文字範囲なら 'A'"""
    if end_inclusive < start:
        return "?"
    first = chr(start)
    last = chr(end_inclusive)
    return first if first == last else f"{first}-{last}"


@dataclass
class Glyph:
    codepoint: int
    width: int
    height: int
    bitmap_bytes: bytes

    @property
    def bytes_per_row(self) -> int:
        # 2bpp
        return (self.width * 2 + 7) // 8

    @property
    def adv_w_16(self) -> int:
        # LVGL は 1/16 px 単位
        return self.width * 16

    @property
    def c_name(self) -> str:
        return f"glyph_u{self.codepoint:04X}"


def parse_codepoint_from_filename(path: Path) -> int | None:
    # 末尾の _XXXX.bmp を読む
    m = re.search(r"_([0-9A-Fa-f]{4})\.bmp$", path.name)
    if not m:
        return None
    return int(m.group(1), 16)


def rgba_to_2bpp(
    img: Image.Image,
    threshold: int,
    alpha_threshold: int,
    dark_on: bool,
) -> tuple[int, int, bytes]:
    rgba = img.convert("RGBA")
    width, height = rgba.size
    px = rgba.load()

    # LVGL fmt_txt 2bpp の連続パッキング（行末でバイト境界に揃えない）
    levels: list[int] = []
    for y in range(height):
        row: list[int] = []
        for x in range(width):
            r, g, b, a = px[x, y]
            lum = (r * 299 + g * 587 + b * 114) // 1000
            if a < alpha_threshold:
                lv = 0
            else:
                # 中間調を維持するため、二値化せず輝度を 0..3 に量子化する。
                # dark_on=True なら「暗いほど濃い」、False なら「明るいほど濃い」。
                tone = (255 - lum) if dark_on else lum
                # しきい値は極端なノイズ抑制にのみ使用（閾値未満を 0）
                if tone < threshold:
                    lv = 0
                else:
                    # tone(0..255) と alpha(0..255) を掛け合わせて 2bit(0..3) 化
                    eff = (tone * a + 127) // 255
                    lv = (eff * 3 + 127) // 255
                    if lv > 3:
                        lv = 3
            row.append(lv)
        levels.extend(row)

    out = bytearray()
    cur = 0
    bit_pos = 6  # 2bpp: 6,4,2,0
    for lv in levels:
        cur |= (lv & 0x03) << bit_pos
        bit_pos -= 2
        if bit_pos < 0:
            out.append(cur)
            cur = 0
            bit_pos = 6
    if bit_pos != 6:
        out.append(cur)

    return width, height, bytes(out)


def load_glyphs(
    input_dir: Path,
    threshold: int,
    alpha_threshold: int,
    dark_on: bool,
) -> list[Glyph]:
    bmp_files = sorted(input_dir.glob("*.bmp"))
    if not bmp_files:
        raise SystemExit(f"BMP が見つかりません: {input_dir}")

    glyphs: list[Glyph] = []
    for bmp in bmp_files:
        cp = parse_codepoint_from_filename(bmp)
        if cp is None:
            print(f"[skip] codepoint が読めないファイル名: {bmp.name}")
            continue
        with Image.open(bmp) as img:
            w, h, b = rgba_to_2bpp(img, threshold, alpha_threshold, dark_on)
        glyphs.append(Glyph(cp, w, h, b))

    if not glyphs:
        raise SystemExit("有効な BMP が 0 件です。*_XXXX.bmp 形式を確認してください。")

    return sorted(glyphs, key=lambda g: g.codepoint)


def format_bytes(data: bytes, cols: int = 16) -> str:
    hexes = [f"0x{b:02X}" for b in data]
    lines = []
    for i in range(0, len(hexes), cols):
        lines.append("    " + ", ".join(hexes[i : i + cols]))
    return ",\n".join(lines) if lines else "    0x00"


def generate_c(font_name: str, macro_name: str, glyphs: list[Glyph], source_dir: Path) -> str:
    # 空白は必ず入れる（既存 lv_font_conv 生成に合わせる）
    cps = {g.codepoint for g in glyphs}
    if 0x20 not in cps:
        # 代表幅は最小 8px 程度
        w = max(8, min((g.width for g in glyphs), default=8))
        glyphs = [Glyph(0x20, 0, 0, b"")] + glyphs
    glyphs = sorted(glyphs, key=lambda g: g.codepoint)

    # bitmap を連結し、glyph_dsc の bitmap_index を作る
    bitmap_blob = bytearray()
    dsc_rows: list[tuple[int, Glyph]] = []
    for g in glyphs:
        idx = len(bitmap_blob)
        dsc_rows.append((idx, g))
        bitmap_blob.extend(g.bitmap_bytes)

    max_h = max(g.height for g in glyphs)
    # 既存と近い見え方にするため少し余裕を持たせる
    line_height = max_h + 3
    base_line = 2

    # cmap: 0x20-0x7F の SPARSE_TINY 1本で統一
    range_start = 0x20
    range_length = 0x7F - 0x20 + 1
    unicode_offsets = [g.codepoint - range_start for g in glyphs]

    lines: list[str] = []
    lines.append("/*******************************************************************************")
    lines.append(" * Auto generated by tool/bmp_to_ui_font_xlcd.py")
    lines.append(f" * Source dir: {source_dir.as_posix()}")
    lines.append(f" * Glyphs: {len(glyphs)}")
    lines.append(" * Format: lv_font_fmt_txt (bpp=2)")
    lines.append(" ******************************************************************************/")
    lines.append("")
    lines.append('#include "../ui.h"')
    lines.append("")
    lines.append(f"#ifndef {macro_name}")
    lines.append(f"#define {macro_name} 1")
    lines.append("#endif")
    lines.append("")
    lines.append(f"#if {macro_name}")
    lines.append("")
    lines.append("/*-----------------")
    lines.append(" *    BITMAPS")
    lines.append(" *----------------*/")
    lines.append("")
    lines.append("static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {")
    if bitmap_blob:
        lines.append(format_bytes(bytes(bitmap_blob)))
    else:
        lines.append("    0x00")
    lines.append("};")
    lines.append("")
    lines.append("/*---------------------")
    lines.append(" *  GLYPH DESCRIPTION")
    lines.append(" *--------------------*/")
    lines.append("")
    lines.append("static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {")
    lines.append("    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0}, /* id=0 reserved */")
    for idx, g in dsc_rows:
        lines.append(
            "    {.bitmap_index = %d, .adv_w = %d, .box_w = %d, .box_h = %d, .ofs_x = 0, .ofs_y = 0}, /* U+%04X */"
            % (idx, g.adv_w_16, g.width, g.height, g.codepoint)
        )
    lines.append("};")
    lines.append("")
    lines.append("/*---------------------")
    lines.append(" *  CHARACTER MAPPING")
    lines.append(" *--------------------*/")
    lines.append("")
    lines.append("static const uint16_t unicode_list_0[] = {")
    ul = [f"0x{u:X}" for u in unicode_offsets]
    if ul:
        for i in range(0, len(ul), 12):
            lines.append("    " + ", ".join(ul[i : i + 12]) + ("," if i + 12 < len(ul) else ""))
    lines.append("};")
    lines.append("")
    lines.append("static const lv_font_fmt_txt_cmap_t cmaps[] = {")
    lines.append("    {")
    lines.append(
        f"        .range_start = {range_start}, .range_length = {range_length}, .glyph_id_start = 1,"
    )
    lines.append(
        f"        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = {len(unicode_offsets)}, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY"
    )
    lines.append("    }")
    lines.append("};")
    lines.append("")
    lines.append("/*--------------------")
    lines.append(" *  ALL CUSTOM DATA")
    lines.append(" *--------------------*/")
    lines.append("")
    lines.append("#if LV_VERSION_CHECK(8, 0, 0)")
    lines.append("static lv_font_fmt_txt_glyph_cache_t cache;")
    lines.append("static const lv_font_fmt_txt_dsc_t font_dsc = {")
    lines.append("#else")
    lines.append("static lv_font_fmt_txt_dsc_t font_dsc = {")
    lines.append("#endif")
    lines.append("    .glyph_bitmap = glyph_bitmap,")
    lines.append("    .glyph_dsc = glyph_dsc,")
    lines.append("    .cmaps = cmaps,")
    lines.append("    .kern_dsc = NULL,")
    lines.append("    .kern_scale = 0,")
    lines.append("    .cmap_num = 1,")
    lines.append("    .bpp = 2,")
    lines.append("    .kern_classes = 0,")
    lines.append("    .bitmap_format = 0,")
    lines.append("#if LV_VERSION_CHECK(8, 0, 0)")
    lines.append("    .cache = &cache")
    lines.append("#endif")
    lines.append("};")
    lines.append("")
    lines.append("/*-----------------")
    lines.append(" *  PUBLIC FONT")
    lines.append(" *----------------*/")
    lines.append("")
    lines.append("#if LV_VERSION_CHECK(8, 0, 0)")
    lines.append(f"const lv_font_t {font_name} = {{")
    lines.append("#else")
    lines.append(f"lv_font_t {font_name} = {{")
    lines.append("#endif")
    lines.append("    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,")
    lines.append("    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,")
    lines.append(f"    .line_height = {line_height},")
    lines.append(f"    .base_line = {base_line},")
    lines.append("#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)")
    lines.append("    .subpx = LV_FONT_SUBPX_NONE,")
    lines.append("#endif")
    lines.append("#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8")
    lines.append("    .underline_position = 0,")
    lines.append("    .underline_thickness = 0,")
    lines.append("#endif")
    lines.append("    .dsc = &font_dsc")
    lines.append("#if LVGL_VERSION_MAJOR >= 8")
    lines.append("    , .fallback = NULL")
    lines.append("    , .user_data = NULL")
    lines.append("#endif")
    lines.append("};")
    lines.append("")
    lines.append(f"#endif /*#if {macro_name}*/")
    lines.append("")
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="BMP から LVGL の ui_font_xlcd.c 形式を生成する"
    )
    parser.add_argument(
        "--input-dir",
        type=Path,
        default=DEFAULT_INPUT_DIR,
        help=f"BMP ディレクトリ（既定: {DEFAULT_INPUT_DIR}）",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=DEFAULT_OUTPUT,
        help=f"出力 C ファイル（既定: {DEFAULT_OUTPUT}）",
    )
    parser.add_argument(
        "--font-name",
        default="ui_font_xlcd",
        help="生成する lv_font_t 変数名（既定: ui_font_xlcd）",
    )
    parser.add_argument(
        "--macro-name",
        default="UI_FONT_XLCD",
        help="有効化マクロ名（既定: UI_FONT_XLCD）",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=128,
        help="二値化しきい値（0-255, 既定: 128）",
    )
    parser.add_argument(
        "--alpha-threshold",
        type=int,
        default=1,
        help="アルファしきい値（0-255, 既定: 1）",
    )
    parser.add_argument(
        "--dark-on",
        action="store_true",
        help="暗いピクセルを前景として扱う（既定は明るいピクセルを前景）",
    )
    parser.add_argument(
        "--allow-partial",
        action="store_true",
        help="必須グリフ欠落時もエラーにしない（既定はエラー）",
    )
    args = parser.parse_args()

    glyphs = load_glyphs(
        args.input_dir,
        threshold=args.threshold,
        alpha_threshold=args.alpha_threshold,
        dark_on=args.dark_on,
    )
    required = (
        set(range(REQUIRED_DIGIT_START, REQUIRED_DIGIT_END + 1))
        | set(range(REQUIRED_LOWER_START, REQUIRED_LOWER_END + 1))
        | set(range(REQUIRED_UPPER_START, REQUIRED_UPPER_END + 1))
    )
    got = {g.codepoint for g in glyphs}
    missing = sorted(required - got)
    if missing and not args.allow_partial:
        required_label = ", ".join(
            [
                format_required_range_label(REQUIRED_DIGIT_START, REQUIRED_DIGIT_END),
                format_required_range_label(REQUIRED_LOWER_START, REQUIRED_LOWER_END),
                format_required_range_label(REQUIRED_UPPER_START, REQUIRED_UPPER_END),
            ]
        )
        miss_txt = ", ".join(f"U+{cp:04X}" for cp in missing)
        raise SystemExit(
            f"必須グリフが不足しています（{required_label}）。"
            " 全アイコン消失を防ぐため中断します。\n"
            f"missing: {miss_txt}\n"
            "必要なら --allow-partial を指定してください。"
        )
    c_src = generate_c(
        font_name=args.font_name,
        macro_name=args.macro_name,
        glyphs=glyphs,
        source_dir=args.input_dir,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(c_src, encoding="utf-8")
    print(f"Wrote: {args.output}")
    print(f"Glyphs: {len(glyphs)}")
    for g in glyphs:
        print(f"  U+{g.codepoint:04X} {g.width}x{g.height} -> {len(g.bitmap_bytes)} bytes")


if __name__ == "__main__":
    main()
