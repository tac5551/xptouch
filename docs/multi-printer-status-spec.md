# 複数プリンター状態取得 調査メモ

クラウドに登録されている「選択中以外」のプリンターの状態も取得し、MQTT で最大5台程度を扱えるようにする新機能の調査結果。

**採用方針**: **A) 同一ブローカーで「複数デバイス分の購読」を最大5台まで**（接続1本・複数トピック購読）。  
**最大台数**: とりあえず **5** で固定（メイン1 + 他4 = 合計5。定数例: `#define XTOUCH_MULTI_PRINTER_MAX 5`）。  
**対象環境**: **5インチ専用**で実装する（`#ifdef __XTOUCH_SCREEN_50__` でガード）。2.8インチはヒープが厳しい想定のため除外し、余裕が確認できたらガード外しを検討する。

**クラウド vs LAN**  
- **クラウド時のみ**「1接続・複数トピック」で複数台の状態取得が可能。Bambu Cloud は1ブローカーに全デバイスが載るため、同一接続で `device/ID1/report`, `device/ID2/report`, ... を購読すればよい。  
- **LANモード時**はプリンターごとにブローカー（各機のIP）が異なるため、複数台の状態を取るには**台数分の個別MQTT接続**（例: 最大5本の PubSubClient + WiFiClientSecure）が必要。現状の複数プリンター対応（他台の report 購読・一覧表示）は**クラウド時のみ**有効とする。

---

## 1. 参照したコーディングルール

- **docs/UI-CONVENTIONS.md**
  - **xtouch/***: 原則 .h で実装し、main.cpp で include して使い回す。types.h 等の宣言経由で他から呼ぶ。
  - **ui_comp_***: 極力個別実装を追加しない。xtouch の呼び出しはイベント（lv_msg）経由にし、Main/xtouch 層で受けて処理。bambuStatus / xTouchConfig は Main が設定し、UI は参照のみ。

---

## 2. 現状の整理

### 2.1 MQTT

- **接続**: 1 接続のみ。
  - `WiFiClientSecure xtouch_wiFiClientSecure`
  - `PubSubClient xtouch_pubSubClient(xtouch_wiFiClientSecure)`
- **トピック**: `xTouchConfig.xTouchSerialNumber`（選択中プリンター）のみ。
  - `device/{serial}/request` … 送信
  - `device/{serial}/report` … 購読（push_status 等を受信）
- **処理**: `xtouch_mqtt_topic_setup()` で上記トピックを組み立て、`xtouch_mqtt_processPushStatus()` で `bambuStatus` を更新。

### 2.2 クラウド・プリンター選択

- **cloud.hpp**
  - `getDeviceList()`: Bambu Cloud API でアカウントに紐づく全デバイス取得。
  - `selectPrinter()`: 1台なら自動選択、複数なら画面(loadScreen(5))で選択 → `setCurrentDevice(dev_id)` → `xTouchConfig.xTouchSerialNumber` に設定。
  - 全デバイスは `xtouch_paths_printers`（`/xtouch/printer.json`）に dev_id をキーとして保存。
- **接続フロー**（main.cpp）: provisioning あり → `cloud.loadPair()` または `cloud.selectPrinter()` → `xtouch_cloud_mqtt_setup()` → 選択中の 1 台だけ MQTT 購読。

### 2.3 状態の保持

- **bambuStatus**（types.h）: 1 台分の印刷・AMS・温度等の状態を保持する単一グローバル。
- UI は `bambuStatus` を参照してホーム画面・温度・AMS 等を表示。

### 2.4 ビルド環境

- **platformio.ini**: 2.8インチ = `esp32dev`、5インチ = `esp32-s3dev`。新機能は **5インチのみ有効**（`#ifdef __XTOUCH_SCREEN_50__` でガード）。2.8インチはヒープが厳しい想定のため最初から含めず、後から余裕が確認できればガード外しを検討する。

---

## 3. 要件の解釈と採用方針

- **「選択されているプリンタ以外の状態も取得」**  
  → クラウドの device list のうち、現在選択中の 1 台以外のプリンターの push_status（report）も受け取り、一覧表示などに使う。
- **「mqttの接続を複数（最大5程度）」**  
  → **採用: A) 同一ブローカーで「複数デバイス分の購読」を最大5台まで**（**クラウド時のみ**）
  - **クラウド**: MQTT の TCP 接続は 1 本のまま、同一ブローカー（Bambu Cloud）に接続。`device/{メイン}/report` に加え、`device/{他1}/report`, ... を最大5トピックまで subscribe し、各デバイスの report を受信する。
  - **LAN**: 各プリンターが別IPのブローカーになるため、1接続・複数トピックでは対応できない。複数台取得する場合は「接続を複数」（台数分の個別接続）が必要。現状実装では LAN 時は複数プリンター一覧は行わない（クラウド時のみ有効）。

---

## 4. 実装方針案

### 4.1 対象環境・上限

- **5インチ専用**。`#ifdef __XTOUCH_SCREEN_50__` でガードし、他プリンター用のデータ・購読・UI は 5インチビルドでのみ有効にする。2.8インチはヒープを守るため対象外；余裕が確認できたらガード外しを検討する。
- 購読するプリンター数は **最大 5 台**（メイン 1 + 他 4）。実装では `#define XTOUCH_MULTI_PRINTER_MAX 5` などで固定してよい。

### 4.2 MQTT

- **接続**: 既存の 1 本の MQTT 接続をそのまま利用。
- **購読**:
  - メイン: 従来どおり `device/{xTouchSerialNumber}/report`（選択中プリンター → `bambuStatus` 更新）。
  - 追加: クラウドの他デバイスから最大 4 台分（合計 5 台まで）を選び、  
    `device/{dev_id_2}/report`, `device/{dev_id_3}/report`, ... を subscribe。
- **コールバック**: 受信トピックをパースして `device/XXXX/report` の XXXX を取得し、
  - XXXX == xTouchSerialNumber → 既存どおり `xtouch_mqtt_processPushStatus()` で `bambuStatus` を更新。
  - それ以外 → 「他プリンター用」のスロットを特定し、そのスロット用の軽量ステータス構造体を更新（後述）。

### 4.3 他プリンター用の状態データ

- **種類**: 一覧表示に必要な最小限のフィールドのみ。
  - 例: `dev_id`, `name`, `print_status`（または `gcode_state`）, `mc_print_percent`, `mc_left_time`, `subtask_name` など。
- **保持**: 最大 4 台分の配列（例: `other_printer_status_t otherPrinters[4]`）を globals または types で定義。
- **更新**: `xtouch_mqtt_parseMessage()` 内でトピックから dev_id を判定し、既存の `processPushStatus` と同様の JSON パースを「他プリンター用」に適用（bambuStatus は触らず、上記配列の該当スロットのみ更新）。

### 4.4 どの「他プリンター」を購読するか

- **案1**: `printer.json` に含まれる全 dev_id のうち、`xTouchSerialNumber` 以外を最大 4 台まで subscribe（登録順や名前順で固定）。
- **案2**: 設定で「監視する他プリンター」を明示的に選択（新規設定ファイル or printer.json の配列）。最大 4 件。

まずは **案1** で「クラウドにいる残りを最大4台まで自動で購読」とし、必要なら後から案2で設定UIを足す。

### 4.5 設定・起動フロー（クラウド時のみ複数台購読）

- **クラウド**: `xtouch_cloud_mqtt_setup()` の流れで、`xtouch_mqtt_topic_setup()` に加え `xtouch_mqtt_load_other_printers()` で他 dev_id を取得し、接続後に `device/{dev_id}/report` を追加で subscribe。接続は 1 本のまま、subscribe するトピックを増やすだけ。
- **LAN**: `xtouch_local_mqtt_setup()` では従来どおり選択中 1 台のみ。他プリンターの購読・一覧は行わない（複数台対応はクラウド時のみ）。

### 4.6 UI（5インチのみ）

- 他プリンターの状態は、`bambuStatus` と同様に「Main が更新したデータを UI が参照」する形にする（UI-CONVENTIONS に従う）。
- 例: ホーム画面の横や別画面に「他プリンター一覧」（名前・進捗・状態）を表示。表示用データは `otherPrinters[]` を参照。
- 他プリンターへの制御（一時停止など）は今回スコープ外とするか、後続で「選択してメインに切り替え」などにするとよさそう。

### 4.7 メモリ・ソケット

- 1 接続・複数トピックなら、ソケットは 1 本のまま。  
- 追加するのは「最大 4 台分の軽量構造体 + トピック文字列」程度。5インチ（ESP32-S3）では許容範囲と想定。2.8インチはヒープが厳しい想定のため 5 インチ専用とし、ガードで 2.8 ビルドには含めない。

---

## 5. 変更が入りそうなファイル（案）

| ファイル | 変更内容 |
|----------|----------|
| **src/xtouch/types.h** | `#ifdef __XTOUCH_SCREEN_50__` で他プリンター用の軽量構造体と配列（または extern 宣言）を追加。 |
| **src/xtouch/globals.h or globals.c** | 5inch 時のみ `otherPrinters[]` の実体を定義。 |
| **src/xtouch/mqtt.h** | 5inch 時のみ: 他 dev_id 一覧取得、複数 report トピックの subscribe、コールバック内でトピック解析→メイン/他で振り分け、他用のパース処理。 |
| **src/xtouch/cloud.hpp** | 他プリンター一覧取得のヘルパ（getOtherDeviceIds など）があればここか paths 経由。必須ではない。 |
| **UI（5inch 専用）** | 他プリンター一覧用コンポーネント/画面を追加し、`otherPrinters[]` を参照。イベントは lv_msg で Main 経由。 |

---

## 6. まとめ

- **コーディングルール**: xtouch は .h で実装・main で include。UI は xtouch を直接呼ばず lv_msg と共有状態（bambuStatus / 他プリンター用配列）のみ参照。
- **MQTT（方針A）**: 接続は 1 本のまま、同一ブローカーで「選択中 1 台 + 他最大4台」の report トピックを購読し、最大5台分の状態取得を実現する。
- **対象環境**: **5インチ専用**（`#ifdef __XTOUCH_SCREEN_50__` でガード）。ヒープが厳しそうな 2.8インチは対象外とし、余裕が確認できたらガード外しを検討する。

この方針で実装に進むと、クラウドに設定されている選択中以外のプリンターの状態も取得でき、かつ「MQTT を最大5台程度」という要件を満たせます。
