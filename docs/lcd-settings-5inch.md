# 5インチ画面の LCD 調整（`lcd.json`）

**JC8048W550 など 5インチ用ファームウェア**専用です。  
**2.8 インチ用ファームでは使えません**（無視されます）。

---

## 何ができるか

- 表示のチューニング用に、**SD カードのルート**に **`lcd.json`** という名前のファイルを置くと、本体が読み取って保存し、**ファイルは消えたうえで一度再起動**します。  
- **その次の起動から**、書き込んだ内容が反映されます（1 回目の起動で処理されるため）。

---

## 手順（共通）

1. 次の「サンプル」をコピーするか、自分で JSON を作る。  
2. ファイル名を必ず **`lcd.json`** にする。  
3. **SD カードのルート**（一番上の階層）に置く。  
4. 電源を入れる。  
5. しばらくすると **自動で再起動**する。`lcd.json` は消えている状態になります。  
6. **もう一度起動したあと**の表示を確認する。

---

## Chrome 拡張の ZIP に入っているサンプル

**xtouch28** で Bambu にログインしたあと、**「Download filaments ZIP」** で落とす ZIP の **`resource`** フォルダに、次があります。

| ファイル | 使いどころ |
|----------|------------|
| **`lcd_default.json`** | 工場出荷時に近い設定に戻したいとき。コピーして **`lcd.json`** にリネームして SD ルートへ。 |
| **`lcd_disable.json`** | LCD の細かい調整だけやめて、**画面の向き（反転）の設定はそのまま**にしたいとき。コピーして **`lcd.json`** にリネーム。 |

周波数だけ変えたい場合は、`lcd_default.json` を `lcd.json` にリネームしたあと、PC のエディタで数値を書き換えてから SD に置いてもかまいません。

---

## `lcd.json` に書ける項目（上級者向け）

名前は **そのまま JSON のキー**にします。

**周波数（Hz）**

- **`freq_write`** … 目安として **8000000～20000000**（8～20 MHz）。**`0`** にすると「工場の既定の周波数」扱いに戻ります。  
- 互換のため **`rgb_pclk_hz`** や **`pclk_hz`** も同じ意味で使えます。

**タイミング（表示が乱れるときの調整用）**

- `hsync_polarity`, `hsync_front_porch`, `hsync_pulse_width`, `hsync_back_porch`  
- `vsync_polarity`, `vsync_front_porch`, `vsync_pulse_width`, `vsync_back_porch`  
- `pclk_active_neg`, `de_idle_high`, `pclk_idle_high`  


**LCD 調整だけリセット**

- **`"disable": 1`** または **`"disable": true`** … 周波数・タイミングの保存内容だけクリアし、**画面の向きの設定は変えません**。他のキーより優先されます。

---

## 記述例

周波数だけ（14.5 MHz の例）:

```json
{ "freq_write": 14500000 }
```

工場に近い一式の例:

```json
{
  "freq_write": 14000000,
  "hsync_polarity": 1,
  "hsync_front_porch": 8,
  "hsync_pulse_width": 4,
  "hsync_back_porch": 8,
  "vsync_polarity": 1,
  "vsync_front_porch": 8,
  "vsync_pulse_width": 4,
  "vsync_back_porch": 8,
  "pclk_active_neg": 1,
  "de_idle_high": 1,
  "pclk_idle_high": 1
}
```

調整をやめる:

```json
{ "disable": 1 }
```

---

## うまくいかないとき・注意

- JSON の形が壊れていると、**ファイルは残ったまま**何も変わらないことがあります。内容を直して再度置いてください。  
- 周波数や数値が **許容範囲外**のときは受け付けられず、ファイルが残ることがあります。  
- 環境によっては、**周波数を変えても見た目があまり変わらない**ことがあります（ライブラリ・ボードの都合）。
