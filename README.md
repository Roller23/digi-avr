# ATmega328p Emulator

An ATmega328p (commonly found in Arduino devices) written in C. Some parts depend on `avra` and `avr-gcc` toolchains in order to work.

Supports most of the MCU's functionality - it's able to decode and execute all AVR instructions, it can handle interrupts, supports sleep mode.

API makes it easily embeddable (as a shared library or just by including the source code)

Contains two GUI apps - one that runs in terminal and one that runs in a browser (requires compiling to shared library and setting up a Python server)

![image](https://user-images.githubusercontent.com/25384028/114919743-ea32a480-9e28-11eb-91af-134661448185.png)
![image](https://user-images.githubusercontent.com/25384028/114920157-6dec9100-9e29-11eb-9951-b1207a91f67e.png)
![image](https://user-images.githubusercontent.com/25384028/114919800-fdde0b00-9e28-11eb-951d-eb61a532789a.png)

TODO:

- Write tests for the rest of the instructions
- A just-in-time recompiler to speed things up maybe?
