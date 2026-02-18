# img_logo_to_bmp — LVGL 画像 C ソースを BMP に変換

`ui_image_logo.c` など、LVGL 用にエクスポートされた C の画像配列から BMP ファイルを復元するツールです。

## 使い方

```bash
# 省略時: src/ui/ui_image_logo.c → tool/img_logo.bmp
python tool/img_logo_to_bmp.py

# 入力・出力を指定
python tool/img_logo_to_bmp.py <input.c> <output.bmp>
```

## 対応フォーマット

- **LV_IMG_CF_RGB565A8**（LVGL の実装に合わせた平面レイアウト）
  - 先頭: 全ピクセルの RGB565（幅×高さ×2 バイト、リトルエンディアン）
  - 続き: 全ピクセルのアルファ（幅×高さ×1 バイト）
  - アルファは黒でブレンドし、24bit BGR の BMP として出力します。

## 出力

- 24bit BMP（BGR、下から上の行順）
- 幅・高さは C ソース内の `.header.w` / `.header.h` から自動取得

## 注意

- C ソース内の **最初の** `uint8_t ... _map[] = { ... };` と `lv_img_dsc_t` の定義を参照します。
- 他のフォーマット（Indexed など）には未対応です。
