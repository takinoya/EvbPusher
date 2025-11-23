[English](README.md) | [日本語](README.ja.md)
----
# SFX Breakout board \(土佐昌典版\)
\(TBD:Photo of Type-T\) <== Type-T \(TBD:Photo of Type-F\) <== Type-F

## Overview
### Concept
- 千円以下で売っていたりもするが、Size・I/F仕様が好みではないので作る。
- 土佐昌典VCによる一刀彫基板前提で素早く試作・実装する。
    - t=1.6mm
- 余り部材の消費
    - 秋月ターミナル青
    - ファストン端子
- 短絡事故防止のため、外枠シェルも作る。
- 古のPSU対応
    - Haswell時代以前の電源は、各系統に最小電力が消費されていないとおかしくなる。
        - 私の経験では鋸波がでてきた。
    - 対策用の最小電力補償抵抗回路を付与
        - Jumper pin で On/Off。

## Source Tree
- KiCad/SFX_BO_01/*
    - KiCad 基板設計データ
- Shell/*
    - 外枠ケースの 3D 設計・製造データ
    - アクリルパネルの設計・製造データ
- doc/*
    - README の 補助データ

### Design constrain
- 実装変更で 2type
    - Type-F : ファストン端子実装
    - Type-T : ターミナルブロック実装
- XH-3 : EvbPusher への制御線
    1. PS_ON#
    2. GND
    3. PWR_OK
- Force On switch

## Build up
### BOM
|Group|Item|Qty.|Maker::ProductID|Purchase link|Note|
|:---|:---|:---|:---|:---|:---|
|Type-F|Shell:Bottom-body-F|1|FDM 3D Print|\(TBD:Link\)|Bambu A1Mini使用|
|Type-T|Shell:Bottom-body-T|1|FDM 3D Print|\(TBD:Link\)|Bambu A1Mini使用|
|Common|Shell:Side-body|2|FDM 3D Print|\(TBD:Link\)|Bambu A1Mini使用|
|Type-F|Shell:Top-panel-F|1|Cutted acrylic|\(TBD:Link\)|trotex Speedy300使用|
|Type-T|Shell:Top-panel-T|1|Cutted acrylic|\(TBD:Link\)|trotex Speedy300使用|
|Common|Countersunk screw M3x6|4|\(Generic one\)|[千石電商 皿ねじ M3×6　BS皿 3×6](https://www.sengoku.co.jp/mod/sgk_cart/detail.php?code=EEHD-6DRL)|---|
|Common|Screw M3x6|4|\(Generic one\)|[千石電商 バインドねじ M3×6　BSバインド 3×6](https://www.sengoku.co.jp/mod/sgk_cart/detail.php?code=EEHD-6DNL)|---|
|Common|片面PCB t=1.6|1|TBD|---|CNC+土佐昌典 一刀彫|
|Common|ATX 24pin CN|1|Molex::0460152402 相当品|[千石電商 4201S-2x12](https://www.sengoku.co.jp/mod/sgk_cart/detail.php?code=EEHD-07A3)|千石のはMolexではなさそう|
|Common|XH 3pin 直立|1|JST::B3B-XH-A\(LF\)\(SN\)|[秋月電子 112248](https://akizukidenshi.com/catalog/g/g112248/)|---|
|Common|Alternate SW 8mm|1|Xiamen::PB-22E85L\(RED\)|[秋月電子 115965](https://akizukidenshi.com/catalog/g/g115965/)|---|
|Common|3mm LED 紫 IR=20mA|1|GB::LLED-P301|[マルツ M0006-2093](https://www.marutsu.co.jp/pc/i/62093/?srsltid=AfmBOooJ0V0tibKL59ehwpArz0U68qGsXHXkMuBlFWteiit1GA-oAnPT)|---|
|Common|3mm LED 緑 IR=1mA|1|OptoSupply::OSG58A3133A-1MA|[秋月電子 116906](https://akizukidenshi.com/catalog/g/g116906/)|---|
|Common|3mm LED 赤 IR=1mA|1|OptoSupply::OSR5PA5A33A-1MA|[秋月電子 116914](https://akizukidenshi.com/catalog/g/g116914/)|---|
|Type-F|ファストン端子基板用\#250 オス|12|康揚企業股份有限公司::PC250|[秋月電子 105119](https://akizukidenshi.com/catalog/g/g105119/)|---|
|Type-T|ターミナルブロック 3P 青 縦 小|4|Alphaplus::TB111-2-3-U-1-1|[秋月電子 101307](https://akizukidenshi.com/catalog/g/g101307/)|---|

### PCB build
TBD

### Circuit assemble
TBD

### 3D print
TBD

### Acrylic cut
TBD

### Shell Assembly
TBD

----

## Appendix
### Early consideration memo
- \[Assumed\] 基板はなるべく小さく・・・
    - 秋月C サイズ : 72x47 t=1.6
    - 片面基板で土佐昌典ビルド (試作 x2 でやめる予定)
- \[Assumed\] 上面 Force-On SW
    - 秋月 Alternate SW の 8mm角でやってみる
- \[Assumed\] 上面 Status LED <== SW 近傍配置
    - 5VSB : 黄/紫 (AC Power indicator)
        - 5VSBのケーブルカラーコードが紫なので合わせてみる。
        - 紫 LLED-P301 の場合
            - 330ohm for IR 15mA (Std:20mA)
        - 黄 の場合
            - 47kohm for IR 1mA
        - 下記 PWR_OK の配線もあるので JP で SW傍まで延ばす
    - PWR_OK : 緑 (Power ready)
        - 200uAしか流せないっぽいのでFET反転駆動回路必要 ==> INK021ABS1-T112 (Nch MOSFET) で
            - 電流源は 5VSB <== 0ohm 配線
    - +3.3V : 赤 (PowerOn)  <== SW傍配置
        - 33kohm for IR 1mA
- \[Assumed\] 0A非対応電源サポート：ダミー電流
    - 使用予定の KRPW-SXP600W/90+ では不要(0A対応電源)
    - 古の電源は +5V と +3.3V に 120mA 程度流さないと、鋸波とかでたりする・・・ので対策回路実験用に抵抗を実装しておく
    - 各系統 500mW 消費基準で、抵抗は 1W 品
        - 3.3V line 150mA : 22Ω 1W~ 縦置き
        - 5.0V line 120mA : 39Ω 1W~ 縦置き
        - 12.0V line 50mA : 240Ω 1W~ 縦置き
        - On/Off Jumper post 2.54mm pitch
            - 端子台脇に配置で配線を容易に
- \[Assumed\] 制御基板向け XH 3pin 直立
    1. PS_ON#
    2. GND
    3. PWR_OK
- \[Assumed\] 3P端子台 x 4
    - Port1(前面 \#2pin 近辺)
        1. 3.3V
        2. GND
        3. 5V
    - Port2(前面 \#10pin 近辺)
        1. 5VSB
        2. GND
        3. 12V
    - Port3(奥面 \#14pin 近辺)
        1. 3.3V
        2. GND
        3. -12V
    - Port4(奥面 \#22pin 近辺)
        1. 5V
        2. GND
        3. 12V
- \[Assumed\] ファストン端子#250 x 12
    - 上記 3P端子台 と排他実装。(スペース厳しい・・・)
    - 根元をポスカで塗っておく:
        - GND : 黒
        - 3.3V : 橙
        - 5V : 赤
        - 12V : 黄
        - 5VSB : 紫

### ToDo
- Footprint
    - [ ] Faston#250
    - [ ] ターミナル青
    - [ ] Switch
    - [ ] 抵抗Horizontal長さ調整
        - [ ] NORTH_PWR_OK : ```expr 4.2*7 + 2.54``` = 31.94
        - [ ] NORTH_PS_ON  : ```expr 4.2*3 + 2.54``` = 15.14
            - 15.24 でいいかも
