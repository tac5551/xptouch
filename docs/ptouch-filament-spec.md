# XPtouch フィラメント編集画面 仕様まとめ  
（Bambu Lab クラウドAPI を利用した実装）

## 全体の目的
- XPtouch のフィラメント編集画面で、現在選択されているプリンターに適合するフィラメント設定のみを表示
- メーカー選択 → 種別（素材タイプ）ドロップダウン更新 → 選択した種別の温度上下限などを詳細APIから取得して表示

## 使用するAPIエンドポイント
1. **一覧取得**  
   `GET /v1/iot-service/api/slicer/setting?version={SLICER_VERSION}`  
   → アカウントに紐づくスライサー設定（print / filament / process など）の一覧を取得  
   → filament セクション（public + private）を使用

2. **詳細取得**  
   `GET /v1/iot-service/api/slicer/setting/{SETTING_ID}`  
   → 選択した filament 設定の詳細パラメータを取得  
   → 主に温度関連フィールドを使用

## 処理フロー

### 1. 初期ロード時
- 現在選択されているプリンターの機種を取得（例: P1S, X1C, A1 など）
- 一覧APIを呼び出し、filament 設定（public + private）を取得
- 現在プリンターに適合する filament のみをフィルタリング

### 2. プリンター適合フィルタリング ルール
- name に含まれる `@BBL XXX` を最優先で参照
- 互換マッピング（P1シリーズ・X1シリーズの共有傾向を反映）

| 表示名に含まれる文字列 | 対応するプリンター（表示OK）       |
|-------------------------|-------------------------------------|
| @BBL P1P               | P1P, **P1S**                       |
| @BBL X1C               | X1C, **X1**                        |
| @BBL A1                | A1, A1 mini                        |
| @BBL H2C / H2D など    | 該当機種のみ                       |
| （@BBL なし / Generic）| 全プリンターで表示（fallback）     |

- P1S 選択時 → `@BBL P1P` のプロファイルも表示対象とする
- X1 または X1C 選択時 → `@BBL X1C` のプロファイルを使用
- フィルタ通過した filament のみでメーカー・種別一覧を作成

### 3. メーカー選択時
- フィルタ済み filament からユニークなメーカー一覧を抽出
  - 抽出ルール（name から）:
    - "Generic ..." → "Generic"
    - "Bambu ..." → "Bambu Lab"
    - その他 → name の先頭単語（PolyLite, eSun など）
- メーカー選択ドロップダウンにセット

### 4. メーカー選択 → 種別ドロップダウン更新
- 選択されたメーカーに対応する filament のみを抽出
- 種別（material type）を name から抽出
  - 抽出ルール例:
    - "Generic ASA" → "ASA"
    - "Generic PC @0.2 nozzle" → "PC"（@以降はオプション表示用に保持推奨）
    - "PolyLite ASA" → "ASA"
- 種別ドロップダウンをクリア → 選択メーカーの種別のみを設定
- 他のメーカーの種別は表示・選択不可とする

### 5. 種別選択時
- 選択メーカー + 選択種別に一致する filament エントリを特定
  - name 文字列で完全一致または最も近いものを選択
  - 例: P1P/P1S で Generic TPU → "Generic TPU for AMS @BBL P1P"（setting_id: GFSU98_01）
- 詳細APIを呼び出し（GET /{SETTING_ID}）
- 温度関連フィールドを抽出
  - 主なフィールド:
    - nozzle_temperature_range_low
    - nozzle_temperature_range_high
    - nozzle_temperature（範囲がない場合のフォールバック）
    - nozzle_temperature_initial_layer
    - bed_temperature など（必要に応じて）
- 画面に表示
  - ノズル温度範囲: [low] - [high] ℃
  - 推奨温度: [標準値] ℃
  - 初層温度: [初層値] ℃
  - 範囲が存在しない場合 → 単一値を min=max として表示

## 注意点
- filament 設定にはプリンター適合条件が明示的にないため、name 文字列によるマッチングが主手段
- nozzle 指定（@0.2 nozzle など）は種別抽出時に除去するか、別途UI表示
- private フィラメントも必ず処理対象に含める
- P1P と P1S、X1 と X1C はプロファイルを共有する傾向が強いため、互換マッピングを適用
- 認証（Bearer トークン）必須

## Chrome 拡張での public_filaments.json 生成（メモリ軽減）
- **xtouch28** 拡張のポップアップに「Printer (for filament list)」ドロップダウン（P1P/P1S, X1/X1C, A1, All）と「Download public_filaments.json」ボタンを追加済み
- Bambu API（`GET /v1/iot-service/api/slicer/setting?version=2.0.0.0`）を Bearer トークンで取得し、選択プリンターでフィルタ・Brand 正規化（Generic / Bambu Lab）・Type 抽出を行い、端末と同じ形式の JSON を生成してダウンロードする
- 形式: `{ "brands": [], "items": { "Bambu Lab": [ { "id", "n", "t" }, ... ], "Generic": [...] } }`。`items` はメーカー名をキーにしたオブジェクトで、値は `{ id, n, t }` の配列。Type ドロップダウンと setting_id 解決をここで完結。
- このファイルを SD の **/xtouch/public_filaments.json** に保存すると、端末は一覧取得・ビルドをスキップし、メモリ負荷（900KB 取得・192KB パース）を避けられる

## 例: P1P / P1S で Generic TPU を選択した場合
- メーカー: Generic
- 種別: TPU（または TPU-AMS）
- 対象: "Generic TPU for AMS @BBL P1P"
- setting_id: GFSU98_01
- 詳細APIで温度範囲を取得 → 画面表示

## A: ESP32 から API 取得して MQTT 送信（クラウドログイン済み時）
- **端末（ESP32）** が Save 時に、クラウドログイン済みかつ SD に温度が無い（min/max が 0）場合、**ESP32 から** `GET /v1/iot-service/api/slicer/setting/{SETTING_ID}` を呼び、取得した `nozzle_temperature_range_low` / `nozzle_temperature_range_high` で `ams_filament_setting` を組み立てて MQTT 送信する。既存の BambuCloud Bearer 認証を利用。
- プリンタ一覧取得ロジックがあるため、**Chrome 側**で同じ API を叩き、対象プリンタの request トピックに直接 publish する運用（Chrome が MQTT 送信）も可能。その場合は端末の Save を使わず Chrome が送る。

## SD 上のフィラメントファイル（メーカーごと・ノズル区別なし・1セット）
- **パス**: `/xtouch/filament/`（フィラメント情報。機種名・ノズル径はファイル名に含めない）
  - ブランド一覧: `filaments_brands.txt`
  - メーカー別タイプ一覧: `filaments_{Brand名}.txt`（例: `filaments_Bambu_Lab.txt`, `filaments_Generic.txt`）
- **setting_id のルール**: API の filament セグメントは同一 `filament_id` で複数機種向けの `setting_id` が並ぶ。SD の各メーカー別 txt には **filament_id ごとに、配列で最初に出現した setting_id を 1 行で記載**する。
  - 例: Bambu ABS → 最初の setting_id は `GFSB00_07`。Bambu ABS-G → `GFSB50_02`。機種ごと・ノズルごとの一覧は使わず、メーカーごとの 1 ファイルのみを使用する。

## B: SD に温度を書き、端末はオフラインで MQTT 送信（Chrome 常時不要）
- 端末は Chrome が常時応答できる前提にしない。Chrome（または PC ツール）は **事前に** 各 SETTING_ID で `GET /v1/iot-service/api/slicer/setting/{SETTING_ID}` を巡回し、取得した `nozzle_temperature_range_low` / `nozzle_temperature_range_high` を SD のフィラメントファイルに書き込む。
- **SD 行形式（拡張）**: 従来 `id|n|t`（例: `GFL03|PLA Basic|PLA`）。拡張で **id|n|t|nozzle_temp_min|nozzle_temp_max** の 5 列を許容。例: `GFL03|PLA Basic|PLA|220|250`。4 列目・5 列目が無い行は従来どおり（Save 時は min/max を 0 で送信）。行先頭の `id` は上記ルールで選んだ setting_id（filament_id ごと最初の 1 つ）。
- Chrome 側: 一覧で得た各 `id` に対して詳細 API を呼び、メーカー別テキストファイル（例: `filaments_Bambu_Lab.txt`）の該当行を **id|n|t|min|max** に更新（または上書き生成）して SD に保存。端末はその SD を読んで Save 時に `ams_filament_setting` の `nozzle_temp_min` / `nozzle_temp_max` に載せて MQTT 送信する。

**A と B は両方使用可能**：オンライン時は A（Chrome が MQTT 直接送信）、オフラインや Chrome 未起動時は B（端末が SD を参照して MQTT 送信）。
