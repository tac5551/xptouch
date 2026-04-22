Chrome# XPTouch (XPerimentsTouch) - 3Dプリンター制御システム

## プロジェクト概要 / Project Overview

BambuLabプリンター用の高度なタッチスクリーン制御システムです。
直感的なユーザーインターフェースと豊富な機能を提供し、3Dプリンターの操作をより簡単で効率的にします。
**このプロジェクトではタッチUIを持たないBambulabP1Sをターゲットに開発しています。**

X1C、X1EおよびP2S / H2S / A1シリーズなどの**シングルノズル機**についても、プロトコル的には同等のため**動作可能と思われますが、現状は動作未確認**です。  
これらの機種は標準のタッチパネルを持っているため、追加のメリットがあまりないため、メインターゲットとしていません。  
**P2S/ X2D / H2D / H2C についてはLANモード専用サポート**です。（クラウド経由の場合情報取得ができません）

P1S Version 1.7以前専用にチューニングをしているため、1.8以降にアップグレードされた個体では使用できません。BambuHandyからダウングレードが必要となります。
Vesion 1.9移行最新のファームウェアをする場合にはLanモードに切り替えLAN接続する必要があります。
（クラウド接続の場合BambuCloud側の制限によりデータの取得はできるが不正なMQTT扱いとなりエラーとなります。）

- ダウングレード手順: [Firmware Downgrade | Bambu Lab Wiki](https://wiki.bambulab.com/en/knowledge-sharing/firmware-downgrade)
- ローカルモード接続: `xtouch.json` を作成して SD カードのルートに配置してください（手順）: `https://tac-lab.tech/xptouch-bin/localOnly.html`

**バージョン 0.0.60 以降では AMS エディタ機能が追加されています。この機能を利用するには、xptouch 本体のファームウェア更新に加えて、Chrome 拡張機能も最新版へ更新してください。**

XPTouch is an advanced touchscreen control system for BambuLab printers.  
It provides an intuitive UI and rich controls to simplify printer operation.  
The primary target is BambuLab P1S (especially models without native touch UI).

Single-nozzle models such as X1C, X1E, and P2S / H2S / A1 are protocol-compatible in theory,  
so they may work, but they are currently unverified.  
Because these models already have native touch panels, they are not the main target of this project.  
P2S / X2D / H2D / H2C are supported in **LAN mode only** (cloud mode cannot retrieve required data).

This project is tuned for P1S firmware up to version 1.7.  
Devices upgraded to 1.8 or later are not supported as-is and require a downgrade (via Bambu Handy).  
For firmware 1.9 and later, switch to LAN mode and use LAN connection.  
In cloud mode, BambuCloud restrictions may allow partial data retrieval, but MQTT is treated as unauthorized and fails.

- Downgrade guide: [Firmware Downgrade | Bambu Lab Wiki](https://wiki.bambulab.com/en/knowledge-sharing/firmware-downgrade)
- LAN mode setup: Create `xtouch.json` and place it in the SD card root (guide): `https://tac-lab.tech/xptouch-bin/localOnly.html`

**From version 0.0.60 onward, the AMS editor feature is available. To use it, update both the xptouch firmware and the Chrome extension to the latest versions.**

## 目次 / Table of Contents

1. [はじめに / Getting Started](#はじめに--getting-started)
  - [ハードウェア / Hardware](#ハードウェア--hardware)
  - [オプションハードウェア / Optional Hardware](#オプションハードウェア--optional-hardware)
  - [xptouchスクリーンの電源供給 / Powering the xptouch Screen](#xptouchスクリーンの電源供給--powering-the-xptouch-screen)
2. [セットアップとインストール / Setup and Installation](#セットアップとインストール--setup-and-installation)
  - [必要なツールと準備 / Required Tools and Preparation](#必要なツールと準備--required-tools-and-preparation)
  - [Chrome拡張機能のインストール / Installing the Chrome Extension](#chrome拡張機能のインストール--installing-the-chrome-extension)
  - [ファームウェアのインストール / Firmware Installation](#ファームウェアのインストール--firmware-installation)
  - [Bambulabアカウントとの紐づけ / Link with Your Bambulab Account](#bambulabアカウントとの紐づけ--link-with-your-bambulab-account)
    - [初回プロビジョニング / Initial Provisioning](#初回プロビジョニング--initial-provisioning)
    - [トークン更新 / Token Renewal](#トークン更新--token-renewal)
    - [LAN Only Mode](#lan-only-mode--lan-only-mode)
  - [タッチパネルのキャリブレーション / Touch Panel Calibration](#タッチパネルのキャリブレーション--touch-panel-calibration)
  - [プリンターのリンク / Link a Printer](#プリンターのリンク--link-a-printer)
3. [Bambulab P1Sへの接続 / Connect to Bambulab P1S](#bambulab-p1sへの接続--connect-to-bambulab-p1s)
4. [スクリーン / Screens](#スクリーン--screens)
  - [メインスクリーン / Main Screen](#メインスクリーン--main-screen)
  - [温度/ファンスクリーン / Temperature/Fan Screen](#温度ファンスクリーン--temperaturefan-screen)
  - [制御スクリーン / Control Screen](#制御スクリーン--control-screen)
  - [フィラメントスクリーン / Filament Screen](#フィラメントスクリーン--filament-screen)
  - [設定スクリーン / Settings Screen](#設定スクリーン--settings-screen)
5. [アップデート / Updates](#アップデート--updates)
  - [OTAアップデート手順 / OTA Update Procedure](#otaアップデート手順--ota-update-procedure)
  - [SDファームウェアアップデート / SD Firmware Update](#sdファームウェアアップデート--sd-firmware-update)
6. [トラブルシューティング / Troubleshooting](#トラブルシューティング--troubleshooting)
7. [5インチ LCD / Bus 設定（JC8048W550・lcd.json）](#5インチ-lcd--bus-設定jc8048w550lcdjson--5-inch-lcdbus-settings-jc8048w550-lcdjson)

## はじめに / Getting Started

xptouchスクリーンは、BambuLabプリンターに革命的な機能を追加し、ユーザーエクスペリエンスを向上させ、高度な制御とモニタリング機能を提供します。
この詳細なREADME.mdガイドでは、製品の機能、インストールプロセス、および各スクリーンの機能について説明します。

**English**

The xptouch screen adds advanced control and monitoring features to BambuLab printers.
This README explains hardware options, installation, provisioning, and screen functions.

### ハードウェア / Hardware

image image

現在、手頃な価格のCYD呼ばれる開発ボードで動作するように拡張しています。
用途や入手性に合わせて、選択できます。
各種サポートを進めていますがすべてに対応することは難しいため、2432S028もしくは、JC2432W328Rがいろいろなところから入手可能なため、お勧めです。
2.4および3.2インチのモデルについてはコントローラーが同一のため恐らく使用可能ですが、テストは実施していません。

**English**

The project currently focuses on affordable CYD-class boards.
2.8-inch models are the most recommended and easiest to source.
Some 2.4-inch and 3.2-inch boards may work, but they are not fully verified.

*推奨ボード(320ｘ240)


| ボード         | 2432S028R                                           | 2432S028R(USB-C)                                   | JC2432W328R                                         | JC2432W328C                                         | 2432S024C                                                     | 2432S032                                            |
| ----------- | --------------------------------------------------- | --------------------------------------------------- | --------------------------------------------------- | --------------------------------------------------- | ------------------------------------------------------------- | --------------------------------------------------- |
| サイズ         | 2.8 inch                                            | 2.8 inch                                            | 2.8 inch                                            | 2.8 inch                                            | 2.4 inch                                                      | 3.2 inch                                            |
| 動作確認        | **Yes**                                             | **Yes**                                             | **Yes**                                             | **Yes**                                             | Probably works                                                | Probably works                                      |
| 温度センサー      | **Yes**                                             | **Yes**                                             | **Yes**                                             | **Yes**                                             | **Yes**                                                       | **Yes**                                             |
| NexpixelLED | **Yes**                                             | Yes                                                 | **Yes**                                             | **Yes**                                             | **Yes**                                                       | **Yes**                                             |
| バッテリ装着      | No                                                  | No                                                  | **Yes**                                             | **Yes**                                             | No                                                            |                                                     |
| 購入先         | [購入リンク](https://s.click.aliexpress.com/e/_c3fGqpNT) | [購入リンク](https://s.click.aliexpress.com/e/_c36MHgF7) | [購入リンク](https://s.click.aliexpress.com/e/_c3V5Eyjj) | [購入リンク](https://s.click.aliexpress.com/e/_c3V5Eyjj) | [購入リンク](https://ja.aliexpress.com/item/1005012039563169.html) | [購入リンク](https://s.click.aliexpress.com/e/_c4miViDf) |
| 注意事項        |                                                     |                                                     |                                                     |                                                     | 検証予定なし                                                        | 検証予定なし                                              |


*推奨ボード(480ｘ320)


| ボード                  | 3248S35C                                                                                                | JC3248W535C | JC3248W535R    |
| -------------------- | ------------------------------------------------------------------------------------------------------- | ----------- | -------------- |
| サイズ                  | 3.5 inch                                                                                                | 3.5 inch    | 3.5 inch       |
| 動作確認                 | **Yes**                                                                                                 |             | Probably works |
| 温度センサー               | **Yes**                                                                                                 | **Yes**     | **Yes**        |
| ステータスLED NexpixelLED | **Yes**                                                                                                 | **Yes**     | **Yes**        |
| バッテリ装着               | No                                                                                                      | **Yes**     | **Yes**        |
| 購入先                  | [購入リンク](https://s.click.aliexpress.com/e/_c3APFetR) [購入リンク](https://s.click.aliexpress.com/e/_c4oUvBW5) | 販売終了？       | 販売終了？          |
| 注意事項                 |                                                                                                         | 検証予定なし      | 検証予定なし         |


*推奨ボード(800x480)


| ボード                  | JC8048W550                                                                                             | 8048S043                                            | 8048S050                                            |
| -------------------- | ------------------------------------------------------------------------------------------------------ | --------------------------------------------------- | --------------------------------------------------- |
| サイズ                  | 5.5 inch                                                                                               | 4.3 inch                                            | 5.0 inch                                            |
| 動作確認                 | **Yes**                                                                                                | Probably works                                      | Probably works                                      |
| 温度センサー               | **Yes**                                                                                                | **Yes**                                             | **Yes**                                             |
| ステータスLED NexpixelLED | **Yes**                                                                                                | **Yes**                                             | **Yes**                                             |
| バッテリ装着               | **Yes**                                                                                                | No                                                  | No                                                  |
| 購入先                  | [購入リンク](https://s.click.aliexpress.com/e/_okFyaE4) [購入リンク](https://s.click.aliexpress.com/e/_c3GmfL9F) | [購入リンク](https://s.click.aliexpress.com/e/_c3q54KiD) | [購入リンク](https://s.click.aliexpress.com/e/_c4qhBxsZ) |


*制限付き互換ボード


| ボード                  | E32-28T                                             | E32-28T_7789                                        | E32-32T                                             |
| -------------------- | --------------------------------------------------- | --------------------------------------------------- | --------------------------------------------------- |
| サイズ                  | 2.8 inch                                            | 2.8 inch                                            | 3.2 inch                                            |
| 動作確認                 | **Yes**                                             | **Yes**                                             | Probably works                                      |
| 温度センサー               | Not Support                                         | Not Support                                         | Not Support                                         |
| ステータスLED NexpixelLED | Not Support                                         | Not Support                                         | Not Support                                         |
| バッテリ装着               | **Yes**                                             | **Yes**                                             | **Yes**                                             |
| 購入先                  | [購入リンク](https://s.click.aliexpress.com/e/_c3IEJhcZ) | [購入リンク](https://s.click.aliexpress.com/e/_c3IEJhcZ) | [購入リンク](https://s.click.aliexpress.com/e/_c3IEJhcZ) |


*480ｘ320モデル(動作不可 ※将来的にサポート予定)


| **No.** | 購入元      | ボード名                    | サイズ      | 購入リンク                                               |
| ------- | -------- | ----------------------- | -------- | --------------------------------------------------- |
| **1**   | Sonton   | 3248S035                | 3.5 inch |                                                     |
| **2**   | GUITION  | 3248S035                | 3.5 inch |                                                     |
| **3**   | GUITION  | JC3248W535              | 3.5 inch | [販売終了]                                              |
| **4**   | IPistBit | ESP-32E(3.5 with touch) | 3.5 inch | [購入リンク](https://s.click.aliexpress.com/e/_c3IEJhcZ) |
| **5**   | IPistBit | ESP-32E(4.0 with touch) | 4.0 inch | [購入リンク](https://s.click.aliexpress.com/e/_c3IEJhcZ) |


*800ｘ480インチモデル (利用不可 対応不可）


| **No.** | 購入元     | ボード名     | サイズ      | 購入リンク                                               |
| ------- | ------- | -------- | -------- | --------------------------------------------------- |
| **1**   | GUITION | 8048S070 | 7.0 inch | [購入リンク](https://s.click.aliexpress.com/e/_c4OgPvgh) |


P1sに取り付ける3Dモデルは以下のリンクで見つけることができます
xtouchとかP1touch用に作られてているものがPrintableやMakerWorldに多数あり。


| **No.** | モデル名                                       | リンク                                                   |            |
| ------- | ------------------------------------------ | ----------------------------------------------------- | ---------- |
| **1**   | xptouch flip enclosure for Bambu Lab P1P/S | [Printables](https://www.printables.com/model/611634) | 2432R028   |
| **2**   | xptouch flip enclosure for Bambu Lab P1P/S | [MakerWorld](https://makerworld.com/ja/models/920980) | JC8048W550 |


---

### オプションハードウェア / Optional Hardware

- **[DS18B20温度センサー](docs/temperature-sensor.md)**: チャンバー温度センサーが内蔵されていないプリンターの場合、外部DS18B20温度センサーを追加するオプションがあります。このセンサーは正確なチャンバー温度測定を提供し、xptouchスクリーンの機能を向上させます。スクリーンのコネクターは1.25 MZ JST 4Pです。
- **[NeoPixel LED](https://s.click.aliexpress.com/e/_c4WmQpq5)**: NEOPIXEL LEDリボンを利用して、LEDステータスバーを追加するオプションがあります。ステータスバーは印刷中のステータスに応じて表現されます。

**English**

-  **[DS18B20 temperature sensor](docs/temperature-sensor.md)** :You can add an external for chamber temperature monitoring.
-  **[NeoPixel LED strip](https://s.click.aliexpress.com/e/_c4WmQpq5)** :You can also add afor print status indication.

#### GPIO 配線（2.8インチ / 5インチ） 

- **2.8インチ環境（`env:esp32dev`）**
  - **DS18B20**: 信号線 → GPIO22（`XTOUCH_CHAMBER_TEMP_PIN`）。3.3V / GND は基板の 3.3V / GND に接続し、4.7kΩ で信号線と 3.3V をプルアップしてください。
  - **NeoPixel**: 信号線 → 基板により **GPIO21 または GPIO27**（CYD 2432S028R / JC2432W328R/C は 21、2432S028_7789 / 2432S028_9341 は 27）。5V 給電時も信号は 3.3V ロジックを前提にしてください。
- **5インチ環境（`env:esp32-s3dev`, JC8048W550）**
  - **DS18B20**: 信号線 → GPIO18（5インチ時に `XTOUCH_CHAMBER_TEMP_PIN` を 18 に切替）。3.3V / GND と 4.7kΩ プルアップは 2.8インチと同様です。
  - **NeoPixel**: 信号線 → GPIO17（5インチ環境では `xTouchConfig.xTouchNeoPixelPinValue` が 17 に設定されます）。
  - **RGB パネル / 表示の個体差調整**: 5インチのみ。手順は後述の **[5インチ LCD / Bus 設定](#lcd-json-5inch)** を参照。

####  GPIO Wiring (2.8-inch / 5-inch)

- **2.8-inch (**`env:esp32dev`**)**
  - DS18B20 signal: GPIO22 (with 4.7k pull-up to 3.3V)
  - NeoPixel signal: GPIO21 or GPIO27 depending on board
- **5-inch (**`env:esp32-s3dev`**, JC8048W550)**
  - DS18B20 signal: GPIO18
  - NeoPixel signal: GPIO17





温度センサーは以下のリンクで購入できます：

- [DS18B20 温度センサーB](https://s.click.aliexpress.com/e/_oBYP2pE)
- **DS18B20 P1P/P1Sチャンバー温度エンクロージャー**: さらに、DS18B20温度センサーをP1P/P1Sプリンター内に簡単に統合できる3Dモデルを作成しました。
3Dモデルは以下のリンクで見つけることができます：  
　- [モデルA](https://makerworld.com/en/models/19658)

　`<img src="https://github.com/xperiments-in/xtouch/assets/417709/a8d14564-09e9-4d36-9ad9-10fd8f295c86" width="200"><br>`
　- [モデルB](https://makerworld.com/en/models/42533)   

　`<img src="https://github.com/xperiments-in/xtouch/assets/417709/22871bdf-ba37-44f0-a4b3-33c6352f7f86" width="300">`

---

# セットアップとインストール / Setup and Installation

このガイドでは、3Dプリンターで使用するためのxptouchスクリーンを初期化および設定するために必要な手順について説明します。
**このプロセスにはGoogle Chromeが必要**です。これは必要なツールと拡張機能をサポートしているためです。

**English**

This section describes how to initialize and configure xptouch for your printer.
Google Chrome is required because the installer and extension depend on Chrome features.

---

## 必要なツールと準備 / Required Tools and Preparation

開始前に、以下を確保してください：

**English**

Before you start, prepare:

- a PC with Google Chrome
- a USB data cable
- a FAT32 microSD card (recommended 32GB or less)
- CH340 driver if needed
- xptouch Chrome extension and online installer URL

1. **Google Chromeがインストールされたコンピューター**
  - このプロセスには、Google Chromeがインストールされたラップトップまたはデスクトップコンピューターが必要です。
   まだインストールしていない場合は、[https://www.google.com/chrome/](https://www.google.com/chrome/)からChromeをダウンロードしてください。
2. **USBケーブル**
  - コンピューターにxptouchスクリーンを接続するための互換性のあるUSBケーブル。
3. **FAT32フォーマットされたSDカード（32GB以下推奨）**
  - **32GB以下**の容量で、FAT32ファイルシステムでフォーマットされたmicroSDカード。
  - 初回セットアップ時にプロビジョニングファイルを転送するために必要です。信頼性の高いSDカードを使用し、問題を避けるために適切にフォーマットされていることを確認してください。
4. **ドライバーのインストール**コンピューターがxptouchスクリーンを認識しない場合、必要なCH340ドライバーをインストールする必要がある場合があります：
  - [CH340ドライバーのダウンロード](https://www.wch.cn/download/CH341SER_ZIP.html)
  - [CH340ドライバーインストールチュートリアル](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all)
5. **Chromeブラウザ拡張機能**
  - このリンクから**拡張機能をダウンロード**してください：**[Chrome拡張機能のダウンロード](https://tac-lab.tech/xptouch-bin/extensions/xptouch28.zip)**
  - この拡張機能は、初期設定ファイルの生成とxptouchスクリーンのリモート管理に必要です。
  - ダウンロードしたファイルを、ドキュメント内の専用「xptouch」フォルダーやバックアップドライブなどの安全な場所に保存してください。
6. **xptouchオンラインインストーラー**
  - このプラットフォーム毎にインストールURLが異なります。
   **[2.8インチ向け](https://tac-lab.tech/xptouch-bin/2.8/)**
   **[5.5インチ向け](https://tac-lab.tech/xptouch-bin/5.0/)**

**セットアッププロセス中にxptouchスクリーンが信頼性の高い電源に接続されていることを確認してください。**

---

## Chrome拡張機能のインストール / Installing the Chrome Extension

  Chrome拡張機能は、初期設定ファイルの生成とxptouchスクリーンのリモート管理に不可欠なツールです。特にBambuLabトークンは3ヶ月ごとに期限切れになるためです。**オンラインインストーラーを続行する前にインストールする必要があります。インストーラーは完了時にBambuLabにリダイレクトするためです。**

**English**

The Chrome extension is required for provisioning and ongoing token refresh.
Install it before using the online installer.

### 拡張機能のダウンロードとインストール / Download and Install the Extension

1. **拡張機能のダウンロード**このリンクをクリックしてChrome拡張機能をダウンロードしてください：**[Chrome拡張機能のダウンロード](https://tac-lab.tech/xptouch-bin/extensions/xptouch28.zip)**
2. **拡張機能の安全な保存**
  - ダウンロードした `.zip`ファイルを、ドキュメント内の専用「xptouch」フォルダーやバックアップドライブなどの安全な場所に保存してください。
  - これにより、ファイルが誤って削除されることを防ぎます。
3. **拡張機能の解凍**
  - `.zip`ファイルの内容を安全な場所に解凍してください。
4. **Chromeに拡張機能を追加**
  - **Google Chrome**を開き、**chrome://extensions/**に移動してください。
  - 右上隅のスイッチを切り替えて**開発者モード**を有効にしてください。
  - **パッケージ化されていない拡張機能を読み込む**をクリックし、拡張機能ファイルを解凍したフォルダーを選択してください。
  - 拡張機能がインストールされ、Chromeで表示されるようになります。
5. **簡単なアクセスのための拡張機能のピン留め**
  - Chromeの右上隅にある**拡張機能**アイコン（パズルピース）をクリックしてください。
  - リスト内のxptouch拡張機能を見つけ、その横にある**ピンアイコン**をクリックしてください。
  - 拡張機能がChromeツールバーに表示され、簡単で迅速なアクセスが可能になります。

---

## ファームウェアのインストール / Firmware Installation

**English**

Use the online installer from Chrome and choose the URL matching your board class.
After connecting the serial port, click install to flash the firmware.

### ステップ1: Google Chromeでオンラインインストーラーを開く / Step 1: Open the Online Installer in Google Chrome

1. コンピューターで**Google Chrome**を開いてください。
  > **注意:** このプロセスでは他のブラウザーはサポートされていません。
2. 以下のURLを入力してインストーラーページに移動してください：

  | 主なボード             | インストーラーURL                                                 |
  | ----------------- | ---------------------------------------------------------- |
  | generic ESP32 CYD | [for ESP32CYD](https://tac-lab.tech/xptouch-bin/2.8/)      |
  | ES3C28P           | [for ES3C28P](https://tac-lab.tech/xptouch-bin/s3_2.8/)    |
  | JC3248W535        | [for JC3248W535](https://tac-lab.tech/xptouch-bin/s3_3.5/) |
  | JC8048W550        | [for JC8048W550](https://tac-lab.tech/xptouch-bin/5.0/)    |


### ステップ2: オンラインインストーラーの使用 / Step 2: Use the Online Installer

1. ウェブページで**「接続」**ボタンをクリックして、コンピューターとxptouchスクリーン間の接続を確立してください。
2. 表示された利用可能なポートのリストから、xptouchスクリーンに割り当てられたシリアルポートを選択してください。
3. 接続後、**「xptouchをインストール」**ボタンをクリックしてインストールプロセスを開始してください。

**English**

1. Click **Connect** on the installer page.
2. Select the correct serial port.
3. Click **Install xptouch**.

---

## Bambulabアカウントとの紐づけ / Link with Your Bambulab Account

xptouchではWifi経由でBambuCloudに接続する為、Wifiの接続設定とBamulabのアカウントと紐詰けが必要です。
初回セットアップ時と3ヶ月ごとに、プロビジョニングする必要があります。

**English**

xptouch uses Wi-Fi and BambuCloud credentials.
Provisioning is required at first setup and periodically (about every 3 months) for token renewal.

**Chrome拡張機能をする場合、事前に[bambulab.com](https://www.bambulab.com)または[bambulab.cn](https://www.bambulab.cn)にログインしている必要があります。**

クラウド接続を行う場合以下のバージョンのBambu Labプリンターファームウェアと互換性があります

P1P/S: 01.08.01.00
X1C --> 01.08.02.00
X1E --> 01.01.02.00
A1 --> 01.04.00.00
A1 Mini --> 01.04.00.00

---

### 初回プロビジョニング / Initial Provisioning

Chrome拡張機能を初めて開く際は、以下の手順に従ってください：
`<img src="readme-assets/ChromeExtention.png" width="300">`

1. **必要な情報の入力**
  - 拡張機能は以下の詳細の入力を求めます：
    - **SSID**: Wi-Fiネットワーク名。
    - **SSID password**: Wi-Fiパスワード。
    - **Xtouch Lite IP**: これは初回プロビジョニングでデフォルトで `0.0.0.0`に事前設定されており、正しい設定です。これを変更しないでください。
2. **xptouchスクリーンのプロビジョニング**
  - SSIDとパスワードを入力した後、IPを `0.0.0.0`のままにして**「Provision xptouch」**ボタンをクリックしてください。
3. **プロビジョニングファイルのダウンロード**
  - ボタンをクリックした後、**「Download provisioning file」**ボタンが表示されます。
  - リモートプロビジョニングが失敗したことを示すエラーも表示される場合があります。**これは初回プロビジョニングでは正常です。**
  - **「プロビジョニングファイルをダウンロード」**をクリックして `provisioning.json`ファイルをダウンロードしてください。
4. **プロビジョニングファイルの保存と挿入**
  - ダウンロードした `provisioning.json`ファイルをSDカードのルートディレクトリに保存してください。
  - SDカードをxptouchスクリーンに挿入して再起動してください。
5. **接続の確認**
  - 再起動後、すべてが正しく設定されている場合、xptouchスクリーンはWi-FiネットワークとBambuLabサーバーに接続されます。

**English**

For first-time provisioning, keep IP as `0.0.0.0`, generate/download `provisioning.json`,
place it in SD root, then reboot with the SD card inserted.

---

### トークン更新 / Token Renewal

3ヶ月ごとに、BambuLabトークンが期限切れになると、xptouchスクリーンはトークンデータをクリアします。画面に以下の指示を含むメッセージが表示されます：
**「xxx.xxx.xxx.xxxでプロビジョニング」**
これは、スクリーンが再プロビジョニングを必要としていることを示しています。

**English**

When tokens expire, xptouch asks for re-provisioning.
Log in to BambuLab in Chrome, open the extension, enter the shown IP, and provision again.

#### 再プロビジョニングの手順： / Re-provisioning Steps

1. **BambuLabにログイン**
  - [bambulab.com](https://www.bambulab.com)または[bambulab.cn](https://www.bambulab.cn)を開き、まだログインしていない場合はアカウントにログインしてください。
2. **Chrome拡張機能を開く**
  - Chrome拡張機能を起動してください。
3. **スクリーンのIPアドレスを入力**
  - xptouchスクリーンに表示されているIPアドレスを拡張機能の**IP**フィールドに入力してください。
4. **xptouchスクリーンのプロビジョニング**
  - **「xptouchをプロビジョニング」**ボタンをクリックしてください。
  - 拡張機能は必要なトークンをxptouchスクリーンに送信します。
5. **自動再起動**
  - 数秒以内に、xptouchスクリーンは新しい設定を適用するために自動的に再起動します。

---

### LAN Only Mode / LAN Only Mode

Lan Only Modeをサポートしました。

[設定ページ](https://tac-lab.tech/xptouch-bin/localOnly.html)でxtouch.jsonを作成してください
※互換性のためxtouch.jsonであることに注意してください。

以下のバージョンのBambu Labプリンターファームウェアより新しい物を使用する場合
プリンタでLAN Only Modeに設定し、開発者モードを有効にしてください。
ステータスは取得できますが、操作ができなくなります。

P1P/S: 01.08.01.00
X1C --> 01.08.02.00
X1E --> 01.01.02.00
A1 --> 01.04.00.00
A1 Mini --> 01.04.00.00

LAN Only モードと、Cloudモードは排他利用となっています。
LAN Only モードを利用する場合SDカードからProvisioning.jsonを削除してください。

**English**

LAN Only mode is supported.
Create `xtouch.json` from the provided settings page and place it in SD root.
LAN Only and Cloud mode are mutually exclusive. Remove `provisioning.json` when using LAN mode.

---

### タッチパネルのキャリブレーション / Touch Panel Calibration

初回起動時、タッチパネルのキャリブレーションを行う必要があります。
画面の指示にしたがって+が表示されるところをタップしてください

1. **左上をタップ**
2. **右下をタップ**

設定が終わると、touch.jsonに保存されます。
画面が操作できなくなった場合等は、SDカードからtouch.jsonを削除すると再設定が可能です。

**English**

On first boot, follow on-screen touch calibration prompts.
Calibration is saved to `touch.json`; delete it from SD to recalibrate.

---

### プリンターのリンク / Link a Printer

初回起動時、Bambuアカウントに複数の3Dプリンターが見つかった場合、使用するプリンタとリンクする必要があります。接続を正常に確立するには、以下の手順に従ってください：

1. **プリンターの検索**:
  - タッチスクリーンの電源を入れた後、「プリンターを検索中」画面が表示されます。タッチスクリーンは利用可能なプリンターをスキャンします。
2. **リンクするプリンターの選択**:
  - スキャンプロセスが完了すると、見つかったプリンターのリストが表示されます。リンクしたいプリンターを選択してください。
3. **チェックマークボタンをクリック**:
  - 希望するプリンターを選択した後、緑のチェックマークボタンをクリックしてリンクプロセスを開始してください。
4. **成功とメインスクリーン**:
  - タッチスクリーンとプリンターのリンクが正常に完了すると、メインスクリーンにリダイレクトされます。プリンターとタッチスクリーンが接続され、使用準備が整います。

これらの手順に従うことで、xptouchスクリーンを3Dプリンターとシームレスにリンクし、スムーズなユーザーエクスペリエンスとタッチスクリーンのすべての機能への簡単なアクセスを確保できます。

**English**

If multiple printers are found in your account, select one from the list and confirm.
After successful linking, the device returns to the main screen.

---

## Bambulab P1Sへの接続 / Connect to Bambulab P1S

### xptouchスクリーンの電源供給 / Powering the xptouch Screen

xptouchスクリーンは、USBポートで電源供給します。

1. 適切なUSBケーブルをコンピューターまたはUSB電源の利用可能なUSBポートに接続します。
  **おすすめは、P1Sのコントローラーの背面に用意されているUSB電源ポートへの接続です。**
   　ケーブルを本体内に通す際には、右側にある穴を経由して導入してください。
   　[AMS Rizer](https://makerworld.com/ja/models/647484-ams-airflow-slim-riser-slider-p1p-p1s-x1c#profileId-573815)等を導入済みの方は加工して最短距離の配線も可能です。
   　`<img src="readme-assets/USB_Route.png" width="300"><img src="readme-assets/simple_route.png" width="300">`

**English**

Power xptouch via USB. For P1S, the rear USB power port is recommended.
Route the cable through the side opening as shown in the diagrams.

---

## スクリーン / Screens

**English**

The following sections describe each UI page and its key functions.

### メインスクリーン / Main Screen

- **トップバー**: WiFi、カメラ、タイムラプス、AMSステータスを表示。
- **ライト制御**: プリンターのライトのオン/オフ切り替え。
- **温度インジケーター**: リアルタイムのノズル、ベッド、チャンバー温度インジケーター。
- **ステータスエリア**: タッチスクリーンのステータスエリアは2つの主要な目的を果たします：
  - **アイドル状態**: プリンターが使用されていない場合、「準備完了」メッセージとメインロゴを表示します。
  - **印刷状態**: 印刷中は、一時停止/停止ボタン、プログレスバー、レイヤー情報、リアルタイム制御とモニタリングのための印刷速度セレクターを提供します。

**English**

Main screen shows top status indicators, light control, temperatures, and print controls.
During printing, it provides pause/stop, progress, layer info, and speed control.

### 温度/ファンスクリーン / Temperature/Fan Screen

このスクリーンでは、各ボタンが特定のセンサーの温度またはファン速度を表す4つのボタンがあります。これらのボタンのいずれかをタップすると、対応する温度または速度値を簡単に調整できる数値キーボード画面に移動します。変更後、簡単なタップで初期画面に戻ることができます。

**English**

This page provides quick temperature/fan controls.
Tap a target to open the numeric keypad and apply a new value.

### 制御スクリーン / Control Screen

このスクリーンは、プリンターを管理するための重要な制御機能を提供します：

- **ホーミング**: プリンターホーミング手順を開始。
- **XYZ位置制御**: プリントヘッドのXYZ位置の精密制御を有効化。
- **ステップサイズ**: より細かいまたは迅速な調整のために、1mmと10mmのヘッド移動増分の間で切り替えることができます。

**English**

Control screen includes homing, XYZ movement, and move-step selection (1mm/10mm).

### フィラメントスクリーン / Filament Screen

- **フィラメント処理**: フィラメントのロード、アンロード、押し出し、引き込み（注：フィラメント処理は当初AMSのないプリンターでのみ利用可能）。

**English**

Filament screen supports load, unload, extrude, and retract operations.

### 設定スクリーン / Settings Screen

- **LCD**
  - **Back**:
  バックライトの明るさを調整できます。
  - **Sleep**:
  スクリーンのスリープタイマーを調整します。
  指定された時間操作がないと、省電力のためにスリープモードに入ります。0に設定するとスクリーンは点灯したままになります。
  - **Wake On Print**
  印刷開始時に画面がスリープから復帰します。
  - **Wake during Print**
  印字中常に画面が表示されるようになります。手動でのスリープは可能です。
  - **Invert Colors**:
  スクリーン色を反転します。特定のデバイスとの互換性の問題に対処するのに役立ちます。
  - **Flip Screen**:
  スクリーンの向きが上下反転します。電源USBの方向を左右選べます。
- **Chamber LED**
  - **LED Off**
  チャンバーLEDのスリープタイマーを調整します。
  0に設定すると手動でのONOFFが維持されます。
- **OPTIONAL**
  - **Stack Chan Mode**
  スリープ時にスタックチャンが表示されるようになります。
  - **Neopixel**
    - **LEDs**
    接続するNeoPixelの個数を調整できます。0に設定すると制御を行いません。
    - **Blightness**
    LEDバーの明るさを調整できます。
    - **Alarm Timeout**
    印刷完了や、印刷エラー時の表示時間を設定できます。0に設定すると、次の印刷操作をするまで、状態が保持されます。
    - **Idle LED**
    待機状態の時のLEDのOnOffを選べます。Offにすると待機状態でステータスLEDが消灯されます。
- **CONNECTED PRINTERS**
  - **Unlink**
  現在ペアリングされているプリンタの選択を解除します。
  再度プリンタ一覧が表示され、表示するプリンタが再選択できます。
- **XPTOUCH Version**
現在のXPTouchバージョンが表示されます
- **UPDATE**
  - **OTA Update**
  自動OTAアップデートの有効無効を設定します。有効の場合、電源投入時に、自動的に最新のファームウェアが適用されます。
  - **UPDATE Now**
  任意のタイミングでOTAアップデートを実行します。OTA UpdateがOffの場合でもアップデート可能です。
- **Reboot Device**
デバイスが再起動されます。

**English**

Settings include display/LED options, optional feature toggles, printer link management, OTA controls, and reboot.

---

## アップデート / Updates

### OTAアップデート手順 / OTA Update Procedure

xptouchスクリーンはオーバー・ザ・エア（OTA）ファームウェアアップデートをサポートしており、最新の機能と改善でデバイスを最新の状態に保つことが簡単になります。
OTAアップデートを実施するには以下の手順に従ってください：

1. **設定スクリーンにアクセス**:
  - xptouchスクリーンで設定スクリーンに移動してください。
2. **手動アップでード**
  - 設定スクリーンで、UpdateNowを押すと、最新のファームウェアをチェックして適用します。
   OTAUpdateが無効の状態でも、任意のタイミングで適用することができます。
3. **OTAアップデート**:
  - 設定スクリーンで、OTA UPDATEを有効にすると。起動時に自動的にアップデートを適用することができます
  - アップデートが利用可能な場合、タッチスクリーンは最新のファームウェアを自動的にダウンロードしてアップグレードを適用します。

**English**

OTA updates can be triggered manually (`Update Now`) or automatically at boot (`OTA Update` enabled).

### SDファームウェアアップデート / SD Firmware Update

OTAアップデートが利用できない場合、SDカード経由でのアップデートが可能です
手動でアップデートしたい場合、この手順に従ってください：

1. **アップデートファームウェアファイルのダウンロード**:
  - 公式xptouchサイトまたは指定されたファームウェアアップデートソースにアクセスして、最新のファームウェアアップデートファイルをダウンロードしてください。[firmware.bin](https://tac-lab.tech/xptouch-bin/2.8/fw/firmware.bin)という名前でこのファイルをダウンロードしてください。
2. **ファームウェアファイルをSDカードのルートにコピー**:
  - SDカードをコンピューターのカードリーダーに挿入してください。
  - ダウンロードしたファームウェアアップデートファイル[firmware.bin](https://tac-lab.tech/xptouch-bin/2.8/fw/firmware.bin)をSDカードのルートディレクトリにコピーしてください。サブディレクトリには配置しないでください。
3. **xptouchスクリーンの再起動**:
  - ファームウェアアップデートファイルを含むSDカードを、まだ挿入されていない場合はxptouchスクリーンに挿入してください。
  - タッチスクリーンを再起動してください。電源を切ってから再度電源を入れることで実行できます。
4. **ファームウェアアップデートの適用**:
  - タッチスクリーンが起動すると、SDカード上のファームウェアアップデートファイルの自動的に検出します。
  - タッチスクリーンはファームウェアアップデートを適用します。ダウングレードも可能です。
  - アップデートプロセスは完了まで数分かかる場合があります。この間はタッチスクリーンの電源を切ったり、SDカードを削除したりしないでください。
5. **完了と確認**:
  - ファームウェアアップデートが正常に適用された後、タッチスクリーンはアップデートが完了したことを通知します。
  - タッチスクリーンの設定セクションでファームウェアバージョンを確認し、最新バージョンと一致していることを確認できます。

これらの手順に従うことで、xptouchスクリーンのファームウェアを最新バージョンに手動でアップデートし、最新の機能と改善にアクセスできるようになります。

**English**

If OTA is unavailable, place `firmware.bin` at SD root and reboot.
The device detects and applies the firmware automatically.

---

## トラブルシューティング / Troubleshooting

BambuLabプリンタータッチスクリーンのインストールまたは操作中に問題が発生した場合、以下のトラブルシューティング手順を参照して、一般的な問題を解決できます：

1. **データケーブルの使用**:
  - インストールプロセス中にタッチスクリーンをコンピューターに接続する際は、データケーブル（充電ケーブルだけでなく）を使用していることを確認してください。適切な通信にはデータケーブルが必要です。
2. **SDカードの互換性**:
  - 異なるメーカーのSDカードは動作が異なる場合があることに注意してください。多くのSDカードはシームレスに動作しますが、一部はタッチスクリーンと完全に互換性がない場合があります。SDカードで問題が発生した場合は、問題が解決するかどうかを確認するために、異なるブランドやモデルを試してみることを検討してください。
3. **WiFi接続ループ**:
  - 場合によっては、WiFi認証情報が正しいことを確認した後、デバイスが接続を確立せずに連続ループに入ることを報告するユーザーがいます。この問題は、WiFi接続タイムアウトを調整することで解決できる場合があります。
  - これを行うには、`config.json`ファイルを修正し、タイムアウトの数値パラメーターを含めてください。タイムアウト値を増やすことで、この問題に対処できます。
  - ステップバイステップの手順については、[オンラインconfig.jsonフォーム](https://tac-lab.tech/xptouch-bin/config.html)を使用してください。
4. **プリンター起動時の無限再起動**:
  - 場合によっては、プリンターとxptouchスクリーンがデバイスの電源投入プロセス中に無限再起動サイクルを経験し、xptouchが準備完了状態にならない問題が発生する場合があります。この問題は、`config.json`設定ファイル内の「coldboot」値を調整することで解決できる場合がよくあります。
  - この問題に対処するには、`config.json`ファイルを開き、「coldboot」パラメーターの数値パラメーターを含めてください。このパラメーターは、コールドブート後の初期化のためにシステムが待機する時間をミリ秒で指定します。「coldboot」値を増やすことで、システムに起動プロセスを正常に完了するためのより多くの時間を提供します。
  - ステップバイステップの手順については、[オンラインconfig.jsonフォーム](https://tac-lab.tech/xptouch-bin/config.html)を使用してください。
5. **スクリーンの再キャリブレーション**:
  タッチスクリーンのキャリブレーション問題や位置ずれが発生した場合、SDカードの `xptouch`ディレクトリにある `touch.json`ファイルを削除することでスクリーンを再キャリブレーションできます。以下の手順に従ってください：
  - xptouchスクリーンの電源を切ってください。
  - タッチスクリーンからSDカードを削除してください。
  - SDカードをコンピューターに挿入してください。
  - SDカード上の `xptouch`ディレクトリに移動してください。
  - `touch.json`ファイルを削除してください。
  - コンピューターからSDカードを安全に取り出してください。
  - SDカードをタッチスクリーンに再挿入してください。
  - タッチスクリーンの電源を入れてください。
   タッチスクリーンは起動時に自動的に再キャリブレーションプロセスを実行します。

**English**

Common fixes:

- use a USB data cable (not charge-only)
- try a different SD card brand/model
- tune Wi-Fi timeout and coldboot values in config
- delete `touch.json` to recalibrate touch

---

## 5インチ LCD / Bus 設定（JC8048W550・lcd.json） / 5-inch LCD/Bus Settings (JC8048W550, lcd.json)

**5インチ用ファームウェア（JC8048W550）のみ**です。RGB パネル / Bus_RGB の**個体差調整**は、SPIFFS の `**eeprom.bin`** の保存値と、SD カード**ルート**の `**lcd.json`** で行います。

- SD ルートに `**lcd.json`** を置くと、本体が読み取って保存し、**ファイルは消えたうえで一度再起動**します。**もう一度起動したあと**から反映されます。
- **Chrome 拡張の「Download filaments ZIP」** に同梱の `**resource/lcd_default.json`**（工場に近い設定）・`**resource/lcd_disable.json`**（細かい調整だけリセット、画面の向きは維持）を、SD ルートで `**lcd.json**` にリネームして使えます。

手順・JSON の書き方・注意点の詳細は **[docs/lcd-settings-5inch.md](docs/lcd-settings-5inch.md)** を参照してください。

**English**

This section applies only to 5-inch firmware (JC8048W550).
Use `lcd.json` at SD root to tune RGB panel/bus differences, then reboot.
See `docs/lcd-settings-5inch.md` for full details.

---

## 開発環境のセットアップ（ファームウェアのビルドを行う場合） / Development Environment Setup (for Firmware Builds)

詳細は[開発環境のセットアップ](docs/develop.md)を参照してください。

**English**

For local firmware builds, follow `docs/develop.md`.

## 重要なクレジット表示 / Required Credits

XPTouchは、xptouchを源流とした日本語カスタマイズバージョンです。
**本プロジェクトは以下のオープンソースプロジェクトから派生しています：**

**English**

XPTouch is a Japanese-customized derivative of xtouch.
Please keep the project credits and upstream references.

### プロジェクト名称 / Project Name

- **XPTouch** - 日本語コミュニティバージョン
- **開発者**: tac-lab.tech
- **ライセンス**: GPLv3

### オリジナルプロジェクト / Original Project

- **xtouch** - [https://github.com/xperiments-in/xtouch](https://github.com/xperiments-in/xtouch)
- **開発者**: Pedro Casaubon Aguilar
- **ライセンス**: GPLv3

## 法的免責事項 / Legal Disclaimer

- 本プロジェクトは教育・研究目的で開発されています
- Bambulab Cloudを利用したサービスのため、提供元の仕様変更などにより使用できなくなる可能性があります。
- このプロジェクトはXTOUCHの拡張を行い、ボード対応を追加していますが、すべての機能の動作を保証するものではありません。
- 参照元プロジェクトの制限によりXPtouchの商用利用は全面的に禁止します。
- オリジナルプロジェクトの開発者には問い合わせしないでください

**English**

- This project is for educational and research purposes.
- Cloud/API behavior may change and break compatibility.
- Not all boards/features are guaranteed.
- Commercial use of XPTouch is prohibited due to upstream constraints.

## References

画面がうまく表示ができない場合はボードを提供いただけると
対応するかも！
[LovyanGFX](https://github.com/tac5551/LovyanGFX)
[OpenBambuAPI](https://github.com/tac5551/OpenBambuAPI)
[ha-bambulab](https://github.com/tac5551/ha-bambulab)
[Bambu-Lab-Cloud-API](https://github.com/tac5551/Bambu-Lab-Cloud-API)
[BambuP1Streamer](https://github.com/slynn1324/BambuP1Streamer)

---

**注意**: 本プロジェクトは教育・研究目的で開発されています。