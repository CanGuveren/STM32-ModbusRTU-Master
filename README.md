# How to Use This Library

**1.** Download file and include the source file in Core/Src/ModbusRTU_Master.c and Core/Inc/ModbusRTU_Master.h header file in your project.
**2.** Set UART peripheral settings in your project. 

:exclamation: Attention. This project uses USART2 peripheral. If you want to different UART peripheral then you should change the value of UART_HandleTypeDef in the ModbusRTU_Master.c source file.

![image](https://user-images.githubusercontent.com/64977597/155136571-725a783a-b87b-41aa-a6a8-f7af2c698c48.png)



**3.** Set number of register, number of coil and slave id in the slave device.

![image](https://user-images.githubusercontent.com/64977597/155137609-518863a6-ff9a-4240-b51a-944c7636bd23.png)


**3.** You can use the library after making all the above settings. You can see the value of slave device register in the Modbus_Register[] array and you can see the value of slave device coil in the Modbus_Coil[] array. This arrays is in the ModbusRTU_Master.c source file.

![image](https://user-images.githubusercontent.com/64977597/155137841-c1aec211-3c1f-4c65-b783-ef37dc45d929.png)

## Running the Library

- You can use **Modbus Slave** program to test the library. You can download the **[Modbus Slave](https://www.modbustools.com/download.html)** program from here.
- Open the Modbus Slave program and connect to your device (Connection/Connect)
- :exclamation:Pay attention to your serial settings. The serial settings of the modbus slave program and the settings of the uart peripheral must be the same. 

![image](https://user-images.githubusercontent.com/64977597/155140512-c5953311-b0f8-4cab-b72b-25b12425e3e4.png)

- After this step you can use library functions. 

You can find the **MODBUS CRC** function used in this project [here](https://github.com/LacobusVentura/MODBUS-CRC16).  

Modbus Slav Library [here](https://github.com/CanGuveren/STM32-ModbusRTU-Slave).
