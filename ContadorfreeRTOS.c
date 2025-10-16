/*
 *  Por: Leonardo Romão
 *  Data: 15-10-2025
 *
 *  Exemplo de uso dos três tipos de semaforo do FreeRTOS + Adição do sensor RFID MRFC522
 *
 *  Descrição:
 *  Simulação de um dispositivo de controle de vagas de uma biblioteca.
 * Ao apertar o botão A é execultada uma task com o semaforo de contagem
 * que incrementa o número de vagas oculpadas.
 * Ao apertar o botão B ocorre o mesmo mas decrementa o número de vagas.
 * Ao aproximar a tag registrada no leitor RFID é execultada uma task que
 * incrementa o número de vagas oculpadas. E ao aproximar novamente
 * decrementa o número de vagas.
 * Ao apertar o botão do joystick ocorre o reset de vagas oculpadas e
 * utiliza um semaforo binário garantindo sua execução sem concorrência.
 * A função responsável pelo display possui o semaforo mutex que impede
 * que a gravação no display enquanto é execultada termine sem que haja
 * interrupção para gravar novemente.
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "lib/ssd1306.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/bootrom.h"
#include "stdio.h"
#include "lib/buzina.h"
#include "lib/ws2812.h"
#include "lib/font.h"
#include "lib/gpios.h"
#include "mfrc522.h"

#define BUZZER_A 21  // Buzzer B
#define BUZZER_B 10  // Buzzer A
#define BOTAO_A 5    // Botao A
#define BOTAO_B 6    // Botao B
#define BOTAO_JOY 22 // Botao Joystick
#define MAX_PEOPLE 9 // Máximo de pessoas disponíveis

const uint8_t led_rgb[3] = {11, 12, 13}; // G B R

// Variáveis utilizada para o debouncing
static volatile uint32_t passado = 0;
uint32_t tempo_atual;

ssd1306_t ssd;
uint16_t contador = 0;
bool flag_card = 0;
bool flag_tag = 0;
SemaphoreHandle_t xContadorInc;
SemaphoreHandle_t xContadorDec;
SemaphoreHandle_t xDisplayMutex;
SemaphoreHandle_t xButtonReset;
SemaphoreHandle_t xIRQ_RFID;

// ISR do botão A (incrementa o semáforo de contagem)
void gpio_callback_inc(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xContadorInc, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ISR do botão B (decrementa o semáforo de contagem)
void gpio_callback_dec(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xContadorDec, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
// ISR do botão Joystick
void gpio_callback_reset(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xButtonReset, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ISR do sensor RFID
void gpio_callback_tag(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xIRQ_RFID, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Função que consome eventos e mostra no display e os Leds
// Utiliza um semaphoro do tipo binário
void vTaskDisplay()
{
    // Aguarda semáforo (um evento)
    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
    {
        // Atualiza display com a nova contagem
        frame(&ssd, contador);
        desenhaMatriz(numbers[MAX_PEOPLE - contador]);
        xSemaphoreGive(xDisplayMutex);
        switch (contador)
        {
            // Caso tenha todas as vagas = Azul
        case 0:
            gpio_put(led_rgb[0], 0);
            gpio_put(led_rgb[1], 1);
            gpio_put(led_rgb[2], 0);
            break;
            // Caso reste 1 vaga = Amarelo
        case 8:
            gpio_put(led_rgb[0], 1);
            gpio_put(led_rgb[1], 0);
            gpio_put(led_rgb[2], 1);
            break;
            // Caso não reste vaga = Vermelho
        case 9:
            gpio_put(led_rgb[0], 0);
            gpio_put(led_rgb[1], 0);
            gpio_put(led_rgb[2], 1);
            break;
        default: // As demais verde
            gpio_put(led_rgb[0], 1);
            gpio_put(led_rgb[1], 0);
            gpio_put(led_rgb[2], 0);
            break;
        }
        // Pisca a matriz de led caso estaja cheio para alertar
        if (contador == 9)
        {
            sleep_ms(300);
            npClear();
            npWrite();
            sleep_ms(300);
            desenhaMatriz(numbers[MAX_PEOPLE - contador]);
        }
    }
}

// Task de incremento do valor do contador com um semaphoro do tipo count
void vTaskEntrada(void *params)
{
    while (true)
    {
        // Aguarda semáforo (um evento)
        if (xSemaphoreTake(xContadorInc, portMAX_DELAY) == pdTRUE)
        {

            if (contador < 9)
            {
                contador++;
                vTaskDisplay();
            }

            if (contador == 9)
            {
                corneta();
            }
            else
            {
                beep(BUZZER_B, 25e3);
                beep(BUZZER_A, 20e3);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        semSom();
    }
}

// Task de decremento do valor do contador com um semaphoro do tipo count
void vTaskSaida(void *params)
{
    while (true)
    {
        // Aguarda semáforo (um evento)
        if (xSemaphoreTake(xContadorDec, portMAX_DELAY) == pdTRUE)
        {
            if (contador > 0)
            {
                contador--;
                vTaskDisplay();
            }
            beep(BUZZER_B, 20e3);
            beep(BUZZER_A, 25e3);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        semSom();
    }
}

// Task de reset do valor do contador com um semaphoro do tipo mutex
void vTaskReset(void *params)
{
    while (true)
    {

        // Aguarda semáforo (um evento)
        if (xSemaphoreTake(xButtonReset, portMAX_DELAY) == pdTRUE)
        {
            contador = 0;
            flag_card = 0;
            flag_tag = 0;
            vTaskDisplay();
            // Beep duplo
            beep(BUZZER_A, 30e3);
            beep(BUZZER_B, 30e3);
            vTaskDelay(pdMS_TO_TICKS(150));
            semSom();
            vTaskDelay(pdMS_TO_TICKS(300));
            beep(BUZZER_B, 15e3);
            beep(BUZZER_A, 15e3);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        semSom();
    }
}
// Task para o RFID
void vTaskRFID(void *params)
{
    // Inicializa MFRC522
    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);
    PCD_AntennaOn(mfrc); // Liga antena
    vTaskDelay(pdMS_TO_TICKS(500));
    char uid_str[24]; // Buffer para UID formatado

    while (true)
    {
        while (!PICC_IsNewCardPresent(mfrc))
        {
            sleep_ms(500);
        }

        printf("Card detected! Trying to read UID...\n");
        if (PICC_ReadCardSerial(mfrc))
        {
            beep(BUZZER_B, 25e3);
            beep(BUZZER_A, 20e3);
            // Formata UID em string "XX XX XX XX ..."
            int offset = 0;
            for (int i = 0; i < mfrc->uid.size; i++)
            {
                offset += sprintf(&uid_str[offset], "%02X ", mfrc->uid.uidByte[i]);
            }
            printf("UID: %s\n", uid_str);
            // Verifica UID para acionar LEDs
            if (mfrc->uid.size == 4) // Certifica-se que tem 4 bytes no UID
            {
                // Cartão UID: 90 93 18 43  liga LED azul GPIO12
                if (mfrc->uid.uidByte[0] == 0x90 &&
                    mfrc->uid.uidByte[1] == 0x93 &&
                    mfrc->uid.uidByte[2] == 0x18 &&
                    mfrc->uid.uidByte[3] == 0x43)
                {
                    if (!flag_card)
                    {
                        if (contador == 9)
                        {
                            corneta();
                        }
                        else
                        {
                            contador++;
                            vTaskDisplay();
                            flag_card = 1;
                        }
                    }
                    else
                    {
                        contador--;
                        vTaskDisplay();
                        flag_card = 0;
                    }
                }
                // Cartão UID: 43 76 BB 04 liga LED verde GPIO13
                else if (mfrc->uid.uidByte[0] == 0x43 &&
                         mfrc->uid.uidByte[1] == 0x76 &&
                         mfrc->uid.uidByte[2] == 0xBB &&
                         mfrc->uid.uidByte[3] == 0x04)
                {
                    if (!flag_tag)
                    {
                        if (contador == 9)
                        {
                            corneta();
                        }
                        else
                        {
                            contador++;
                            vTaskDisplay();
                            flag_tag = 1;
                        }
                    }
                    else
                    {
                        contador--;
                        vTaskDisplay();
                        flag_tag = 0;
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(300));
            semSom();
        }
    }
}

// ISR para BOOTSEL e botão de evento
void gpio_irq_handler(uint gpio, uint32_t events)
{
    tempo_atual = to_ms_since_boot(get_absolute_time());

    if (tempo_atual - passado > 200) // atualiza a cada 10ms
    {
        passado = tempo_atual;
        if (gpio == BOTAO_B)
        {
            gpio_callback_dec(gpio, events);
        }
        else if (gpio == BOTAO_A)
        {
            gpio_callback_inc(gpio, events);
        }
        else if (gpio == BOTAO_JOY)
        {
            gpio_callback_reset(gpio, events);
        }
    }
}

int main()
{
    stdio_init_all();

    // Função para iniciar o display
    initDisplay(&ssd);
    frame(&ssd, 0); // Inicia o display com todas as vagas disponíveis 9/9

    // Configura os botões
    initBotao(BOTAO_A);
    initBotao(BOTAO_B);
    initBotao(BOTAO_JOY);

    // Configura os leds RGB
    initLEDs(led_rgb);
    gpio_put(led_rgb[1], 1); // Inicia com contador 0 = Azul

    // Configuração das IRQ
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BOTAO_JOY, GPIO_IRQ_EDGE_FALL, true);

    // Inicia e configura os Buzzers
    initPwm();

    // Inicia e Configura a matriz de led
    npInit();
    npClear();
    desenhaMatriz(numbers[MAX_PEOPLE - contador]); // Liga a matriz de LED com todas os valores disponíveis

    // Cria semáforo de contagem (máximo 10, inicial 0)
    xContadorInc = xSemaphoreCreateCounting(5, 0);
    xContadorDec = xSemaphoreCreateCounting(5, 0);
    xDisplayMutex = xSemaphoreCreateMutex();
    xButtonReset = xSemaphoreCreateBinary();

    // Cria tarefa
    xTaskCreate(vTaskEntrada, "Task de incremento", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Task de decremento", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Task de reset", configMINIMAL_STACK_SIZE + 128, NULL, 2, NULL);
    xTaskCreate(vTaskRFID, "Task do sensor", configMINIMAL_STACK_SIZE + 128, NULL, 5, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
