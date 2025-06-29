#include "stm8s.h"
#include <stdint.h>

// Bit değişkenlerinin tanımı
bool bit0 = 0, bit1 = 0, bit2 = 0, bit3 = 0, bit4 = 0, bit5 = 0, bit6 = 0, bit7 = 0, bit8 = 0, bit9 = 0, bit10 = 0, bit11 = 0, bit12 = 0, bit13 = 0, bit14 = 0, bit15 = 0;

// 4x4 Matrix: 4 satır, 4 sütun
#define ROWS 4
#define COLS 4

// Satır pinleri: PD1, PD2, PD3, PD4
const uint8_t row_pins[ROWS] = {1, 2, 3, 4};
// Sütun pinleri: PC6, PC5, PC4, PC7
const uint8_t col_pins_port[COLS] = {1, 1, 1, 1}; // 0: GPIOA, 1: GPIOC
const uint8_t col_pins[COLS] = {6, 5, 4, 7};      // PC6, PC5, PC4, PC7

// Basılan butonun değeri (0-15, -1: basılmadı)
volatile int8_t button_value = -1;

// milisaniye gecikme fonksiyonu (yaklaşık)
void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        for (volatile uint32_t i = 0; i < 119; i++)
        {
            // Yaklaşık 1 ms gecikme (saat frekansına göre ayarlanabilir)
        }
    }
}

// Satırları çıkış, sütunları giriş olarak ayarla
void matrix_init(void)
{
    // Satır pinlerini çıkış yap (PD1, PD2, PD3, PD4)
    for (uint8_t i = 0; i < ROWS; i++)
    {
        GPIOD->DDR |= (1 << row_pins[i]); // Output
        GPIOD->CR1 |= (1 << row_pins[i]); // Push-pull
    }
    // Sütun pinlerini giriş ve pull-up yap (PC6, PC5, PC4, PC7)
    for (uint8_t i = 0; i < COLS; i++)
    {
        if (col_pins_port[i] == 0)
        { // GPIOA
            GPIOA->DDR &= ~(1 << col_pins[i]);
            GPIOA->CR1 |= (1 << col_pins[i]);
        }
        else
        { // GPIOC
            GPIOC->DDR &= ~(1 << col_pins[i]);
            GPIOC->CR1 |= (1 << col_pins[i]);
        }
    }
}

// 4x4 matris buton tarama fonksiyonu
int8_t scan_matrix(void)
{
    // Satır pinlerinin maskesini oluştur
    uint8_t row_mask = 0;
    for (uint8_t i = 0; i < ROWS; i++)
    {
        row_mask |= (1 << row_pins[i]);
    }

    for (uint8_t row = 0; row < ROWS; row++)
    {
        // Tüm satır pinlerini HIGH yap
        GPIOD->ODR |= row_mask;
        // Sadece aktif satırı LOW yap
        GPIOD->ODR &= ~(1 << row_pins[row]);
        // Sütunları tara (PC6, PC5, PC4, PC7)
        for (uint8_t col = 0; col < COLS; col++)
        {
            uint8_t pin_val;
            if (col_pins_port[col] == 0)
                pin_val = GPIOA->IDR & (1 << col_pins[col]);
            else
                pin_val = GPIOC->IDR & (1 << col_pins[col]);
            if (!pin_val)
            {                            // Butona basıldıysa
                return row * COLS + col; // Butonun değeri (0-15)
            }
        }
    }
    return -1; // Hiçbir butona basılmadı
}

// UART1'i 1200 baudrate ile başlat
void uart1_init(void)
{
    // UART1_TX (PD5) çıkış olarak ayarlanır
    GPIOD->DDR |= (1 << 5);
    GPIOD->CR1 |= (1 << 5);

    UART1->BRR2 = 0x0B; // 1200 baud için BRR2 ve BRR1 ayarları (f_CPU=16MHz için)
    UART1->BRR1 = 0x68;
    UART1->CR2 = UART1_CR2_TEN; // Sadece TX aktif
}

// UART1 ile bir byte gönder
void uart1_send(uint8_t data)
{
    while (!(UART1->SR & UART1_SR_TXE))
        ; // TX buffer boşalana kadar bekle
    UART1->DR = data;
}

// Watchdog başlatma fonksiyonu (yaklaşık 2 saniye timeout)
void iwdg_init(void)
{
    IWDG->KR = 0xCC;  // Enable access to PR and RLR
    IWDG->KR = 0x55;  // Enable write access
    IWDG->PR = 0x06;  // Prescaler 256
    IWDG->RLR = 0xFF; // Reload value (max)
    IWDG->KR = 0xAA;  // Reload counter
    IWDG->KR = 0xCC;  // Start the watchdog
}

// Watchdog'u besle
void iwdg_refresh(void)
{
    IWDG->KR = 0xAA;
}

int main(void)
{
    // PA1, PA2, PA3 çıkış (ör: LED veya diğer çıkışlar)
    GPIOA->DDR |= (1 << 1) | (1 << 2) | (1 << 3);
    GPIOA->CR1 |= (1 << 1) | (1 << 2) | (1 << 3);
    GPIOA->ODR &= ~((1 << 1) | (1 << 2) | (1 << 3)); // Başlangıçta hepsi low
    GPIOA->ODR |= (1 << 3);                          // i̇lk açılışta manuel oldu belirtir

    matrix_init(); // Matris butonları için başlatma
    uart1_init();  // UART başlat
    iwdg_init();   // Watchdog başlat

    volatile uint8_t binaryData_1 = 0;
    volatile uint8_t binaryData_2 = 0;

    bool otomatik_aktif = 0;    // Otomatik On-Off için değişken
    bool makina_aktif = 0;      // Makina On-Off için değişken
    bool lamba_aktif = 0;       // Lamba On-Off için değişken
    bool otomatik_btn_flag = 0; // Otomatik buton flag'i
    bool makina_btn_flag = 0;   // Makina buton flag'i
    bool lamba_btn_flag = 0;    // Lamba buton flag'i

    while (1)
    {
        int8_t val = scan_matrix(); // Matris butonlarını tara

        binaryData_1 = 0;
        binaryData_1 |= (bit0 & 0x01);
        binaryData_1 |= (bit1 & 0x01) << 1;
        binaryData_1 |= (bit2 & 0x01) << 2;
        binaryData_1 |= (bit3 & 0x01) << 3;
        binaryData_1 |= (bit4 & 0x01) << 4;
        binaryData_1 |= (bit5 & 0x01) << 5;
        binaryData_1 |= (bit6 & 0x01) << 6;
        binaryData_1 |= (bit7 & 0x01) << 7;

        binaryData_2 = 0;
        binaryData_2 |= (bit8 & 0x01);
        binaryData_2 |= (bit9 & 0x01) << 1;
        binaryData_2 |= (bit10 & 0x01) << 2;
        binaryData_2 |= (bit11 & 0x01) << 3;
        binaryData_2 |= (bit12 & 0x01) << 4;
        binaryData_2 |= (bit13 & 0x01) << 5;
        binaryData_2 |= (bit14 & 0x01) << 6;
        binaryData_2 |= (bit15 & 0x01) << 7;

        if (val != -1)
        {
            button_value = val;
            GPIOA->ODR |= (1 << 1); // LED yak

            // uart1_send((uint8_t)button_value); // Buton değerini UART ile gönder

            switch (button_value)
            {
            case 0: // Söküm Yukarı
                bit4 = 1;
                break;
            case 8: // Söküm Aşağı
                bit3 = 1;
                break;
            case 2: // OK Sağ
                bit2 = 1;
                break;
            case 11: // OK Sol
                bit1 = 1;
                break;
            case 3:                      // Otomatik
                GPIOA->ODR &= ~(1 << 3); // PA3 pinini low yapar
                GPIOA->ODR |= (1 << 2);  // PA2 pinini high yapar
                bit0 = 1;
                break;
            case 1:                      // Manuel
                GPIOA->ODR &= ~(1 << 2); // PA2 pinini low yapar
                GPIOA->ODR |= (1 << 3);  // PA3 pinini high yapar
                bit0 = 0;
                break;
            case 15: // Makina Açık
                if (!makina_btn_flag)
                {
                    makina_aktif = !makina_aktif;
                    bit13 = !makina_aktif; // Makina aktifse bit13'ü 1 yap, pasifse 0 yap
                    makina_btn_flag = 1;
                }
                break;
            case 7: // Makina Ters
                bit15 = 1;
                break;
            case 12: // Hazırlayıcı Yukarı
                bit9 = 1;
                break;
            case 13: // Hazırlayıcı  Aşağı
                bit10 = 1;
                break;
            case 9: // Depo  Kapalı
                bit7 = 1;
                break;
            case 10: // Depo  Açık
                bit8 = 1;
                break;
            case 4: // Boşaltma çalıştır
                bit5 = 1;
                break;
            case 5: // Boşaltma Durdur
                bit5 = 0;
                bit6 = 1; // Boşaltma durdurulduğunda bit6'yı 1 yap
                break;
            case 6: // Flaş
                bit12 = 1;
                break;
            case 14: // Lamba
                if (!lamba_btn_flag)
                {
                    lamba_aktif = !lamba_aktif;
                    bit11 = !lamba_aktif;
                    lamba_btn_flag = 1;
                }
                break;
            }
        }
        else
        {
            makina_btn_flag = 0;     // Makina buton bırakıldıysa flag sıfırlanır
            lamba_btn_flag = 0;      // Depo buton bırakıldıysa flag sıfırlanır
            GPIOA->ODR &= ~(1 << 1); // LED söndür

            if (makina_aktif)
            {
                bit13 = 1; // Makina aktifse bit13'ü 1 yap
            }
            else
            {
                bit13 = 0; // Makina pasifse bit13'ü 0 yap
            }
            if (lamba_aktif)
            {
                bit11 = 1; // Lamba aktifse bit11'i 1 yap
            }
            else
            {
                bit11 = 0; // Lamba pasifse bit11'i 0 yap
            }

            bit1 = 0, bit2 = 0, bit3 = 0, bit4 = 0, bit6 = 0, bit7 = 0, bit8 = 0, bit9 = 0, bit10 = 0, bit12 = 0, bit14 = 0, bit15 = 0;
        }
        uart1_send((uint8_t)68);
        uart1_send((uint8_t)84);
        uart1_send((uint8_t)89);
        uart1_send((uint8_t)binaryData_1);
        uart1_send((uint8_t)binaryData_2);
        iwdg_refresh(); // Watchdog'u besle
        delay_ms(20);
    }
    return 0;
}
