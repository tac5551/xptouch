# print_status / print_gcode_action 仕様

Bambu MQTT の push_status で受信する印刷状態まわりの仕様メモ。  
参照: `src/xtouch/types.h`, `src/ui/components/ui_comp_homecomponent.c`, `src/xtouch/neopixel.h`

---

## print_status（印刷状態）

`bambuStatus.print_status`。enum `XTouchPrintStatus` に対応。

| 値 | 定数名 | 意味 |
|----|--------|------|
| 0 | XTOUCH_PRINT_STATUS_IDLE | アイドル |
| 1 | XTOUCH_PRINT_STATUS_RUNNING | 印刷中 |
| 2 | XTOUCH_PRINT_STATUS_PAUSED | 一時停止 |
| 3 | XTOUCH_PRINT_STATUS_FINISHED | 完了 |
| 4 | XTOUCH_PRINT_STATUS_PREPARE | 準備中 |
| 5 | XTOUCH_PRINT_STATUS_FAILED | 失敗 |

---

## print_gcode_action（Gコード側の動作種別）

`bambuStatus.print_gcode_action`。  
印刷中（RUNNING）のとき、現在どの処理をしているかを示す。  
UI では「レイヤー数」の代わりにメッセージを表示する場合がある。

**注意**: P2 および H2 で追加されたキャリブレーションには非対応です。本ドキュメントで扱う値は主に X1 等の機種で観測されたものです。

### 表示・LED対応あり

| 値 | 表示メッセージ（英語） | 備考 |
|----|------------------------|------|
| 0 | Wait heating | ライトオフ、ノズル温度待ち |
| 1 | Bed leveling | ベッドレベル |
| 2 | Heatbed preheat | ヒートベッド予熱 |
| 3 | Vibration compensation | 振動補正 |
| 8 | Calibration | 外因キャリブレーション等 |
| 13 | Head Homing | ワイプ後のホーム |
| 14 | Nozzle wipe | ノズルワイプ |
| 18 | MicroRider Calibration | MicroRider キャリブレーション |
| 25 | Motor noise cancelling | モーターノイズキャンセリング（要確認） |
| 48 | Motor noise cancelling | モーターノイズキャンセリング |

### 特殊扱い

| 値 | 動作 |
|----|------|
| 255 | **表示は更新しない**（直前の文字列を維持）。neopixel はシアン（水色）パターン3。 |

### 上記以外（default）

- **表示**: レイヤー数 `current_layer / total_layers`（例: `5/100`）
- **neopixel**: LED オフ

---

## print_real_action

`bambuStatus.print_real_action`。実機側の動作種別（別途 Bambu 仕様）。  
デバッグ用にシリアルに出力している。

---

## 参照コード

- ラベル表示分岐: `ui_comp_homecomponent.c` の `onXTouchPrintStatus` 内 switch
- LED 分岐: `neopixel.h` の `XTOUCH_PRINT_STATUS_RUNNING` 内の `print_gcode_action` 判定
