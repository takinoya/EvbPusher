/*
    SPDX-License-Identifier: Apache-2.0
    SPDX-FileCopyrightText: 2026 Sakae OTAKI <niagara.ta.ki.no@gmail.com>
 */

/*
 * EVB-Pusher on RP2040 based board : A tool for pushing switches on EVB(Evaluation board)
 *
 * == Default Pin assign for NanoR4/Nano/Nano_Every ==
 * +---+---------+------------------+---------------------------------------+
 * |idx| Pin no  | Type:Label       | Note                                  |
 * +---+---------+------------------+---------------------------------------+
 * |   |  D0     | (RXD)            | Do not use (UART RxD)                 |
 * |   |  D1     | (TXD)            | Do not use (UART TxD)                 |
 * | 0 |  D2     | SW:od1           |                                       |
 * | 1 |  D3(~)  | SW:od2           |                                       |
 * | 2 |  D4     | SW:od3           |                                       |
 * | 3 |  D5(~)  | PWM:light        |                                       |
 * | 4 |  D6(~)  | SRV:srv1         |                                       |
 * | 5 |  D7     | SRV:srv2         |                                       |
 * | 6 |  D8     | SRV:srv3         |                                       |
 * | 7 |  D9(~)  | SRV:srv4         |!? PWM not work on Nano (by SERVO Lib) |
 * | 8 | D10(~)  | VOID:void10      |!? PWM not work on Nano (by SERVO Lib) |
 * | 9 | D11     | IN:in11          |                                       |
 * |10 | D12     | IN:in12          |                                       |
 * |   | D13     | (LEDB)           | Used by Built-in LED_BUILTIN.         |
 * |11 | D14(A0) | SW:relay1        |                                       |
 * |12 | D15(A1) | SW:relay2        |                                       |
 * |13 | D16(A2) | ADC:a2           |                                       |
 * |14 | D17(A3) | ADC:a3           |                                       |
 * |   | D18(A4) | (Reserved:I2C)   | Used for I2C_SDA                      |
 * |   | D19(A5) | (Reserved:I2C)   | Used for I2C_SCL                      |
 * |15 | D20(A6) | SW:relay3        | Nano cannot use to SW. (Turn to void) |
 * |16 | D21(A7) | SW:relay4        | Nano cannot use to SW. (Turn to void) |
 * +---+--------+------------------+---------------------------------------+
 *     Type
 *         IN  : Digital input (GPI)
 *         PWM : PWM output
 *         SW  : Switch output (GPO)
 *         ADC : A/D converter
 *         SRV : Servo PWM
 *         VOID: No function assigned
 *
 * == Default Pin assign for SEEED_XIAO_RP2040 ==
 * +---+---------+------------------+---------------------------------------+
 * |idx| Pin     | Type:Label       | Note                                  |
 * +---+---------+------------------+---------------------------------------+
 * | 0 |  D0(A0) | ADC:a0           |                                       |
 * | 1 |  D1(A1) | SW:5v            |                                       |
 * | 2 |  D2(A2) | SW:12v1          |                                       |
 * | 3 |  D3(A3) | SW:12v2          |                                       |
 * | 4 |  D4     | VOID:void4       | (Reserved for I2C extender I2C:SDA)   |
 * | 5 |  D5     | VOID:void5       | (Reserved for I2C extender I2C:SCL)   |
 * |   |  D6     | (TX)             | Do not use (UART0 for Serial1)        |
 * |   |  D7     | (RX)             | Do not use (UART0 for Serial1)        |
 * | 6 |  D8     | PWM:pwm8         | (SPI:SCK)                             |
 * | 7 |  D9     | IN:in9           | (SPI:MISO)                            |
 * | 8 | D10     | SRV:srv10        | (SPI:MOSI)                            |
 * +---+---------+------------------+---------------------------------------+
 *     Type
 *         IN  : Digital input (GPI)
 *         PWM : PWM output
 *         SW  : Switch output (GPO)
 *         ADC : A/D converter
 *         SRV : Servo PWM
 *         VOID: No function assigned
 *
 *     Note for SEEED_XIAO_RP2040
 *         USER_LED_R : GPIO17
 *         USER_LED_B : GPIO25
 *         USER_LED_G : GPIO26
 *         Serial usage : https://arduino-pico.readthedocs.io/en/latest/serial.html
 */

/*
 * Source File separation
 *
 *     - sketch_EVB_Pusher_AVR.ino
 *         - Main routins
 *             - loop
 *             - call setup functions
 *         - System wide variables
 *         - Configuration/Definition
 *         - Serial command line functions
 *         - Debug function
 *     - 11_eeprom.ino
 *         - Save/load to EEPROM relative implementation
 *     - 21_sw_in.ino
 *         - Digital pin relative implementation
 *             - SW<SWitch>
 *             - IN<Input>
 *     - 22_servo.ino
 *         - SRV<SERVO> relative implementation
 *     - 23_pwm.ino
 *         - PWM relative implementation
 *     - 24_adc.ino
 *         - ADC relative implementation
 *     - 25_i2c.ino
 *         - I2C relative implementation
 *     - 91_cmdline.ino
 *         - Command line intarface relative implementation
 */

/*
 * System wide global variables / Definitions
 */
#define MAX_CMDLINE_LEN 64
#define MAX_ARGC 6
struct sysvar_per_core_t {
  String cmdline_string;
  String argv[MAX_ARGC];
  int argc;
  String last_cmd_error_mesg;
  HardwareSerial* serial;
};

#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)
struct sysvar_per_core_t sysvar_core[2];
#else //#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)
struct sysvar_per_core_t sysvar_core[1];
#endif //#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)

/*
 * Platform cofiguration
 */
#if defined(ARDUINO_AVR_NANO_EVERY)
/***********************************
 * Core board : Arduino Nano Every *
 ***********************************/
#define NR_SERVO_INSTANCE           8
    // Note:
    //   Servo lib can make instance up to 12.
    //   But for decrease RAM usage, I choised 8 now.

#define NR_PIN_CTRL_ENTRY           17
    // Note:
    //   NanoEvery and Nano has controllable pin up to 17.
    //   See Top comment of this file.

#define FIXED_PIN_LABEL_STR_LEN     8
    // Note:
    //   I decided 8 for EEPROM shortage on NanoEvery.

#define EEPROM_SAVE_REGION_LEN  256
    // Note:
    //   For reduce unneeded calculation, use fixed 256byte.

// NanoEvery limitation descriptor
const uint8_t pin_white_list_adc[] = {14, 15, 16, 17, 18, 19, 20, 21};
const uint8_t pin_white_list_pwm[] = {3, 5, 6, 9, 10};
const uint8_t pin_black_list_digital[] = {0, 1, 13};

#elif defined(ARDUINO_AVR_NANO)
/*****************************
 * Core board : Arduino Nano *
 *****************************/
#define NR_SERVO_INSTANCE           8
    // Note:
    //   Servo lib can make instance up to 12.
    //   But for decrease RAM usage, I choised 8 now.

#define NR_PIN_CTRL_ENTRY           17
    // Note:
    //   NanoEvery and Nano has controllable pin up to 17.
    //   See Top comment of this file.

#define FIXED_PIN_LABEL_STR_LEN     8
    // Note:
    //   I decided 8 for EEPROM shortage on NanoEvery.

#define EEPROM_SAVE_REGION_LEN  256
    // Note:
    //   For reduce unneeded calculation, use fixed 256byte.

// Nano limitation descriptor
const uint8_t pin_white_list_adc[] = {14, 15, 16, 17, 18, 19};
const uint8_t pin_white_list_pwm[] = {3, 5, 6, 11};
const uint8_t pin_black_list_digital[] = {0, 1, 13, 20, 21};

#elif defined(ARDUINO_NANO_R4)
#error "Not supported yet. under planning."

#define NR_SERVO_INSTANCE           8
    // Note:
    //   Servo lib can make instance up to 12.
    //   But for decrease RAM usage, I choised 8 now.

#define NR_PIN_CTRL_ENTRY           17
    // Note:
    //   NanoR4, NanoEvery and Nano has controllable pin up to 17.
    //   See Top comment of this file.

#define FIXED_PIN_LABEL_STR_LEN     8
    // Note:
    //   I decided 8 for EEPROM shortage on NanoEvery.

#define EEPROM_SAVE_REGION_LEN  320
    // Note:
    //   Expected size is 280.
    //     Align 64byte ==> 64*5 ==> 320byte

// NanoEvery limitation descriptor
const uint8_t pin_white_list_adc[] = {14, 15, 16, 17, 18, 19, 20, 21};
const uint8_t pin_white_list_pwm[] = {3, 5, 6, 9, 10};
const uint8_t pin_black_list_digital[] = {0, 1, 13};

#elif defined(ARDUINO_SEEED_XIAO_RP2040)
/**********************************
 * Core board : Seeed Xiao RP2040 *
 **********************************/
#define NR_SERVO_INSTANCE           4
    // Note:
    //   arduino-pico's Servo lib can make instance up to 8.
    //   But for decrease PIN usage, I choised 4 now.

#define NR_PIN_CTRL_ENTRY           9
    // Note:
    //   SEED_XIAO_RP2040 has controllable pin up to 9.
    //   See Top comment of this file.

#define FIXED_PIN_LABEL_STR_LEN     8
    // Note:
    //   I decided 8 for EEPROM shortage on NanoEvery.

#define EEPROM_SAVE_REGION_LEN  256
    // Note:
    //   For reduce unneeded calculation, use fixed 256byte.

// Xiao RP2040 limitation descriptor
const uint8_t pin_white_list_adc[] = {0, 1, 2, 3};
const uint8_t pin_white_list_pwm[] = {6, 7};
const uint8_t pin_black_list_digital[] = {6, 7};

#elif defined(ARDUINO_RASPBERRY_PI_PICO)
/**********************************
 * Core board : Raspberry PI Pico *
 **********************************/
#error "Not supported yet. under planning."

#define NR_SERVO_INSTANCE           6
    // Note:
    //   arduino-pico's Servo lib can make instance up to 6.
    //   But for decrease PIN usage, I choised 4 now.

#define NR_PIN_CTRL_ENTRY           24
    // Note:
    //   SEED_XIAO_RP2040 has controllable pin up to 9.
    //   See Top comment of this file.

#define FIXED_PIN_LABEL_STR_LEN     8
    // Note:
    //   I decided 8 for EEPROM shortage on NanoEvery.

#define EEPROM_SAVE_REGION_LEN  512
    // Note:
    //   For reduce unneeded calculation, use fixed 256byte.

// Xiao RP2040 limitation descriptor
const uint8_t pin_white_list_adc[] = {0, 1, 2, 3};
const uint8_t pin_white_list_pwm[] = {6, 7};
const uint8_t pin_black_list_digital[] = {6, 7};

#else //...#if defined(ARDUINO_AVR_NANO)
#error "Unknown board"

#endif //...#if defined(ARDUINO_AVR_NANO)

/*
 * Debug flags/macros
 */
typedef union {
  struct {
    uint8_t echo_enabled         : 1;   // bit0
    uint8_t init_debug_enabled   : 1;   // bit1 : For verbose debug print in initialize relatives
    uint8_t debug_enabled        : 1;   // bit2 : For verbose debug print
    uint8_t cmd_debug_enabled    : 1;   // bit3 : For verbose debug print in cmdline relatives
    uint8_t tim_debug_enabled    : 1;   // bit4 : For verbose debug print in timing relatives(e.g. toggle)
    uint8_t i2c_debug_enabled    : 1;   // bit5 : For verbose debug print in I2C relatives
    uint8_t reserved_bit6        : 1;   // bit6 (Reserved, not used yet)
    uint8_t reserved_bit7        : 1;   // bit7 (Reserved, not used yet)
  } flags;
  uint8_t body;
} debug_flag_byte_t;
static_assert(sizeof(debug_flag_byte_t) == 1, "struct debug_flag_byte_t is broken.");

debug_flag_byte_t dbg_flg_boot;
debug_flag_byte_t dbg_flg = {
  .flags = {
    .echo_enabled = 0,        // false
    .init_debug_enabled = 1,  // true
    .debug_enabled = 0,       // false
    .cmd_debug_enabled = 0,   // false
    .tim_debug_enabled = 0,   // false
    .i2c_debug_enabled = 0,   // false
  },
};

#define IS_INIT_DEBUG_ENABLED   dbg_flg.flags.init_debug_enabled
#define IS_DEBUG_ENABLED        dbg_flg.flags.debug_enabled
#define IS_CMD_DEBUG_ENABLED    dbg_flg.flags.cmd_debug_enabled
#define IS_TIM_DEBUG_ENABLED    dbg_flg.flags.tim_debug_enabled
#define IS_I2C_DEBUG_ENABLED    dbg_flg.flags.i2c_debug_enabled

#define INIT_PRINT(...)      {if (IS_INIT_DEBUG_ENABLED) {sys->serial->print(__VA_ARGS__);};}
#define INIT_PRINTLN(...)    {if (IS_INIT_DEBUG_ENABLED) {sys->serial->println(__VA_ARGS__);};}
#define DBG_PRINT(...)       {if (IS_DEBUG_ENABLED) {sys->serial->print(__VA_ARGS__);};}
#define DBG_PRINTLN(...)     {if (IS_DEBUG_ENABLED) {sys->serial->println(__VA_ARGS__);};}
#define CMD_DBG_PRINT(...)   {if (IS_CMD_DEBUG_ENABLED) {sys->serial->print(__VA_ARGS__);};}
#define CMD_DBG_PRINTLN(...) {if (IS_CMD_DEBUG_ENABLED) {sys->serial->println(__VA_ARGS__);};}
#define TIM_DBG_PRINT(...)   {if (IS_TIM_DEBUG_ENABLED) {sys->serial->print(__VA_ARGS__);};}
#define TIM_DBG_PRINTLN(...) {if (IS_TIM_DEBUG_ENABLED) {sys->serial->println(__VA_ARGS__);};}
#define I2C_DBG_PRINT(...)   {if (IS_I2C_DEBUG_ENABLED) {sys->serial->print(__VA_ARGS__);};}
#define I2C_DBG_PRINTLN(...) {if (IS_I2C_DEBUG_ENABLED) {sys->serial->println(__VA_ARGS__);};}

/*******************************************************************************
 * PIN Control table implementation
 ******************************************************************************/
#define DEFAULT_SW_PUSH_DURATION 250
                             //  ^^^ 250msec

#define DEFAULT_SERVO_HOLD_TIME 330
                             // ^^^ 330msec

#define PIN_TYPE_VOID    0  // No function assigned
#define PIN_TYPE_IN      1
#define PIN_TYPE_PWM     2
#define PIN_TYPE_SW      3
#define PIN_TYPE_ADC     4
#define PIN_TYPE_SRV     5  // Auto allocation to SERVO instance
#define PIN_TYPE_SRV0   10  // Reserved for future static allocation
#define PIN_TYPE_SRV1   11  // Reserved for future static allocation
#define PIN_TYPE_SRV2   12  // Reserved for future static allocation
#define PIN_TYPE_SRV3   13  // Reserved for future static allocation
#define PIN_TYPE_SRV4   14  // Reserved for future static allocation
#define PIN_TYPE_SRV5   15  // Reserved for future static allocation
#define PIN_TYPE_SRV6   16  // Reserved for future static allocation
#define PIN_TYPE_SRV7   17  // Reserved for future static allocation

#define PIN_POL_ACTIVE_HI   1
#define PIN_POL_ACTIVE_LO   0

struct tunable_entry_sw_in {
  uint8_t polarity;
};

struct tunable_entry_servo {
  uint8_t deg_dn;
  uint8_t deg_n;
  uint8_t deg_up;
};

struct tunable_entry_pwm {
  int duty;
};

struct tunable_entry {
  uint8_t type;
  int16_t duration;
  union {
    struct tunable_entry_sw_in sw;
    struct tunable_entry_sw_in in;
    struct tunable_entry_servo srv;
    struct tunable_entry_pwm   pwm;
  };
};
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_NANO_EVERY)
static_assert(sizeof(struct tunable_entry) == 6, "struct tunable_entry is broken.");
#else
static_assert(sizeof(struct tunable_entry) == 8, "struct tunable_entry is broken.");
#endif

#include <Servo.h>

struct pin_ctrl_entry_t {
  const int pin;    // pin number (Same to D*)
  unsigned long hold_start_time;
  unsigned long hold_duration;
  unsigned long hold_interval;
  int hold_value;   // Used by SERVO and PWM
  int cdn_toggle;   // Count down number of toggle
  Servo* srv_obj;
  struct tunable_entry tunable;
  char name[FIXED_PIN_LABEL_STR_LEN + 1];    // +1 for NULL termination
};

void factory_reset_pin(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr, const char* name, uint8_t type)
{
  strlcpy(pin_ctrl_ptr->name, name, FIXED_PIN_LABEL_STR_LEN + 1);
  pin_ctrl_ptr->cdn_toggle = 0;
  pin_ctrl_ptr->srv_obj = nullptr;
  pin_ctrl_ptr->tunable.type = type;
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      factory_reset_tunables_in(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      factory_reset_tunables_sw(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      factory_reset_tunables_servo(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      factory_reset_tunables_pwm(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      factory_reset_tunables_adc(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_VOID:
      factory_reset_tunables_void(sys, pin_ctrl_ptr);
      break;
    default:
      break;
  }
}

#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_NANO_EVERY) || defined(ARDUINO_NANO_R4)

struct pin_ctrl_entry_t pin_ctrl_tbl[] = {
  // idx 0
  { .pin = 2 },
  // idx 1
  { .pin = 3 },
  // idx 2
  { .pin = 4 },
  // idx 3
  { .pin = 5 },
  // idx 4
  { .pin = 6 },
  // idx 5
  { .pin = 7 },
  // idx 6
  { .pin = 8 },
  // idx 7
  { .pin = 9 },
  // idx 8
  { .pin = 10 },
  // idx 9
  { .pin = 11 },
  // idx 10
  { .pin = 12 },
  // idx 11
  { .pin = 14 },
  // idx 12
  { .pin = 15 },
  // idx 13
  { .pin = 16 },
  // idx 14
  { .pin = 17 },
  // idx 15
  { .pin = 20 },
  // idx 16
  { .pin = 21 },
};

void setup_pin_ctrl_tbl(struct sysvar_per_core_t * sys)
{
  // Setup factory settings once
  factory_reset_pin(sys, &pin_ctrl_tbl[0],  "od1",    PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[1],  "od2",    PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[2],  "od3",    PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[3],  "light",  PIN_TYPE_PWM);
  factory_reset_pin(sys, &pin_ctrl_tbl[4],  "srv1",   PIN_TYPE_SRV);
  factory_reset_pin(sys, &pin_ctrl_tbl[5],  "srv2",   PIN_TYPE_SRV);
  factory_reset_pin(sys, &pin_ctrl_tbl[6],  "srv3",   PIN_TYPE_SRV);
  factory_reset_pin(sys, &pin_ctrl_tbl[7],  "srv4",   PIN_TYPE_SRV);
  factory_reset_pin(sys, &pin_ctrl_tbl[8],  "void10", PIN_TYPE_VOID);
  factory_reset_pin(sys, &pin_ctrl_tbl[9],  "in11",   PIN_TYPE_IN);
  factory_reset_pin(sys, &pin_ctrl_tbl[10], "in12",   PIN_TYPE_IN);
  factory_reset_pin(sys, &pin_ctrl_tbl[11], "relay1", PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[12], "relay2", PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[13], "a2",     PIN_TYPE_ADC);
  factory_reset_pin(sys, &pin_ctrl_tbl[14], "a3",     PIN_TYPE_ADC);
  factory_reset_pin(sys, &pin_ctrl_tbl[15], "relay3", PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[16], "relay4", PIN_TYPE_SW);
                               // Note: ^^^^^^^^ 8 char max
  // Override via saved data
  load_from_eeprom(sys, true); // Load all data (include debug flags)
}

#elif defined(ARDUINO_SEEED_XIAO_RP2040)

struct pin_ctrl_entry_t pin_ctrl_tbl[] = {
  // idx 0
  { .pin = 0 },
  // idx 1
  { .pin = 1 },
  // idx 2
  { .pin = 2 },
  // idx 3
  { .pin = 3 },
  // idx 4
  { .pin = 4 },
  // idx 5
  { .pin = 5 },
  // idx 6
  { .pin = 8 },
  // idx 7
  { .pin = 9 },
  // idx 8
  { .pin = 10 },
};

void setup_pin_ctrl_tbl(struct sysvar_per_core_t * sys)
{
  // Setup factory settings once
  factory_reset_pin(sys, &pin_ctrl_tbl[0],  "a0",    PIN_TYPE_ADC);
  factory_reset_pin(sys, &pin_ctrl_tbl[1],  "5v",    PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[2],  "12v1",  PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[3],  "12v2",  PIN_TYPE_SW);
  factory_reset_pin(sys, &pin_ctrl_tbl[4],  "void4", PIN_TYPE_VOID);
  factory_reset_pin(sys, &pin_ctrl_tbl[5],  "void5", PIN_TYPE_VOID);
  factory_reset_pin(sys, &pin_ctrl_tbl[6],  "pwm8",  PIN_TYPE_PWM);
  factory_reset_pin(sys, &pin_ctrl_tbl[7],  "in9",   PIN_TYPE_IN);
  factory_reset_pin(sys, &pin_ctrl_tbl[8],  "srv10", PIN_TYPE_SRV);
                               // Note: ^^^^^^^^ 8 char max
  // Override via saved data
  load_from_eeprom(sys, true); // Load all data (include debug flags)
}

#else
#error "Unknown board"
#endif

struct pin_ctrl_entry_t*
search_pin_ctrl_entry_by_name(String& name)
{
  struct pin_ctrl_entry_t* ret = nullptr;

  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (String(pin_ctrl_tbl[i].name) == name) {
      ret = &pin_ctrl_tbl[i];
      break;
    }
  }
  return ret;
}

/****************************************
 * TOGGLE procedure
 ****************************************/
void proc_toggle(struct sysvar_per_core_t * sys) {
  unsigned long hold_time;
  for (int i = 0; i < sizeof(pin_ctrl_tbl)/sizeof(pin_ctrl_tbl[0]); i++) {
    if (pin_ctrl_tbl[i].cdn_toggle) {
      if (pin_ctrl_tbl[i].cdn_toggle % 2) {
        hold_time = pin_ctrl_tbl[i].hold_duration;  // End of hold_duration
      }
      else {
        hold_time = pin_ctrl_tbl[i].hold_interval;  // End of hold_interval
      }
      if (0 == hold_time) continue;  // 0 means infinite for ON or OFF command
      if ( millis() - pin_ctrl_tbl[i].hold_start_time >= hold_time ) {
        switch (pin_ctrl_tbl[i].tunable.type) {
          case PIN_TYPE_SW:
            do_sw_toggle(sys, &pin_ctrl_tbl[i]);
            break;
          case PIN_TYPE_SRV:
            do_servo_toggle(sys, &pin_ctrl_tbl[i]);
            break;
          case PIN_TYPE_PWM:
            do_pwm_toggle(sys, &pin_ctrl_tbl[i]);
            break;
          default:
            // Error
            TIM_DBG_PRINTLN(F("#TIM: TOGGLE: Unsupported for this pin_type !?!?"));
            break;
        }
        // Updatge count down value
        --pin_ctrl_tbl[i].cdn_toggle;
        TIM_DBG_PRINTLN(String(F("#TIM: TOGGLE: cdn_toggle=")) + pin_ctrl_tbl[i].cdn_toggle);
      }
    }
  }
}

/***********************************************************************
 * Debug implementation
 *
 * - Heatbeat blinking LED
 * - Care to debug flags
 ***********************************************************************/
void proc_blink_led(struct sysvar_per_core_t * sys) {
  // Blinking LED_BUILTIN is used for catch up freezing.
  static int led_out = HIGH;
  static unsigned long last_tick = 0;

  if ( millis() - last_tick >= 500 ) {
    //                         ^^^ 500msec
    digitalWrite(LED_BUILTIN, led_out);
    led_out = !led_out;
    last_tick = millis();
  }
}

void setup_debug(struct sysvar_per_core_t * sys) {
  dbg_flg_boot = dbg_flg; // Save dbg_flg at boot up
  pinMode(LED_BUILTIN, OUTPUT);
}

/*
 * ARDUINO framework implementation
 */
void setup() {
  // put your setup code here, to run once:
  /*
   * Setup Serial port first.
   */
#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)
  sysvar_core[0].serial = &Serial1;
#else //#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)
  sysvar_core[0].serial = &Serial;
#endif //#else //#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)

  /*
   * Setup lower layer (Debug flags are derived from built-in)
   */
  setup_eeprom(&sysvar_core[0]);
  setup_cmdline(&sysvar_core[0]);
  setup_pin_ctrl_tbl(&sysvar_core[0]);
  setup_debug(&sysvar_core[0]);
  
  /*
   * Setup upper layer (Debug flags are derived from EEPROM)
   */
  setup_in(&sysvar_core[0]);
  setup_sw(&sysvar_core[0]);
  setup_servo(&sysvar_core[0]);
  setup_pwm(&sysvar_core[0]);
  setup_adc(&sysvar_core[0]);
  setup_void_pin(&sysvar_core[0]);
}

void loop() {
  // put your main code here, to run repeatedly:
  proc_receive_cmdline(&sysvar_core[0]);
  proc_toggle(&sysvar_core[0]);
  proc_blink_led(&sysvar_core[0]);
}

#if defined(ARDUINO_SEEED_XIAO_RP2040) || defined(ARDUINO_RASPBERRY_PI_PICO)
/*
 * RP2040 series has dual command stream port
 *     1. USB
 *     2. UART0
 * So, Core1 to use USB-UART
 */

void setup1() {
  // For RP2040 Core1:
  sysvar_core[1].serial = &Serial;  // USB Serial
  setup_cmdline(&sysvar_core[1]);
}

void loop1() {
  // For RP2040 Core1
  proc_receive_cmdline(&sysvar_core[1]);
}
#endif // ...#if defined(ARDUINO_SEEED_XIAO_RP2040)
