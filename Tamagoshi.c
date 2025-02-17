#include <stdio.h>
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"


#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 10000

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); 
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(pin, 0);
}

void beep(uint pin, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);

    pwm_set_gpio_level(pin, 2048);

    sleep_ms(duration_ms);

    pwm_set_gpio_level(pin, 0);
}


int main() {
    
    const uint BUTTON_PIN_A = 5;  
    const uint BUTTON_PIN_B = 6;
    const uint RED_LED_PIN  = 13;
    int status_alarme = 0;

    
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    char *text[] = {
        "  Alarme        ",
        "  Para          ",
        "  Bikes         "};

    int y = 0;
    for (uint i = 0; i < count_of(text); i++)
    {
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);


    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0);

    gpio_init(BUTTON_PIN_A);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);

    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_B);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    
    
    pwm_init_buzzer(BUZZER_PIN);

    while (true) {
        if (gpio_get(BUTTON_PIN_A) == 0) {  
            sleep_ms(200);  
            if (gpio_get(BUTTON_PIN_A) == 0) {  
                status_alarme = !status_alarme;  
                while (gpio_get(BUTTON_PIN_A) == 0);  
                sleep_ms(200);  
            }
        }

        if (status_alarme == 1) {
            if (gpio_get(BUTTON_PIN_B) == 0) {  // Aqui deverá ficar a vericação de alterações brusca no acelerômetro
                while (gpio_get(BUTTON_PIN_A) == 1){
                    beep(BUZZER_PIN, 500);
                    sleep_ms(10);
                    gpio_put(RED_LED_PIN, 1);
                    sleep_ms(100);
                    gpio_put(RED_LED_PIN, 0);
                    sleep_ms(100);
                } 
            }
        }
    }

    return 0;
}