# AI／開発者向け 引き継ぎメモ

前のインスタンスや新規セッションで参照するための注意点・仕様の要約です。  
MDファイル（Readme.md, develop.md, print-status-spec.md, tool/README_icomoon.md, docs/CREDITS.md 等）から抽出しています。

---

## プロジェクト概要

- **XPTouch (XPerimentsTouch)**: BambuLabプリンター用タッチスクリーン制御。P1S（タッチパネルなし）向けにチューニング。
- **制約**: P1S **Version 1.7 以前専用**。1.8 以降へアップグレードした個体では使用不可（BambuHandy からダウングレードが必要。BambuCloud 側制限で閲覧のみになる）。
- X1C / X1E / A1 等でも動く可能性はあるが、**動作確認対象外**。

---

## 法的・運用上の注意

- **教育・研究目的**の開発。商用利用は参照元プロジェクトの制限により**全面的に禁止**。
- **オリジナル（xtouch）の開発者には問い合わせないこと**。
- 本プロジェクトは xtouch → XPTouch → XP の系譜。GPLv3。

---

## 開発環境（ビルドする場合）

- **必要**: Python 3.7+, Node.js 14+, PlatformIO。LovyanGFX は `lib/LovyanGFX`（tac-lab 版）を使用。
- **esptool**: PlatformIO で自動インストール。ビルドスクリプトがパス解決するため、PATH 設定は**不要**（旧バージョンでは必要だった）。
- **ESP32-S3-DevKitC-1-N16R8** を使う場合: `docs/boards/esp32-s3-devkitc1-n16r8.json` を `~/.platformio/boards/` にコピーすること。

---

## 印刷状態・MQTT まわり（print-status-spec.md）

- **print_status**: `XTouchPrintStatus`（IDLE / RUNNING / PAUSED / FINISHED / PREPARE / FAILED）。
- **print_gcode_action**: RUNNING 時の処理種別。UI 表示・NeoPixel の分岐に使用。
  - **注意**: P2 / H2 で追加されたキャリブレーションには**非対応**。値は主に X1 等で観測されたもの。
  - 値 **255**: 表示は更新しない（直前の文字列を維持）。NeoPixel はシアン（水色）パターン3。
- 参照: `src/xtouch/types.h`, `src/ui/components/ui_comp_homecomponent.c`, `src/xtouch/neopixel.h`

---

## アイコンフォント（icomoon / ui_font_xlcd48）

- 元の `icomoon.ttf` はリポジトリに**含まれていない**。既存の .c はラスタ化済みビットマップのため、そこから .ttf を完全復元することは**不可**。
- **文字マッピング（既存コードと合わせる場合）**:
  - `a` (0x61) = Home, `b` = Temp, `c` = Control, `d` = Settings, `n` = AMS, `p` = Nozzle 等。
  - 範囲: 0x20, 0x30～0x34, 0x61～0x7A。
- 新規 .ttf を作る場合は上記コードポイントに同じ意味のアイコンを割り当てると、既存の `lv_label_set_text(..., "a")` 等をそのまま使える。
- LVGL 用再生成: `lv_font_conv`。LVGL のバージョンでオプションや形式が変わる可能性あり。

---

## LAN Only / プロビジョニング

- LAN Only と Cloud は**排他**。LAN Only 利用時は SD カードから `provisioning.json` を削除すること。
- 互換のため設定ファイル名は **xtouch.json** であることに注意（設定ページで xtouch.json を作成する旨が Readme に記載）。

---

## ハードウェア・ボード

- 推奨ボードは Readme の表を参照（2432S028 / JC2432W328R 等）。3248S35R は「検証中」、2432S028C は未確認、LED 使用時エラー報告ありのボードあり。
- 480x320 の「動作不可」「対応不可」ボード一覧あり（8048S070 等）。320x240 の「制限付き互換」では温度センサー・NeoPixel 非対応の機種あり。

---

## その他

- **DS18B20**: チャンバー温度用オプション。接続は 1.25mm 4P JST。設定画面で有効化が必要（docs/temperature-sensor.md）。
- **WiFi ループ・無限再起動**などのトラブル時は `config.json` の `coldboot` や WiFi タイムアウトを調整。オンライン config フォームが案内されている。
- タッチ再キャリブレーション: SD の `touch.json`（xptouch ディレクトリ）を削除して再起動。

---

*このファイルは、MD を読んで前のインスタンスからの注意点を引き継ぐために作成・更新しています。*
