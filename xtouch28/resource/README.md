このフォルダについて
======================

- この `resource` フォルダに置いたファイルは、Chrome 拡張の「Download filaments ZIP」ボタンを押したときに作られる ZIP に、`/resource/` 以下として同梱されます。
- 現在の実装では、`popup.js` 内の `resourceFiles` 配列に列挙したファイルだけが ZIP に含まれます。
  - 例: `const resourceFiles = ["logo.png"];`
  - 新しい画像やファイルを追加したときは、`resourceFiles` にファイル名を足す必要があります。

ZIP の中身（想定）
------------------

- `/provisioning.json`  
  - 拡張側で入力した Wi-Fi / クラウド設定などを含む JSON。
- `/resource/*`  
  - このフォルダに入っていて、`resourceFiles` に列挙されているファイル。
- `/xtouch/filament/*`  
  - フィラメント一覧 (`filaments_brands.txt`, `filaments_*.txt`) と `json/*.json`。

注意
----

- フォルダ名・ファイル名を変更した場合は、`popup.js` の対応箇所も一緒に更新してください。
- 実際に ZIP の中身を確認してから配布・使用するようにしてください。

ロゴ画像 (`logo.png`) について
------------------------------

- `logo.png` は、xptouch 本体の Home / Printers 画面で **Idle 時のサムネイル**として使われます。
- ファーム側は PNG を専用デコーダ（pngle）で読み込んでいるため、**PNG のフォーマットに制約があります**。
- もっとも安全なのは、**プリンタからダウンロードしたサムネイル PNG をベースにしてロゴを作る**方法です。

推奨の作り方（例）
-------------------

1. プリンタからダウンロードしたサムネイル（`/tmp/*.png`）を PC にコピーする。
2. その PNG を画像編集ソフトで開き、上からロゴ画像を貼り付けて編集する。
3. 書き出し時は、以下のような条件で PNG を保存する。
   - 形式: **PNG**
   - カラーモード: **RGB（TrueColor）**
   - ビット深度: **8bit/チャンネル**
   - インターレース: **なし**
   - 余計なメタデータ（カラープロファイルやコメントなど）は可能なら削除
4. 画像変換ツール（例: ImageMagick）が使える場合は、次のように再変換すると安全です。
   - `magick input.png -define png:color-type=2 -depth 8 -strip -interlace none logo.png`
5. 最終的な `logo.png` を SD カードの `/resource/logo.png` として配置する。

注意点
------

- Photoshop で「開いてそのまま保存」した PNG は、プロファイルやチャンク構成の違いにより **pngle で読み込めない場合があります**。
- その場合でも上記のように、**サムネイル PNG を元にして、ImageMagick 等でフォーマットを正規化したファイル**であれば動作しやすくなります。

Windows で ImageMagick を使う場合
---------------------------------

- Windows 11/10 であれば、以下のコマンドで ImageMagick（Q8版）をインストールできます。
  - `winget install ImageMagick.Q8`
- インストール後は、PowerShell などで `magick` コマンドが使えるようになります。

