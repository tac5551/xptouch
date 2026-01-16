# 開発環境のセットアップ

ファームウェアのビルドを行う場合、以下の開発環境のセットアップが必要です。

## 必要なツール

- **Python**: PlatformIOのビルドシステムで使用されます。Python 3.7以上が必要です。
  - [Python公式サイト](https://www.python.org/downloads/)からダウンロードできます。
  - インストール時に「Add Python to PATH」にチェックを入れることを推奨します。
- **Node.js**: ビルドスクリプトでエラーデータのダウンロードに使用されます。Node.js 14以上が必要です。
  - [Node.js公式サイト](https://nodejs.org/)からLTS版をダウンロードできます。
  - インストール時に自動的にPATHに追加されます。
- **PlatformIO**: ESP32のファームウェアをビルドするために必要です。
  - VS Codeの拡張機能としてインストールするか、コマンドラインからインストールできます。
  - [PlatformIO公式サイト](https://platformio.org/)を参照してください。
- **LovyanGFX（tac-lab version）**: グラフィックスライブラリとして使用されます。
  - このプロジェクトには`lib/LovyanGFX`ディレクトリに含まれています。
  - [tac-lab版LovyanGFX](https://github.com/tac5551/LovyanGFX)は、xptouchで使用する各種ボードに対応したカスタマイズ版です。
  - PlatformIOが自動的にこのライブラリを認識して使用します。

## esptool（ESP32用ツール）

ビルド後のファームウェアマージ処理で使用されます。

- PlatformIOをインストールすると自動的に`~/.platformio/packages/tool-esptoolpy`にインストールされます。
- ビルドスクリプトで`esptool.py`を直接実行するため、PATH設定は不要です（スクリプト内で自動的にパスを解決します）。

### 以前のバージョンでのPATH設定（参考）

以前のバージョンでは、PATHに追加する必要がありました。現在のバージョンでは不要ですが、参考として記載します。

- **Windows環境でのPATH設定方法**:
  - システムの環境変数に以下を追加してください：
    ```
    %USERPROFILE%\.platformio\packages\tool-esptoolpy
    ```
  - または、PowerShellで一時的に設定する場合：
    ```powershell
    $env:PATH += ";$env:USERPROFILE\.platformio\packages\tool-esptoolpy"
    ```
- **Linux/macOS環境でのPATH設定方法**:
  - `~/.bashrc`または`~/.zshrc`に以下を追加：
    ```bash
    export PATH="$HOME/.platformio/packages/tool-esptoolpy:$PATH"
    ```

## カスタムボード定義ファイル

ESP32-S3-DevKitC-1-N16R8ボードを使用する場合、カスタムボード定義ファイルをコピーする必要があります。

- プロジェクトの`docs/boards/esp32-s3-devkitc1-n16r8.json`をPlatformIOのboardsディレクトリにコピーしてください。
- **Windows環境でのコピー方法**:
  ```powershell
  Copy-Item "docs\boards\esp32-s3-devkitc1-n16r8.json" "$env:USERPROFILE\.platformio\boards\esp32-s3-devkitc1-n16r8.json"
  ```
- **Linux/macOS環境でのコピー方法**:
  ```bash
  cp docs/boards/esp32-s3-devkitc1-n16r8.json ~/.platformio/boards/esp32-s3-devkitc1-n16r8.json
  ```
- コピー後、PlatformIOを再起動するか、新しいターミナルを開いてビルドを実行してください。

## 注意事項

- ファームウェアのビルドを行わない場合（事前にビルドされたファームウェアを使用する場合）は、PythonとNode.jsは不要です。
