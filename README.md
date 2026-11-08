## alarm clock 

<img width="937" height="782" alt="image" src="https://github.com/user-attachments/assets/0de047fd-e9d7-49a5-8edb-5be881c37a43" />


it is a alarm clock and it has daily morning affirmations and manifestation words so i can read them when i woke up from the  screen and attract the life i always 


## why i made this

from very long time i wanted to integrate the routine of affirmations and manifestation as some of idols mentioned so i feel this is true , i wanted to try it so here i am doing it 

## what it does 
All day: The screen shows the time, today’s date, and one small affirmation, like “I am calm, capable, and ready.”
7:30 AM: The buzzer starts with soft little beeps that slowly get louder. If you want 5 more minutes, tap SW1. To stop it, hold SW1 for 2 seconds.
After you stop it: The screen shows “Good morning,” your affirmation, and your manifestation (“Today I choose calm over worry”). It stays for 30 seconds, then goes back to the clock.
9 PM: Two soft chimes play, and the screen shows an evening affirmation and manifestation, like “You did enough today.” Any button closes it.

## how it works

Time: When you power it on, the ESP32 connects to your WiFi and asks the internet for the exact time. It has no clock battery, so it needs WiFi each time it starts. If WiFi isn’t available, it asks you to set the time by hand.
Alarm: The code checks the time every second. When it matches your alarm time, it starts the buzzer and changes the screen.
Affirmations: The code has lists of sentences. Each day it picks the next one using the date, so the words change daily without any extra work.
Buttons: The four switches control the alarm and screens. SW1 snoozes or stops. SW2 (hold) sets the alarm time, and SW3 and SW4 change the numbers. SW3 on the main screen shows the next affirmation, and SW4 dims the screen for night.

## pcb 

schematic pcb 

<img width="1712" height="833" alt="image" src="https://github.com/user-attachments/assets/3e6dee80-98b9-4acd-a875-52527db82f53" />

pcb routing

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/1c12aa5c-266f-4319-b4a7-ccc94c7a1d6d" />

## flimware 

arduino ide 

## Bill of Materials

| Component    | Quantity | Description         | Source                       | Price |
| ------------ | -------: | ------------------- | ---------------------------- | ----: |
| ESP32-C3     |        1 | Microcontroller     | [link](https://robu.in/product/seeed-studio-xiao-esp32c3-tiny-mcu-board-with-wi-fi-and-ble-battery-charge-supported-power-efficiency-and-rich-interface/) | 749  |
| OLED Display |        1 | Small display       | [link](https://robocraze.com/products/0-96in-oled-display-module-4pin) |169 |
| 10K Resistor |        1 | Pull-up resistor    | [link](https://robocraze.com/products/10k-resistor-pack-of-10) |13|
| switches  |        3 | User input buttons  | [link](https://amzn.in/d/0aKbDkSN) | 359 |
| DHT11  |        1 | environment sensor  | [link](https://robu.in/product/serial-dht11-temperature-and-humidity-sensor/) | 289 |
| PCB          |        1 | Custom-designed PCB | [link](https://jlcpcb.com/)  | 1330|
| MPU6050 | 1 | 6-axis motion sensor module (GY-521) | [Link](https://robocraze.com/products/mpu-6050-triple-axis-accelerometer-gyroscope-module) | 151 |



