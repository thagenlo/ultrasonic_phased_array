// void app_main() {}


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <driver/i2s.h>
#include <math.h>

#define SAMPLE_RATE 160000 // Higher sample rate for better resolution
#define CARRIER_FREQ 40000 // 40 kHz carrier
#define BUFFER_SIZE 1024   // DMA buffer size

int16_t i2s_buffer[BUFFER_SIZE];

// Audio signal placeholder (e.g., sine wave for testing)
float get_audio_sample(int sample_index) {
    return sin(2.0 * M_PI * 1000.0 * sample_index / SAMPLE_RATE);  // Example 1 kHz sine wave
}

void generate_modulated_waveform() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        float audio_sample = get_audio_sample(i);  // Get the audio amplitude [-1,1]
        float carrier_sample = sin(2.0 * M_PI * CARRIER_FREQ * i / SAMPLE_RATE); // 40 kHz sine wave
        float modulated_signal = (1.0 + audio_sample) * carrier_sample;  // AM modulation

        // Convert to 16-bit signed integer
        i2s_buffer[i] = (int16_t)(modulated_signal * 32767);
    }
}

void setup_i2s() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // Transmit mode
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = 26,  // bit clock pin
        .ws_io_num = 25,   // word select pin (LRCLK)
        .data_out_num = 22, // data output pin
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM_0, SAMPLE_RATE);
}

void app_main(void) {
    printf("initializing I2S...");
    setup_i2s();
    while (1) {
        generate_modulated_waveform(); // generate AM waveform
        size_t bytes_written;
        i2s_write(I2S_NUM_0, i2s_buffer, sizeof(i2s_buffer), &bytes_written, portMAX_DELAY);
    }
}

