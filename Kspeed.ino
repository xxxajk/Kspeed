/*
 *      Find your CPU speed program.
 *
 *              Dynamically acquires the CPU core speed by monitoring the
 *              CLK_OUT signal on Freescale Kinetis microcontrollers.
 *
 *              If your uC lacks the pin or the RTC, this program will not work.
 *              Requires 32.768MHz crystal connected to to the RTC crystal pins.
 *
 *              This code also assumes that the RTC crystal has been engaged.
 *
 *              Caveats:
 *                      No attempt is made to check, however it is easy to do.
 *                      Just watch to see if you get a HIGH level on the pin.
 *
 *                      Results are sent to UART0 (Serial1 on Teensy 3.x)
 *                      because lower speeds can't use the USB module.
 *
 */

// CLK_OUT pin on all uC with RTC is on PORTE, Bit 0
// ** Note1: This is pin 31 on Teensy 3.x
// ** Note2: I don't use the "pin": definitions in order to be portable
#define RTC_CLKOUT_CFG PORTE_PCR0
#define RTC_CLK_PORT GPIOE_PDOR
#define RTC_CLK_BIT 0

// Define the bit location in the bitband.
#define RTC_BIT (*((((volatile uint8_t *)((((uint32_t)&(RTC_CLK_PORT) - 0x40000000) * 32 + (RTC_CLK_BIT) * 4 + 0x42000000)))) + 512))



#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <kinetis.h>
#endif

extern "C" {

        uint32_t cpu_speed(void) {
                register uint32_t loopsoff = 0;
                register uint32_t waste;
                __asm__ volatile ("" :: :);
                cli();

                while(RTC_BIT);
                while(!RTC_BIT);

                while(RTC_BIT) loopsoff++;
                sei();
                // compensate for 2MHz. This is the lowest I am willing to go.
                if(loopsoff < 142858) loopsoff = 142858;
                waste = ((loopsoff * 14LU) / 1000000LU);
                if(waste & 3) {
                        // Compensate for pipeline/cache effects.
                        waste = (((loopsoff * 12.1) / 1000000LU) * 1000000LU);
                }
                return (waste * 1000000LU);
        }
}

void setup() {
        // Set RTC_CLKOUT to generate 1Hz
        RTC_CLKOUT_CFG = (RTC_CLKOUT_CFG & 0xFFFFF8FFUL) | 0x00000700UL;
        SIM_SOPT2 = (SIM_SOPT2 & (~(SIM_SOPT2_CLKOUTSEL(7) | SIM_SOPT2_RTCCLKOUTSEL))) | SIM_SOPT2_CLKOUTSEL(5);
        Serial1.begin(115200);
        Serial1.println("");
}

void loop() {
        Serial1.print(cpu_speed() / 1000000LU);
        Serial1.println("MHz");
        Serial1.flush();
        delay(100); // wait a little
}

