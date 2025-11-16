# mazuno
Arduino uno random maze game
- random maze generator
- two kind of bonuces
  - :star: - need to pick up them all to unlock an exit
  - :hearts: - increate a step count by 3 
- [ ] exit when all :star: are collect
- :snowman: (step count) - limit of steps

# using 
- U8G2 library for SH1107 I2C display 128X128
- 2 position xy joystic
- buzzer

  

![image](https://github.com/user-attachments/assets/04ce9575-4d35-4b29-b32e-e3587c632e11)


   Start Game | Exit unlocked
:-------------------------:|:-------------------------:
![image](https://github.com/user-attachments/assets/cc7d3fed-8caf-4516-bf1d-1268fd7119c6) | ![image](https://github.com/user-attachments/assets/279201a9-d9a5-447a-9980-0fec78df80e1)



# wires

display |  | UNO   
--- | --- | --- 
SDA | <--> | A4   
SCL | <--> | A5
GND | <--> | GND
VCC | <--> | VCC



Joystic | | UNO   
--- | --- | --- 
VRY | <--> | A0
VRX | <--> | A1     
VCC | <--> | VCC
GND | <--> | GND


Buzzer | | UNO  
--- | --- | --- 
GND | <--> | GND
VCC | <--> | D2




[WOKWI project](https://wokwi.com/projects/422621312997012481)

![image](https://github.com/user-attachments/assets/abe99a57-3721-4b30-a1d8-b5887e2bc1e0)


# Libs

[Maze alghorithm](https://en.wikipedia.org/wiki/Maze_generation_algorithm#Simple_algorithms)

[U8G2](https://github.com/olikraus/u8g2)


# issues
## U8G2
- [Screen contrast](https://github.com/olikraus/u8g2/issues/501#issuecomment-366543758)

## UNO R3
- max ~13x11~ 15x13 blocks due a memory limit
