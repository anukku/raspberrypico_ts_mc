#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/adc.h"
#include "stdbool.h"

void core1_interrupt_handler(){
    while(multicore_fifo_rvalid){
        uint16_t raw = multicore_fifo_pop_blocking();
        const float conversion_factor = 3.3f / (1<<12);
        float result = raw * conversion_factor;
        float temp = 27 - (result - 0.706)/0.001721;
        printf("Temp = %f C\n", temp);
        if(temp >= 23.5){
             gpio_put(25, 1);
        }
        else gpio_put(25, 0);
        
    }
    multicore_fifo_clear_irq();
}

void core1_entry(){
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);

    while(1){
        tight_loop_contents();
    }
}

int main(void) {
    stdio_init_all();
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    multicore_launch_core1(core1_entry);

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while(1){
        uint16_t raw = adc_read();
        multicore_fifo_push_blocking(raw);
        sleep_ms(1000);
    }

}