# xptouch コーディングルール

## xptouch 内部実装（xtouch/*）

- **原則として .h ファイルとして作成し、main.cpp で include して使い回す。**
- 実装を .h に書き、main がその .h を include することで 1 か所で定義し、他からは types.h 等の宣言経由で呼び出す。paths.h や filaments_rev.h と同様のパターンとする。

## ui_ 関連ファイル（ui/*）

- **外部ツールで生成し直すことがあるため、極力個別実装を追加しない。**
- ui_comp_*.c / ui_*Screen.c 等は LVGL の自動生成やツールで再生成される可能性がある。カスタムロジックは最小限にし、必要な振る舞いはイベント経由で xtouch 側に寄せる。

## UI から xtouch の呼び出し

- **xtouch の内部実装関数の呼び出しは、イベント（lv_msg 等）経由で行う。**
- UI コンポーネントから直接 `xtouch_*` を呼ばず、メッセージを送信し、main または xtouch 層で受けて処理する。これにより UI と xtouch の結合を避け、ui_ ファイルの再生成時にも影響を抑えられる。

## 注意（ESPDEBUG）

- **ESPDEBUG 機能をオンにした場合、ヒープ領域が不足し、AMS Edit 画面が動作しなくなる可能性がある。** デバッグ時以外はオフにしておくことを推奨する。

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
