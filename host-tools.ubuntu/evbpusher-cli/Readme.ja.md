# evbpusher-srv/evbpusher-cmd

## Overview
### Background
- Arduino Nano Every の tty を Open すると Reset が入ってしまう。
    - Classic Nano では外部コンデンサで回避可能だったが、Every では無理っぽい。
- Kernel module 弄って Open時に DTR を Dont care にするとか・・・汎用性がなくなる
- ということで、tty を開きっぱなしの Server に中継させることとした
    - Server/Client間は UNIX Domain Socket で接続。

## Setup EvbPusher service
deb でインストール後、ちょいと設定が必要。

### Build deb package
```
$ fakeroot dpkg-deb --build debian .
```

### Install deb package
Like following:
```
$ sudo apt install evbpusher-cli_x.y.z_amd64.deb
```

### Setup UDEV rules to USB iSerial of Arduino device
1. EvbPusher を USB に挿す
2. dmesg 等で tty device path を確認する
    ```
    $ sudo dmesg|tail
    [1538738.098774] usb 3-2: New USB device found, idVendor=2341, idProduct=0074, bcdDevice= 1.00
    [1538738.098788] usb 3-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
    [1538738.098792] usb 3-2: Product: Nano R4
    [1538738.098795] usb 3-2: Manufacturer: Arduino
    [1538738.098799] usb 3-2: SerialNumber: 36030E293632363267AF33334B572F52
    [1538738.101250] cdc_acm 3-2:1.0: ttyACM0: USB ACM device
    ```
3. udevadm で iSerial\(ID_SERIAL_SHORT\) を確認する
    ```
    $ udevadm info -q all -n /dev/ttyACM0 | grep -P '(ID_SERIAL_SHORT|ID_VENDOR_ID|ID_MODEL_ID)'
    E: ID_VENDOR_ID=2341
    E: ID_MODEL_ID=0074
    E: ID_SERIAL_SHORT=36030E293632363267AF33334B572F52
    ```
4. /etc/udev/rules.d/99-EvbPusher-usb-serial.rules を編集する
    - ID_SERIAL_SHOT が xxxx になっているので、接続したものに書き換える。
5. udev rule 再読み込み
    ```
    $ sudo udevadm control --reload-rules
    ```
6. EvbPusher を一度抜き差しし、/dev/ttyEvbPusher が現れればOK。


### Enable and start EvbPusher service
```
$ sudo systemctl enable evbpusher.service
$ sudo systemctl restart evbpusher.service
```

## Usual command Usage
### SYNOPSIS
```
evbpusher-cmd \<strings to EvbPusher\>
```

### Description
TBD

## Development note
- 実体は Python3 の Script
- Arduino の Serial port アクセスのため、python3-serial に依存関係。
- Server/Client 共に UDS の path を変えれるようにしてある。
    - 複数の EvbPusher を扱う場合の布石。

## ToDo
- \[ \] USB iSerial 設定のヘルパーScript
- \[ \] deb package の pre/post script 整備
- \[ \] Makefile の作成
