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

#define DIV1   0x00
#define DIV2   0x01
#define DIV4   0x02
#define DIV8   0x03
#define DIV16  0x04
#define DIV32  0x05
#define DIV64  0x06
#define DIV128 0x07

#define TDIV   DIV1

extern "C" {

        uint32_t cpu_speed(void) {
                register uint32_t loopsoff = 0;
                register uint32_t shifterB = 0;
                register uint32_t shifterC = 0;
                shifterB = 1 + ((SIM_CLKDIV1 >> 24) & 0x0f); // BUSS
                shifterC = 1 + ((SIM_CLKDIV1 >> 28) & 0x0f); // CORE
                // Calculate the buss speed
                FTM1_CNTIN = 0;
                FTM1_SC = TDIV | 0x08;
                cli();
                while(!RTC_BIT);
                while(RTC_BIT);
                FTM1_CNT = 0;
                while(!RTC_BIT);
                while(RTC_BIT);
                FTM1_SC = TDIV;
                sei();
                loopsoff = FTM1_CNT;
                Serial1.print(loopsoff);
                Serial1.println(" BUSS ticks (actual)");
                loopsoff += 6; // Add 6 ticks to compensate for low speed jitter
                if(loopsoff >= 1800) loopsoff = 1800; // 60MHz buss
                else if(loopsoff >= 1680) loopsoff = 1680; // 56MHz buss
                else if(loopsoff >= 1440) loopsoff = 1440; // 48MHz buss
                else if(loopsoff >= 1080) loopsoff = 1080; // 36MHz buss
                else if(loopsoff >= 960) loopsoff = 960; // 32MHz buss
                else if(loopsoff >= 720) loopsoff = 720; // 24MHz buss
                else if(loopsoff >= 480) loopsoff = 480; // 16MHz buss
                else if(loopsoff >= 240) loopsoff = 240; // 8MHz buss
                else if(loopsoff >= 120) loopsoff = 120; // 4MHz buss
                else if(loopsoff >= 60) loopsoff = 60; // 2MHz buss
                else loopsoff = 30; // 1MHz buss (or less!)

                Serial1.print(loopsoff);
                Serial1.println(" BUSS ticks (rounded)");
                loopsoff = loopsoff * shifterB;
                Serial1.print(loopsoff);
                Serial1.print(" PLL ticks, ");
                loopsoff = loopsoff / shifterC;
                Serial1.print(loopsoff);
                Serial1.print(" CORE ticks, ");
                loopsoff = loopsoff / 30;
                return (loopsoff * 1000000LU);
        }
}

void setup() {
        // Set RTC_CLKOUT to generate 1Hz
        RTC_CLKOUT_CFG = (RTC_CLKOUT_CFG & 0xFFFFF8FFUL) | 0x00000700UL;
        //SIM_SOPT2 = (SIM_SOPT2 & (~(SIM_SOPT2_CLKOUTSEL(7) | SIM_SOPT2_RTCCLKOUTSEL))) | SIM_SOPT2_CLKOUTSEL(5);
        SIM_SOPT2 = (SIM_SOPT2 & (~SIM_SOPT2_CLKOUTSEL(7))) | SIM_SOPT2_CLKOUTSEL(5) | SIM_SOPT2_RTCCLKOUTSEL;
        Serial1.begin(115200);
        Serial1.println("");
}

void loop() {
        Serial1.print(cpu_speed() / 1000000LU);
        Serial1.println("MHz");
        Serial1.flush();
        delay(100); // wait a little
}

