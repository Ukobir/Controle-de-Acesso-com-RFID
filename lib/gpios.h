//Funções para habilitar o led RGB e os botões


#ifndef GPIOS_H
#define GPIOS_H

#include "pico/stdlib.h"

void initBotao(int bot){
    gpio_init(bot);
    gpio_set_dir(bot, GPIO_IN);
    gpio_pull_up(bot);
}

void initLEDs(const uint8_t gpio[3])
{
    for (size_t i = 0; i < 3; i++)
    {
        gpio_init(gpio[i]);
        gpio_set_dir(gpio[i], GPIO_OUT);
        gpio_put(gpio[i], 0);
    }
}

#endif