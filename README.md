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
PIN = 1
PWM_RANGE_MAX = 100
PWM_RANGE_MIN = 75
HARDWARE_PWM = true
HARDWARE_PWM_MODE = PWM_MODE_BAL
INIT_PWM_INTENSITY = 25
HARDWARE_PWM_CLOCK = 200

[values]
TEMPERATURE_MAX = 80
TEMPERATURE_MIN = 55
TEMPERATURE_RANGE = 15

[files]
PATH_TO_TEMP_DATA = /sys/devices/virtual/thermal/thermal_zone0/temp
PATH_TO_LOG = /var/log/FanPiPWMld.log
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

- GCC или Clang
- CMake ≥ 3.10
- systemd
- [wiringPi](https://github.com/WiringPi/WiringPi.git) — for Raspberry Pi
- [wiringOP](https://github.com/orangepi-xunlong/wiringOP.git) — for Orange Pi

---

## 📄 License

The project is licensed under [MIT](LICENSE).

---

## 👤 Author

[RSnakin](https://github.com/rsnakin)
