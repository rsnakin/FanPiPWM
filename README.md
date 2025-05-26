# FanPiPWM

This is a C project designed to control a fan on Raspberry Pi and Orange Pi (only Orange Pi 3 LTS). It installs as a systemd service and provides a monitoring utility `fanmon`.

---

## 📦 Installation

### 🔧 Build and install

```bash
git clone https://github.com/rsnakin/FanPiPWM.git
cd FanPiPWM
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
cd /usr/local/etc
sudo cp fanpwmd.ini.default fanpwmd.ini
sudo nano fanpwmd.ini
```

### 🗂 The following components will be installed:

| File              | Installation path          | Purpose                                 |
|-------------------|----------------------------|-----------------------------------------|
| `fanpwmd`         | `/usr/local/bin/`          | Fan control daemon                      |
| `fanmon`          | `/usr/local/bin/`          | Fan and temperature monitoring utility  |
| `fanpwmd.ini`     | `/usr/local/etc/`          | Configuration file                      |
| `fan_pwm.service` | `/etc/systemd/system/`     | systemd service to autostart the daemon |

---

## 🔌 Running as a systemd service

```bash
sudo systemctl daemon-reload
sudo systemctl enable fan_pwm.service
sudo systemctl start fan_pwm.service
```

Check status and monitor:

```bash
systemctl status fan_pwm.service
fanmon
```
To exit fanmon, press 'q'

Stop the service:

```bash
sudo systemctl stop fan_pwm.service
```

---

## ⚙️ Configuration

File: `/usr/local/etc/fanpwmd.ini`

Example contents:

```ini
[hardware]
PIN = 1                          ; PIN number (see WiringPi/pins/pins.pdf or use `gpio readall` command)
PWM_RANGE_MAX = 100              ; PWM max range
PWM_RANGE_MIN = 75               ; PWM min range
HARDWARE_PWM = true              ; Use hardware PWM RPi PWM-pins: (GPIO12(PIN: 26), GPIO18(PIN 1), GPIO13(PIN 23), GPIO19(PIN 24))
HARDWARE_PWM_MODE = PWM_MODE_BAL ; PWM_MODE_MS || PWM_MODE_BAL (for Orange PI it does not work)
INIT_PWM_INTENSITY = 25          ; Initial rise in PWM intensity
HARDWARE_PWM_CLOCK = 200         ; value for pwmSetClock function (for Orange PI it does not work)

[values]
TEMPERATURE_MAX = 80             ; max temperature -> PWM range RANGE_MAX
TEMPERATURE_MIN = 55             ; min temperature -> PWM range RANGE_MIN
TEMPERATURE_RANGE = 15           ; temperature range

[files]
PATH_TO_TEMP_DATA = /sys/devices/virtual/thermal/thermal_zone0/temp
PATH_TO_LOG = /var/log/FanPiPWMld.log ; optional log file path
```

---

## 🧾 Project structure

```
FanPiPWM/
├── CMakeLists.txt
├── src/
│   ├── FanPiPWMld.cpp
│   ├── FanMon.cpp
│   ├── cfgini.cpp
│   ├── Log.cpp
│   ├── fanKbd.cpp
│   ├── fanPWM.cpp
│   ├── fanStrings.cpp
│   ├── fanTemp.cpp
│   └── shmMem.cpp
├── include/
│   ├── common.h
│   ├── Log.hpp
│   ├── cfgini.hpp
│   ├── fanKbd.hpp
│   ├── fanPWM.hpp
│   ├── fanStrings.hpp
│   ├── fanTemp.hpp
│   └── shmMem.hpp
├── config/
│   ├── fanpwmd.ini.default
│   └── platform_info.h.in
├── service/
│   └── fan_pwm.service.in
└── README.md
```

---

## 📚 Dependencies

- GCC or Clang
- CMake ≥ 3.18
- systemd
- [wiringPi](https://github.com/WiringPi) — for Raspberry Pi
- [wiringOP](https://github.com/orangepi-xunlong) — for Orange Pi

---

## 📄 License

The project is licensed under [MIT](LICENSE).

---

## 👤 Author

[RSnakin](https://github.com/rsnakin)
