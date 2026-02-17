# アイコンフォント (icomoon.ttf) の復元・再生成

元の `icomoon.ttf` はプロジェクトに含まれていません。  
既存の LVGL 用フォント (.c) は**ラスタ化済みビットマップ**のため、そこから .ttf を完全に復元することはできません。

代わりに、**同じ文字マッピングで新しいアイコンフォントを作成し、LVGL 用に再生成する**手順です。

---

## 現在のフォントで使っている文字マッピング

| 文字 (Unicode) | 用途 (コード内) |
|----------------|-----------------|
| `a` (0x61) | Home |
| `b` (0x62) | Temp |
| `c` (0x63) | Control |
| `d` (0x64) | Settings |
| `n` (0x6E) | AMS |
| `p` (0x70) | Nozzle（他で使用） |

フォントに含まれる範囲（ui_font_xlcd / ui_font_xlcd48）:

- **0x20** … スペース
- **0x30～0x34** … 数字 `0`～`4`
- **0x61～0x7A** … 英小文字 `a`～`z`

**重要:** 新しい .ttf を作る際は、上記のコードポイントに同じ意味のアイコンを割り当てると、既存の `lv_label_set_text(..., "a")` などのコードをそのまま使えます。

---

## 手順 1: 新しい icomoon.ttf を作る

### 方法 A: IcoMoon アプリ (https://icomoon.io/app/)

1. ブラウザで https://icomoon.io/app/ を開く。
2. **Import Icons** で、使いたいアイコン（SVG や既存のアイコンフォント）を読み込む。
3. 必要なアイコンを選択し、**Font** タブで **Preferences (歯車)** を開く。
4. **Unicode codepoint** を次のように設定する（既存コードと合わせる場合）:
   - 1 つ目 → `0x61` (a = Home)
   - 2 つ目 → `0x62` (b = Temp)
   - 3 つ目 → `0x63` (c = Control)
   - 4 つ目 → `0x64` (d = Settings)
   - 5 つ目 → `0x6E` (n = AMS)
   - 6 つ目 → `0x70` (p = Nozzle)
   - その他も a～z / 0～4 に合わせて割り当て。
5. **Export** → **Export Font** で .ttf をダウンロードし、`icomoon.ttf` として保存。

### 方法 B: 既存のアイコンセットを流用

- Font Awesome や Material Icons などを IcoMoon に取り込み、上記のコードポイント (0x61, 0x62, …) に割り当ててから .ttf をエクスポート。

---

## 手順 2: LVGL 用フォントを再生成する

[LVGL の lv_font_conv](https://github.com/lvgl/lv_font_conv) を使います。

### インストール (Node.js が必要)

```bash
npm install -g lv_font_conv
```

### 生成コマンド例

`icomoon.ttf` を `src/ui/fonts/` に置いた場合:

**小さいアイコン (24px) — ui_font_xlcd.c 相当**

```bash
lv_font_conv --bpp 2 --size 24 --font src/ui/fonts/icomoon.ttf -o src/ui/fonts/ui_font_xlcd.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
```

**大きいアイコン (48px) — ui_font_xlcd48.c 相当**

```bash
lv_font_conv --bpp 2 --size 48 --font src/ui/fonts/icomoon.ttf -o src/ui/fonts/ui_font_xlcd48.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
```

**ミニ (16px) — ui_font_xlcdmin.c 相当**

```bash
lv_font_conv --bpp 2 --size 16 --font src/ui/fonts/icomoon.ttf -o src/ui/fonts/ui_font_xlcdmin.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
```

生成した .c の先頭に、既存ファイルと同じ `#include "../ui.h"` や `#if UI_FONT_XXX` のガードが必要な場合は、既存の ui_font_xlcd.c などを参考に追加・調整してください。

---

## 注意

- 既存の .c は **LVGL のフォント形式**に依存しているため、LVGL のバージョンが変わると `lv_font_conv` のオプションや生成形式が変わる場合があります。そのときは LVGL のドキュメントを確認してください。
- 「見た目をできるだけ元に近づけたい」場合は、既存の .c のビットマップを参考に、IcoMoon 側で似た形のアイコンを選んで同じコードポイントに割り当てる必要があります。
- 完全に同一の icomoon.ttf を復元するには、元の x-lcd プロジェクトのアセットや、別バックアップが無い限り不可能です。上記は「同じ使い方で新しいアイコンフォントを用意する」ための手順です。
