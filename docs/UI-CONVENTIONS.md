# xptouch コーディングルール

## xptouch 内部実装（xtouch/*）

- **原則として .h ファイルとして作成し、main.cpp で include して使い回す。**
- 実装を .h に書き、main がその .h を include することで 1 か所で定義し、他からは types.h 等の宣言経由で呼び出す。paths.h や filaments_rev.h と同様のパターンとする。

## ui_ 関連ファイル（ui/*）

- **外部ツールで生成し直すことがあるため、極力個別実装を追加しない。**
- ui_comp_*.c / ui_*Screen.c 等は LVGL の自動生成やツールで再生成される可能性がある。カスタムロジックは最小限にし、必要な振る舞いはイベント経由で xtouch 側に寄せる。
- **画面追加時には ui_comp 側に必ず実装する。** 新規画面の UI 構造・ウィジェット作成・レイアウトは ui_comp_*.c にまとめ、ui_*Screen.c は画面オブジェクトの作成とそのコンポーネントの配置・購読登録にとどめる。
- **必ず既存の他画面を参照し、同様のルール・パターンで実装する。** 例: Printers 画面（ui_comp_printerscomponent.c / ui_printersScreen.c）、Home（ui_comp_homecomponent / ui_homeScreen）などの構成・イベントの送り方・購読の置き場所を踏襲する。

## ui_*.h で宣言されるものは xptouch 側から呼ばれる

- **ui.h / ui_*Screen まわり**: `ui_*Screen_screen_init()` などは **xptouch 側の画面切り替え**で呼ばれる。流れは main → `loadScreen()`（ui_loaders.c）→ `ui_*Screen_screen_init()`。つまり「画面の初期化」は xptouch から呼び出す入口になる。
- **コールバック**: コールバック関数は UI（ui_*.c）で実装し、`lv_obj_add_event_cb` や `lv_msg_subsribe_obj` で LVGL に登録する。xptouch が直接その関数を呼ぶのではなく、**lv_msg_send などでメッセージを送る → それを契機に LVGL が登録済みコールバックを実行**する形。つまり「xptouch のアクションがきっかけで、UI に登録したコールバックが動く」。
- **補足**: `ui_comp_*.h` の `ui_*Component_create()` は xptouch/main からは直接呼ばれない。呼び出し元は **ui_*Screen.c** の `ui_*Screen_screen_init()` 内（画面初期化の一環でコンポーネントを作る）。「xptouch から呼ばれる」のは画面の screen_init までで、コンポーネントの create は「画面初期化のなかでの UI 内呼び出し」。

## xtouch と UI 間のデータのやり取りは types の構造体を経由する

- **xtouch と UI 間で共有するデータは、types.h に定義した構造体・配列を経由して行う。** xtouch がそれらを更新し、UI は参照するだけにする。
- 例: `bambuStatus`, `xTouchConfig`, `otherPrinters`, `xtouch_thumbnail_slot_path[]` など。新規に共有データが必要なときも types.h に型・extern を追加し、実体は globals 等で定義する。

## UI から xtouch の呼び出し

- **xtouch の内部実装関数の呼び出しは、イベント（lv_msg 等）経由で行う。**
- UI コンポーネントから直接 `xtouch_*` を呼ばず、メッセージを送信し、main または xtouch 層で受けて処理する。これにより UI と xtouch の結合を避け、ui_ ファイルの再生成時にも影響を抑えられる。

## 注意（ESPDEBUG）

- **ESPDEBUG 機能をオンにした場合、ヒープ領域が不足し、AMS Edit 画面が動作しなくなる可能性がある。** デバッグ時以外はオフにしておくことを推奨する。

## 必要データが揃わないとき: サイレントなフォールバック禁止と UI での通知

再印刷・履歴・Cloud 連携など、ユーザー操作に直結する処理では、**続行に必要なデータが欠けている・無効である**場合（例: Cloud の `task_id` が空や `"0"`、GET `/my/task` や履歴 API で必須フィールドや **amsDetailMapping** 等が取得できない、MQTT の `push_status` に **`print.task_id` も有効な代替も無い** 等）に次を守る。

1. **禁止**: 足りない情報を、別名のフィールドや推測で勝手に埋めて処理を続ける「フォールバック」でごまかさないこと。データが揃わなければ **そのステップは失敗**として扱う（既存の仕様で変えてはいけない明確な代替だけを別途ドキュメント化する）。**本ドキュメントや関連仕様（MD）で必須と書いた条件を満たせないときも同様**に、黙って代替値で進めないこと。
2. **UI で知らせる**: ユーザーが **エラーまたは不成立であること**に気づけるようにすること。**ログ（`ConsoleError` 等）だけ**にせず、**画面上で分かる表示**を行う（例: `ui_confirmPanel_show` によるメッセージ、当該画面・一覧での説明、既存の `lv_msg` と購読側でのエラー表示パターンに合わせる）。
3. **画面遷移**: 必須データなしで **空の設定画面や誤った前提の画面へ進めない**（ホーム／Printers から Reprint へ進む前に `task_id` を検証するなど、既存実装と同様の扱い）。

新機能を足すときも、API／MQTT の欠損時に「とりあえず続行」せず、**失敗を明示し UI で伝える**方針に合わせること。

---

# UI component (ui_comp_*) conventions

## Do not include or call xtouch/* from UI components

**Do not `#include` xtouch headers (e.g. `xtouch/public_filaments.h`, `xtouch/filesystem.h`) or call xtouch-layer functions from within ui/components (ui_comp_*.c).**

- Reason: Including xtouch files from ui_comp_* causes build errors (e.g. undeclared `ConsoleDebug`/`ConsoleError` when filesystem.h is pulled in, or other dependency/link issues). It also couples the UI layer to the xtouch layer.
- What to do: Have **Main (or the xtouch layer)** load data, check file existence, and set shared state (e.g. `bambuStatus`, `xTouchConfig`). The UI only reads that state or uses data prepared by Main. For dropdown options that come from xtouch (e.g. public_filaments), load in Main and expose results via globals or accessors that the UI can use without including xtouch headers.

## Do not call xtouch_* from UI

**Do not call `xtouch_*` functions from within ui/components (ui_comp_*.c).**

- Reason: To avoid link-time multiple definitions and coupling between UI and the xtouch layer.
- What to do: Have **Main (or the xtouch layer)** fetch or determine the needed state once and store it in `bambuStatus` or another shared global. The UI should only read that value.

### Example

- ❌ Calling `xtouch_has_public_filaments()` from the UI
- ✅ Main sets `bambuStatus.has_public_filaments = (file exists) ? 1 : 0` at startup; the UI reads `bambuStatus.has_public_filaments`

Similarly, SD/file existence, settings read/write, etc. should be done on the **Main side**, and the result passed to the UI via **bambuStatus**, **xTouchConfig**, or similar.

## 方針: コンポーネントで xtouch に依頼したいときは「イベント送信 → xtouch 側で購読して呼ぶ」

**ui_comp_\*.c からは xtouch を include せず、呼び出しも行わない。** 代わりに次の形にする。

1. **コンポーネント**: やりたいこと用のメッセージを ui_msgs.h に定義し、`lv_msg_send(メッセージID, &payload)` で送るだけにする。
2. **xtouch 側**: mqtt.h の `xtouch_mqtt_subscribe_commands` のように、**xtouch の .h 内で** `lv_msg_subscribe(メッセージID, callback, NULL)` を登録し、コールバック内で `xtouch_*` を呼ぶ。main の setup でその購読登録関数を一度呼ぶ。

こうすると、UI はイベントを送るだけで、**呼び出しは xtouch の購読に集約**され、ui.h に xtouch の宣言を並べる必要がない。**表示用データ（例: サムネイルのパス）は types.h の共有構造体・配列に xtouch が書き、UI はそれを参照するだけ**にし、`xtouch_*` の直接呼び出しは避ける。

### 実例: Printers 画面のサムネイル全スロット取得

- **メッセージ**: `XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH`（ui_msgs.h で定義）
- **ui_comp_printerscomponent.c**: コンポーネント作成時に `lv_msg_send(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, &eventData)` のみ送信。`xtouch/thumbnail.h` は include しない。
- **xtouch 側で購読**: mqtt.h の `xtouch_mqtt_subscribe_commands` と同様に、**xtouch/thumbnail.h** で `lv_msg_subscribe(XTOUCH_PRINTERS_SCHEDULE_THUMB_FETCH, callback, NULL)` を登録。main の setup で `xtouch_thumbnail_subscribe_events()` を一度呼ぶ。UI は送信だけし、**呼び出しは xtouch の購読コールバック**で行う（ui.h に宣言を並べず、イベント定義＋xtouch 側購読で完結）。
