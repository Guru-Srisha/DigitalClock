# DigitalClock

An ESP8266-based clock project using an RTC DS3231 and OLED display. Ever faced that the RTC gets reset due to power fluctuations? It's pain to flash the code again into the ESP board. This project with some 'personality' overcomes this. It automatically fetches and updates time from a cloud server when connected to Wi-Fi and allow the user to update the time from the cloud on their demand by just clicking a button. So there's no need to flash the code again to set the time, neither there is a need to keep your WiFi connection running, since when there's no WiFi connection available, the clock runs on the RTC.

# Components used:
1. ESP8266 development board
2. I2C OLED display
3. RTC DS3231
4. Push button
