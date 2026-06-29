/** Written by Gera 4 Attiny13 v1.1 release june 24 2026
    TODO: to run on (ATtiny25/45/85)
    recommended changes to run on (ATtiny25/45/85)
      1. Register Name Changes
          Timer0 and External Interrupts are named differently on the ATtiny45:
          Interrupt Enable: Change TIMSK0 to TIMSK.
          External Interrupt Mask: Change GIMSK to GIMSK (This stays the same,
   as both use GIMSK for INT0). MCU Control: Change MCUCR (This stays the same,
   as both use MCUCR for ISC01/ISC00). Timer Control: TCCR0B is identical on
   both.
      2. Internal Clock: The ATtiny45 usually runs at 8.0 MHz internal, whereas
   the ATtiny13 runs at 9.6 MHz. Timing Math: tick counts must be recalculated
   for 8 MHz.
      3. Fuses:
          Low Fuse: 0xE2
          High Fuse: 0xDF
      4. I2C: "bit-banged" i2cmaster.h will work on the ATtiny45,
          though the ATtiny45 actually has more efficient hardware I2C.
    minimalistic changes:
        Change TIMSK0 to TIMSK, adjust F_CPU math for 8.0 MHz(~16% slower), and
   it should work (not tested)
 */

/** Useful info:
  Firmware size:
RAM:   [=         ]   7.8% (used 5 bytes from 64 bytes)
Flash: [======    ]  61.3% (used 628 bytes from 1024 bytes)

  Calculator si5351:
    https://rfzero.net/documentation/tools/si5351a-frequency-tool/

  PSX hardware:
    PS1 PU-8 EARLY GPU pinout https://psx-spx.consoledev.net/pinouts/#pinouts-gpu-pinouts-for-old-160-pin-gpu
        156-/HSYNC
        157-/VSYNC
    On early PU-8 boards, IC501 pin 7 must be lifted(INPUT) for NTSC and grounded(OUTPUT LOW) for PAL

  IC501 24pin "SONY CXA1645M" Analog RGB to Composite
    https://www.consolesunleashed.com/files/img/pinouts/sony-cxa1645m-rgb-encoder-890x1024.png
    On early PU-8 boards, IC501 pin 7 must be lifted(INPUT) for NTSC
    and puled to ground (OUTPUT LOW) for PAL

  PAL/NTSC frequencies:
    Feature             	  NTSC              PAL
    GPU Master Oscillator	  53.693175 MHz	    53.203425 MHz
    according to megavolt85: "PAL синхроинпульсы повторяются каждые 20 миллисекунд, а у NTSC каждые 16,6 миллисекунд"
    Pulse Intervals on GPU pin 157 measured via my arduino nano:
    PAL      Vsync interval (microseconds): 19884
    NTSC     Vsync interval (microseconds): 16772

  Attiny13 pinout:
            ,--_--,
    (PB5) 1|      |8  VCC (5V)
    (PB3) 2|      |7  PB2 (SCK/SCL/Analog 2)
    (PB4) 3|      |6  PB1 (MISO/PWM/Analog 1) (INT0)
      GND 4|      |5  PB0 (MOSI/PWM/SDA/Analog 0)
 */

#include "i2cmaster.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

// Si5351 Address: 0x60, shifted left and add write bit (0) = 0xC0
#define SI5351_I2C_ADDR 0xC0
// ATtiny13 default internal clock (1.2MHz)
#define F_CPU 9600000UL
// Debounce/Filter: Only accept if more than 5ms has passed
#define DEBOUNCE_TICKS 47 // ~5ms lockout period at 9.6MHz / 1024

/**-=[ PINOUT ]=-
                         ,--_--,
                 (PB5) 1|      |8 VCC -> (5V)
 LED/optional <- (PB3) 2|      |7 PB2 -> SDA of S1535
 IC501 pin7   <- (PB4) 3|      |6 PB1 -> pin157-VSYNC of PS1 GPU/IC203/CXD8514Q
                   GND 4|______|5 PB0 -> SCL of S1535

  Set up SCL/SDA pins as defined in your i2cmaster.S file
  (I made changes in i2cmaster.S)
    SDA:   PORTB PIN2    PB2
    SCL:   PORTB PIN0    PB0
  On early PU-8 boards, IC501 pin 7 must be lifted(INPUT) for NTSC
  and grounded(OUTPUT LOW) for PAL
    IC501 pin 7 connect to:      PORTB PIN4    PB4*/
#define PIN_IC501 PB4
#define PIN_IC203_GPU_VSYNC PB1 // ATtiny13 PB1 interrupt INT0
#define PIN_LED PB3             // optional LED for debugging purposes

typedef enum { UNKNOWN = 0, PAL = 1, NTSC = 2 } region;

volatile uint8_t measuredRegion = UNKNOWN;
volatile uint8_t si5351_Region = UNKNOWN;
volatile uint8_t timer_overflows = 0;
volatile uint8_t first_pulse = 1;

// Function to write a byte to a specific Si5351 register
void si5351_write(uint8_t reg, uint8_t data);

// Function set Si5351 CLK0 freq PAL:53203425 or NTSC:53693175
void si5351_setFrequency(uint8_t rgn);

// Function set timer, enable Timer Overflow Interrupt, enable global interrupt
void timer_setup();

// Function set interrupt INT0 on pin PB1 on Falling, enables global interrupts
void interrupt_setup();

// Function puling to ground IC501 pin 7 PAL and lift it for NTSC
void ic501_set_pin7(uint8_t rgn);

int main() {
  // -----------------------------=[ setup ]=-----------------------------------
  DDRB |= (1 << PIN_LED);              // set PIN_LED as output
  DDRB &= ~(1 << PIN_IC203_GPU_VSYNC); // Set PB1 as Input
  PORTB |= (1 << PIN_IC203_GPU_VSYNC); // Enable internal Pull-up

  i2c_init(); // Initialize the SCL/SDA pins as outputs
  _delay_ms(10);

  timer_setup();

  interrupt_setup();

  // according to megavolt85: on boot, the PS1 is always at the NTSC frequency
  // also verified it via my brand new oscilloscope
  si5351_setFrequency(NTSC);
  ic501_set_pin7(NTSC);

  // ------------------------------=[ loop ]=-----------------------------------
  uint8_t newRegion = UNKNOWN, prevRegion = UNKNOWN;
  while (true) {
    //cli(); //8-bit AVR reading a single byte is atomic. Don't need cli()/sei()
    newRegion = measuredRegion;
    //sei();

    if (newRegion == prevRegion &&
        newRegion != UNKNOWN) { // Act as a low-pass filter
                                //  (requiring at least two measurements)
                                //  to prevent flickering
      if (si5351_Region != newRegion) {
        cli();
        si5351_setFrequency(newRegion);
        ic501_set_pin7(newRegion);
        //_delay_ms(100);
        first_pulse = 1; // ensures that the very first pulse after the Si5351
                         // reconfigures is ignored
        PINB = (1 << PIN_LED); // TODO remove debugging LED
        sei();
      }
    }
    prevRegion = newRegion;
  }
  return 0;
}

void si5351_write(uint8_t reg, uint8_t data) {
  i2c_start_wait(SI5351_I2C_ADDR); // Start I2C, send 0xC0, wait for ACK
  i2c_write(reg);                  // Send register address
  i2c_write(data);                 // Send the data byte
  i2c_stop();                      // Release the bus
}

void si5351_setFrequency(uint8_t rgn) {
  // 1. Disable all outputs while configuring
  si5351_write(3, 0xFF);

  // 2. Set Crystal Load Capacitance (Register 183) Used to calibrate freq
  // Even though it is an 8-bit register, only the top two bits are used
  // The Full "Range" of Register 183
  // Value (Hex)	Value (Binary)	Capacitance	Effect on Clock
  // 0x40	        0100 0000	      6 pF	Speeds up the clock
  // 0x80	        1000 0000	      8 pF	Medium (factory default)
  // 0xC0	        1100 0000	      10 pF	Slows down the clock
  // si5351_write(183, 0x80);
  // I'm not using the feature due to a lack of professional frequency measuring equipment.

  // 3. Write PLL Parameters (Example for PLLA)
  // "Magick numbers" DATA got from Si5351A frequency online tool
  i2c_start_wait(SI5351_I2C_ADDR);
  i2c_write(26);     // Start at Reg 26
  i2c_write(0xFF);   // Reg 26
  i2c_write(0xFF);   // Reg 27
  i2c_write(0x00);   // Reg 28
  i2c_write(0x0A);   // Reg 29
  if (rgn == PAL) {  // PAL
    i2c_write(0xC4); // Reg 30
    i2c_write(0xFD); // Reg 31
    i2c_write(0x18); // Reg 32
    i2c_write(0x44); // Reg 33
    si5351_Region = PAL;
  } else if (rgn == NTSC) { // NTSC
    i2c_write(0xE2);        // Reg 30
    i2c_write(0xFE);        // Reg 31
    i2c_write(0x89);        // Reg 32
    i2c_write(0xE2);        // Reg 33
    si5351_Region = NTSC;
  }
  i2c_stop();

  // 4. Write Multisynth Parameters for CLK0
  // Magick DATA numbers from Si5351A frequency online tool
  i2c_start_wait(SI5351_I2C_ADDR);
  i2c_write(42);   // Start at Reg 42
  i2c_write(0x00); // Reg 42
  i2c_write(0x01); // Reg 43
  i2c_write(0x00); // Reg 44
  i2c_write(0x04); // Reg 45
  i2c_write(0x00); // Reg 46
  i2c_write(0x00); // Reg 47
  i2c_write(0x00); // Reg 48
  i2c_write(0x00); // Reg 49
  i2c_stop();

  // 5. Configure CLK0 Control (Register 16)
  // 0x4F = Powered up, integer mode, PLLA source, 8mA drive
  si5351_write(16, 0x4F);

  // 6. Reset the PLLs (Register 177)
  // 0x20	Reset PLLA only
  // 0x80	Reset PLLB only
  // 0xA0	Reset Both PLLA and PLLB
  // 0xAC	Reset Both (Industry standard "Safe" reset)
  si5351_write(177, 0xAC);

  // 7. Enable the output (Register 3)
  // Clear bit 0 to enable CLK0
  si5351_write(3, 0xFE);
}

// Timer overflow ISR: keeps track of time beyond the 8-bit (255) limit
ISR(TIM0_OVF_vect) {
  if (timer_overflows < 255)
    timer_overflows++;
}

// External Interrupt ISR: Triggers on PB1 Falling Edge
ISR(INT0_vect) {
  //  Clock: 9.6 MHz (9,600,000 Hz)
  //  Prescaler: 1024
  //  Timer Frequency: 9,600,000/1024=9,375 Hz
  //  Tick Duration: 1/9.375 = 0.106667ms 0106.67μs
  //  PAL        20.0ms/106.67μs = 187 ticks
  //       mid point (187+156)/2 = 172 ticks
  //  NTSC       16.6ms/106.67μs = 156 ticks
  //
  // To get milliseconds:
  // ms = (ticks*1024)/(F_CPU/1000)=(156*1024)/(96000000/1000)=16.64

  // Calculate total ticks: (Overflows * 256) + current remainder
  // uint32_t ticks = (timer_overflows << 8) + TCNT0;

  // Only measure if this isn't the very first pulse (prevents junk data)
  if (first_pulse) {
    first_pulse = 0;
    timer_overflows = 0; // reset overflow counter
    TCNT0 = 0;           // reset timer
    measuredRegion = UNKNOWN;
    return;
  }

  if (timer_overflows > 0) {
    // if timer overflow (no pulses/too long during PS boot)
    // do no what is better: TODO: make a decision
    // 1.do nothing
    // 2. set PAL
    // 3. set NTFS
  } else {
    if (TCNT0 < DEBOUNCE_TICKS) {
      return;
    }
    if (TCNT0 >= DEBOUNCE_TICKS && TCNT0 < 172) {
      measuredRegion = NTSC;
    } else if (TCNT0 >= 172) {
      measuredRegion = PAL;
    }
  }

  timer_overflows = 0; // reset overflow counter
  TCNT0 = 0;           // reset timer
}

void timer_setup() {
  // TCCR0B: Timer/Counter Control Register B. It is the primary register used
  // to select the clock source and the prescaler for the 8-bit timer.
  /*Clock Select (CS0x) Table for ATtiny13
      CS02 	CS01	CS00	Description
      0     0     0   	Timer Stopped
      0     0     1   	No Prescaling (Direct Clock)
      0     1     0   	Prescaler 8
      0     1     1   	Prescaler 64
      1     0     0   	Prescaler 256
      1     0     1   	Prescaler 1024*/
  TCCR0B |= (1 << CS02) | (1 << CS00); // Prescaler 101 = 1024
  // ATtiny13 is running at the 9.6MHz(CKDIV8 is disabled)
  // the timer will increment once every 1024 clock cycles.
  // timer will only tick ~9,375 times per second.
  /*TIMSK0: Timer Interrupt Mask Register 0:
      enabling/disabling specific timer-related interrupts.
    TOIE0: Timer Overflow Interrupt Enable 0, timer max value 255*/
  TIMSK0 |= (1 << TOIE0); // Enable Timer Overflow Interrupt
}

void interrupt_setup() {
  // MCUCR – MCU Control Register: contains the Interrupt Sense Control bits.
  //    Specifically, bits ISC01 & ISC00 set the behavior:
  //    ISC01	ISC00	Description
  //    0    0	    The low level of INT0 generates an interrupt request.
  //    0    1	    Any logical change on INT0 generates an interrupt request.
  //    1    0     The falling edge of INT0 generates an interrupt request.
  //    1    1     The rising edge of INT0 generates an interrupt request.
  MCUCR |= (1 << ISC01);  // falling
  MCUCR &= ~(1 << ISC00); // -//-
  /*GIMSK (General Interrupt Mask Register): control register that enables or
    disables a specific type of external interrupt.*/
  GIMSK |= (1 << INT0); // Enable External Interrupt Request INT0

  sei(); // Global interrupt enable
}

void ic501_set_pin7(uint8_t rgn) {
  // IC501 24pin "SONY CXA1645M" Analog RGB to Composite
  // https://www.consolesunleashed.com/files/img/pinouts/sony-cxa1645m-rgb-encoder-890x1024.png
  // On early PU-8 boards, IC501 pin 7 must be lifted(INPUT) for NTSC
  // and puled to ground (OUTPUT LOW) for PAL
  if (rgn == PAL) {
    DDRB |= (1 << PIN_IC501);   // PIN_IC501 as an output
    PORTB &= ~(1 << PIN_IC501); // PIN_IC501 set ot LOW
  } else if (rgn == NTSC) {
    DDRB &= ~(1 << PIN_IC501); // PIN_IC501 as an INPUT
  }
}