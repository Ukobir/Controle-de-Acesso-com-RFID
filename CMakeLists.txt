



cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "C:/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(PiscaLed C CXX ASM)
pico_sdk_init()


include_directories(${CMAKE_SOURCE_DIR}/lib)


add_executable(${PROJECT_NAME}  
        ContadorfreeRTOS.c 
        lib/ssd1306.c # Biblioteca para o display OLED
        lib/mfrc522.c
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib 
        hardware_gpio
        hardware_spi
        hardware_i2c
        hardware_adc
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        hardware_pwm
        hardware_pio
        )


#Converte o .pio para .h
pico_generate_pio_header(${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}/ws2818b.pio)

target_sources(${PROJECT_NAME} PRIVATE ContadorfreeRTOS.c)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})




