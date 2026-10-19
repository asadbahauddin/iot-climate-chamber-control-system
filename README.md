# Sistem Kendali dan Monitoring Ruang Iklim Berbasis IoT

Prototipe *climate chamber* (ruang iklim) terkendali untuk penelitian, dengan arsitektur dua mikrokontroler:

- **STM32F446RE** — kendali real-time: pembacaan sensor (DHT22, ADC), aktuator heater, humidifier, dimmer AC, serta pengendalian AC via inframerah.
- **ESP32-S3** — gateway IoT: menerima data sensor dari STM32 melalui UART, mengirimkannya ke Supabase (REST), dan mengambil *setpoint* dari server.
- **Dashboard SCADA web** (Next.js + Supabase) — repositori terpisah: [ruang-iklim-scada](https://github.com/Asadganteng/ruang-iklim-scada).

## Struktur Repositori

| Path | Isi |
|---|---|
| `dokumentasi ruang ilim/Ruang_Iklim_Final_Sukses/` | Firmware STM32 (proyek STM32CubeIDE, `.ioc`, driver HAL) |
| `dokumentasi ruang ilim/Kode_ESP32S3_iklim/` | Firmware ESP32-S3 (Arduino) |
| `dokumentasi ruang ilim/*.txt` | Catatan dokumentasi fisik dan kendala |
| `2D_PCB1_*.png`, `pcb *.pdf` | Desain PCB (mikrokontroler dan sensor) |
| `Rangkaianpt1.eprj`, `rangkaian.fzz` | Skematik rangkaian (EasyEDA Pro, Fritzing) |

## Alur Data

```
Sensor -> STM32F446RE --UART--> ESP32-S3 --HTTPS--> Supabase <--> Dashboard SCADA
                ^                                      |
                +--------- setpoint (UART) <-----------+
```

## Kendala yang Masih Terbuka

- Pembacaan DHT22 tidak stabil (kadang gagal membaca).
- Dimmer AC terprogram masih berkedip.
- Kendali humidifier via optocoupler sudah berjalan di ESP32 (PWM), belum di STM32.
