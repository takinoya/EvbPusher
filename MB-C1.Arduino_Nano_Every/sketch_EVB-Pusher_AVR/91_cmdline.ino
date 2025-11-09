/*
 * Implementation for CLI
 */
#define PUSHPULL_MODE_OFF   0
#define PUSHPULL_MODE_PUSH  1
#define PUSHPULL_MODE_PULL  3

/******************************************************************************
 * Command line intermediate function
 *****************************************************************************/
int _cmd_pushpull(int repeat, int duration, int interval, int mode)
{
  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  // Do push action by pin type
  if (PUSHPULL_MODE_PULL == mode) {  // PULL operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("PULL:SW"));
        do_sw_off(pin_ctrl_ptr);      // For SW, OFF is equivalent to PULL.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("PULL:SERVO"));
        do_servo_up(pin_ctrl_ptr);    // For SERVO, UP is wquivalent to PULL.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("PULL:PWM"));
        do_pwm_off(pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("PULL:Unsupported"));
        last_cmd_error_mesg = F("PULL: Unsupported for this pin_type !?!?");
        return -1;
        break;
    }
  }
  else if (PUSHPULL_MODE_PUSH == mode){  // PUSH operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("PUSH:SW"));
        do_sw_on(pin_ctrl_ptr);
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("PUSH:SERVO"));
        do_servo_down(pin_ctrl_ptr);    // For Serbo, DOWN operation is same to PUSH.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("PUSH:PWM"));
        do_pwm_on(pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("PUSH:Unsupported"));
        last_cmd_error_mesg = F("PUSH: Unsupported for this pin_type !?!?");
        return -1;
        break;
    }
  }
  else {  // OFF operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("OFF:SW"));
        do_sw_off(pin_ctrl_ptr);
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("OFF:SERVO"));
        do_servo_n(pin_ctrl_ptr);    // For SERVO, N is equivalent to OFF.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("OFF:PWM"));
        do_pwm_off(pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("OFF:Unsupported"));
        last_cmd_error_mesg = F("OFF: Unsupported for this pin_type !?!?");
        return -1;
        break;
    }
  }
  // Set toggle information
  pin_ctrl_ptr->cdn_toggle = 1 + ((repeat - 1) * 2);
                             // ex. sinble click means  1 toggle
                             //     double click means  3 toggle
                             //     tripple click means 5 toggle
  pin_ctrl_ptr->hold_duration = duration;
  pin_ctrl_ptr->hold_interval = interval;
  pin_ctrl_ptr->hold_start_time = millis();

  return 0;
}

void print_list_entry_header()
{
  Serial.println(F("|Pin|Name|Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|"));
  Serial.println(F("|:---|:---|:---|:---|:---|:---|:---|"));
}

void print_list_entry_common(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  Serial.print(F("|"));
  Serial.print(pin_ctrl_ptr->pin, DEC);
  Serial.print(F("|"));
  Serial.print(pin_ctrl_ptr->name);
  Serial.print(F("|"));
}

void print_list_entry_in(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.print(F("IN|"));
  Serial.print(pin_ctrl_ptr->tunable.duration, DEC);
  Serial.print(F("|"));
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.in.polarity) {
    Serial.println(F("ACTIVE_LO|-|-|"));
  }
  else {
    Serial.println(F("ACTIVE_HI|-|-|"));
  }
}

void print_list_entry_sw(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.print(F("SW|"));
  Serial.print(pin_ctrl_ptr->tunable.duration, DEC);
  Serial.print(F("|"));
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    Serial.println(F("ACTIVE_LO|-|-|"));
  }
  else {
    Serial.println(F("ACTIVE_HI|-|-|"));
  }
}

void print_list_entry_srv(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.print(F("SRV|"));
  Serial.print(pin_ctrl_ptr->tunable.duration, DEC);
  Serial.print(F("|"));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_dn, DEC);
  Serial.print(F("|"));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_n, DEC);
  Serial.print(F("|"));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_up, DEC);
  Serial.println(F("|"));
}

void print_list_entry_pwm(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.print(F("PWM|"));
  Serial.print(pin_ctrl_ptr->tunable.duration, DEC);
  Serial.print(F("|0x"));
  Serial.print(pin_ctrl_ptr->tunable.pwm.duty, HEX);
  Serial.println(F("|-|-|"));
}

void print_list_entry_adc(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.println(F("ADC|-|-|-|-|"));
}

void print_list_entry_void(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.println(F("VOID|-|-|-|-|"));
}

void print_list_entry_unknown(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  Serial.println(F("???|-|-|-|-|"));
}

void print_list_entry(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      print_list_entry_in(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      print_list_entry_sw(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      print_list_entry_srv(pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      print_list_entry_pwm(pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      print_list_entry_adc(pin_ctrl_ptr);
      break;
    case PIN_TYPE_VOID:
      print_list_entry_void(pin_ctrl_ptr);
      break;
    default:
      print_list_entry_unknown(pin_ctrl_ptr);
      break;
  }
}

/*******************************************************************************
 * Command body implementation
 ******************************************************************************/

/* Command : PUSH <pin_label> [duration] [[repeat] [interval]]
 *
 * Note:
 *   TBD
 */
int cmd_push() {
  int arg_repeat = 1;     // Default: 1 push
  int arg_duration = -1;  // Negative value means will be override default value
  int arg_interval = -1;  // Negative value means will be override default value

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  if (4 <= argc) {
    // Has repeat
    arg_repeat = argv[3].toInt();
  };
  if (5 <= argc) {
    // Has off_duration
    arg_interval = argv[4].toInt();
  };

  return _cmd_pushpull(arg_repeat, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : PUSH2X <pin_label> [duration]
 *
 * Note:
 *   - Concept : Double click
 */
int cmd_push2x() {
  int arg_duration = -1;
  int arg_interval = -1;

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  if (4 <= argc) {
    // Has off_duration
    arg_interval = argv[3].toInt();
  };

  return _cmd_pushpull(2, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : PUSH3X <pin_label> [duration]
 *
 * Note:
 *   - Concept : Double click
 */
int cmd_push3x() {
  int arg_duration = -1;
  int arg_interval = -1;

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  if (4 <= argc) {
    // Has interval
    arg_interval = argv[3].toInt();
  };

  return _cmd_pushpull(3, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : ON <pin_label> [duration]
 *
 * Note:
 *   Equivalent to PUSH
 *   - repeat   : Fixed 1
 *   - duration : Default - Infinite
 *   - interval : Do not care
 */
int cmd_on() {
  int arg_duration = 0;  // Default : Infinite

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  return _cmd_pushpull(1, arg_duration, 0, PUSHPULL_MODE_PUSH);  // Convert to infinite push once
}

/* Command : OFF <pin_label> [duration]
 *
 * Note:
 *   Equivalent to PUSH
 *   - repeat   : Fixed 1
 *   - duration : Default - Infinite
 *   - interval : Do not care
 */
int cmd_off() {
  int arg_duration = 0;  // Default : Infinite

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  return _cmd_pushpull(1, arg_duration, 0, PUSHPULL_MODE_OFF);  // Convert to infinite pull once
}

/* Command : PULL <pin_label> [duration] [[repeat] [interval]]
 *
 * Note:
 *   Equivalent to PUSH
 *   - is_pull : true (pull mode)
 */
int cmd_pull()
{
  int arg_repeat = 1;     // Default: 1 push
  int arg_duration = -1;  // Negative value means will be override default value
  int arg_interval = -1;  // Negative value means will be override default value

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  if (4 <= argc) {
    // Has repeat
    arg_repeat = argv[3].toInt();
  };
  if (5 <= argc) {
    // Has off_duration
    arg_interval = argv[4].toInt();
  };

  return _cmd_pushpull(arg_repeat, arg_duration, arg_interval, PUSHPULL_MODE_PULL);
}

#if 0
/* Command : N <pin_label>
 *
 * Note:
 *   - SERVO only
 *   - Move and keep to N position
 */
int cmd_up()
{
  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  do_servo_n(pin_ctrl_ptr);
  return 0;
}
#endif //#if 0

/* Command : UP <pin_label> [duration]
 *
 * Note:
 *   Equivalent to PULL
 *   - repeat   : Fixed 1
 *   - duration : Default - Infinite
 *   - interval : Do not care
 */
int cmd_up() {
  int arg_duration = 0;  // Default : Infinite

  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= argc) {
    // Has duration
    arg_duration = argv[2].toInt();
  };
  return _cmd_pushpull(1, arg_duration, 0, PUSHPULL_MODE_PULL);  // Convert to infinite pull once
}

/* Command : DUTY <pin_label> <duty>
 *
 * Note:
 *   TBD
 */
int cmd_duty() {
  if (argc < 3) {
    last_cmd_error_mesg = F("At least 3 arguments required !?!?");
    return -1;
  }
  int duty = argv[2].toInt();
  if ((duty < 0) || (100 < duty)) {
    last_cmd_error_mesg = F("Range error (should 0-100) !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  do_pwm_set_duty(pin_ctrl_ptr, duty);

  return 0;
}

/* Command : I2CR
 *
 * Note:
 *   - TBD
 */
int cmd_i2cr() {
  return 0; // To be implemented
}

/* Command : I2CW
 *
 * Note:
 *   - TBD
 */
int cmd_i2cw() {
  return 0; // To be implemented
}

/* Command : LIST
 *
 * Note:
 *   - Print pin control settings as Markdown table.
 */
int cmd_list() {
  print_list_entry_header();

  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    print_list_entry(&pin_ctrl_tbl[i]);
  }
  return 0;
}

/* Command : STATUS <pin_label>
 *
 * Note:
 *   - ToDo
 */
int cmd_status() {
  if (argc < 2) {
    last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  // branch per pin type
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      print_status_in(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      print_status_sw(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      print_status_servo(pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      print_status_pwm(pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      print_status_adc(pin_ctrl_ptr);
      break;
    default:
      last_cmd_error_mesg = F("Unsupported pin type !?!?");
      return -1;
      break;
  }
  return 0;
}

bool type_change_to(struct pin_ctrl_entry_t* pin_ctrl_ptr, String& typeStr)
{
  bool (*type_change_func)(struct pin_ctrl_entry_t* pin_ctrl_ptr) = nullptr;
  argv[3].toUpperCase();
  if (String(F("VOID")) == argv[3]) {
    type_change_func = type_change_to_void;
  }
  else if (String(F("ADC")) == argv[3]) {
    type_change_func = type_change_to_adc;
  }
  else if (String(F("IN")) == argv[3]) {
    type_change_func = type_change_to_in;
  }
  else if (String(F("SW")) == argv[3]) {
    type_change_func = type_change_to_sw;
  }
  else if (String(F("SRV")) == argv[3]) {
    type_change_func = type_change_to_srv;
  }
  else if (String(F("PWM")) == argv[3]) {
    type_change_func = type_change_to_pwm;
  }
  else {
    last_cmd_error_mesg = F("Unknown pin type !?!?");
    return -1;
  }
  // Pre operation per pin type
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      type_change_from_in(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      type_change_from_sw(pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      type_change_from_srv(pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      type_change_from_pwm(pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      type_change_from_adc(pin_ctrl_ptr);
      break;
    case PIN_TYPE_VOID:
      type_change_from_void(pin_ctrl_ptr);
      break;
    default:
      last_cmd_error_mesg = F("Unexpected error !?!?");
      return -1;
      break;
  }
  
  // Do it with check
  return type_change_func(pin_ctrl_ptr);
}

/* Command : CONFIG <pin_label> [<key> <value>]
 *
 * Note:
 *   - ToDo
 */
int cmd_config() {
  bool is_key_value_mode = true;

  if (argc == 2) {
    is_key_value_mode = false;
  }
  else if (argc < 4) {
    last_cmd_error_mesg = F("At least 1 or 3 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }
  
  if (is_key_value_mode) {
    // <key> <value> operation
    if (String(F("type")) == argv[2]) {
      if (false == type_change_to(pin_ctrl_ptr, argv[3]) ) {
        // last_cmd_error_mesg is already stored in type_chage_to_*()
        return -1;
      }
    }
    else if (String(F("duration")) == argv[2]) {
      pin_ctrl_ptr->tunable.duration = argv[3].toInt();
    }
    else if (String(F("polarity")) == argv[2]) {
      pin_ctrl_ptr->tunable.sw.polarity = argv[3].toInt();
    }
    else if (String(F("deg_dn")) == argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_dn = argv[3].toInt();
    }
    else if (String(F("deg_n")) == argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_n = argv[3].toInt();
    }
    else if (String(F("deg_up")) == argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_up = argv[3].toInt();
    }
    else if (String(F("duty")) == argv[2]) {
      pin_ctrl_ptr->tunable.pwm.duty = argv[3].toInt();
    }
    else {
      last_cmd_error_mesg = F("Unknown key !?!?");
      return -1;
    }
  }
  // Print result
  print_list_entry_header();
  print_list_entry(pin_ctrl_ptr);
  return 0;
}

/* Command : RENAME <pin_label> <new_pin_label>
 *
 * Note:
 *   - ToDo
 */
int cmd_rename() {
  if (argc < 3) {
    last_cmd_error_mesg = F("At least 2 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }
  // Do rename
  strlcpy(pin_ctrl_ptr->name, argv[2].c_str(), FIXED_PIN_LABEL_STR_LEN + 1);
  return 0;
}

/* Command : RESET
 *
 * Note:
 *   - Use WDT to reset
 */
#include <avr/wdt.h>

int cmd_reset() {
  INIT_PRINTLN(F("#INIT: Reset via WDT."));
  //wdt_disable();
#if defined(ARDUINO_AVR_NANO)
  wdt_enable(WDTO_15MS);
#elif defined(ARDUINO_AVR_NANO_EVERY)
  wdt_enable(WDT_PERIOD_32CLK_gc);
#endif
  wdt_reset();
  while (1) {};
  return 0;
}

/*
 * Debug commands
 */

/* Command : DEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_debug_on() {
  Serial.println(F("Turn on debug message.") );
  dbg_flg.flags.debug_enabled = 1;  // true
  return 0;
}

/* Command : DEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_debug_off() {
  Serial.println(F("Turn off debug message.") );
  dbg_flg.flags.debug_enabled = 0;  // false
  return 0;
}

/* Command : INITDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_init_debug_on() {
  Serial.println(F("Turn on init debug message.") );
  dbg_flg.flags.init_debug_enabled = 1;  // true
  return 0;
}

/* Command : INITDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_init_debug_off() {
  Serial.println(F("Turn off init debug message.") );
  dbg_flg.flags.init_debug_enabled = 0;  // false
  return 0;
}

/* Command : CMDDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_cmd_debug_on() {
  Serial.println(F("Turn on cmdline debug message.") );
  IS_CMD_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : CMDDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_cmd_debug_off() {
  Serial.println(F("Turn off cmdline debug message.") );
  IS_CMD_DEBUG_ENABLED = 0;  // false
  return 0;
}

/* Command : TIMDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_tim_debug_on() {
  Serial.println(F("Turn on timing debug message.") );
  IS_TIM_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : TIMDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_tim_debug_off() {
  Serial.println(F("Turn off timing debug message.") );
  IS_TIM_DEBUG_ENABLED = 0;  // false
  return 0;
}

/* Command : I2CDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_i2c_debug_on() {
  Serial.println(F("Turn on I2C debug message.") );
  IS_I2C_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : I2CDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_i2c_debug_off() {
  Serial.println(F("Turn off I2C debug message.") );
  IS_I2C_DEBUG_ENABLED = 0;  // false
  return 0;
}
#if 0
/* Command : *DEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_*_debug_on() {
  Serial.println(F("Turn on * debug message.") );
  IS_*_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : *DEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_*_debug_off() {
  Serial.println(F("Turn off * debug message.") );
  IS_*_DEBUG_ENABLED = 0;  // false
  return 0;
}
#endif //#if 0

/* Command : ECHO=ON
 *
 * Note:
 *   TBD
 */
int cmd_echo_on() {
  Serial.println(F("Turn on echo back to serial.") );
  dbg_flg.flags.echo_enabled = 1; // true
  return 0;
}

/* Command : ECHO=OFF
 *
 * Note:
 *   TBD
 */
int cmd_echo_off() {
  Serial.println(F("Turn off echo back to serial.") );
  dbg_flg.flags.echo_enabled = 0; // false
  return 0;
}

/* Command : LOAD [OPTIONS]
 *
 * OPTIONS:
 *   --with-debug :
 *     Load with debug flags
 *
 * Note:
 *   TBD
 */
int cmd_load() {
  if (2 <= argc) {
    // Has option
    if (String(F("--with-debug")) == argv[1]) {
      CMD_DBG_PRINTLN("#DBG: LOAD : load with debug flags\n");
      return load_from_eeprom(true);
    }
    else {
      CMD_DBG_PRINTLN("#DBG: LOAD : Skip unknown option\n");
    }
  }
  return load_from_eeprom(false);
}

/* Command : SAVE [OPTIONS]
 *
 * OPTIONS:
 *   --with-debug :
 *     Load with debug flags
 *
 * Note:
 *   TBD
 */
int cmd_save() {
  if (2 <= argc) {
    // Has option
    if (String(F("--with-debug")) == argv[1]) {
      CMD_DBG_PRINTLN("#DBG: SAVE : save with debug flags\n");
      return save_to_eeprom(true);
    }
    else {
      CMD_DBG_PRINTLN("#DBG: SAVE : Unknown option\n");
    }
  }
  return save_to_eeprom(false);
}

/* Command : EEPROM-CLEAR
 *
 * Note:
 *   TBD
 */
int cmd_eeprom_clear() {
  fill_eeprom_data(0);
  return 0;
}

/* Command : HELP
 *
 * Note:
 *   TBD
 */
int cmd_help() {
  Serial.println(F(
    "This is EVB-Pusher Arduino Nano/Nano_Every edition ver.0.0.1 .\n"
    "Capability:\n"
    "  IN  : GPI for digital input\n"
    "  PWM : PWM output\n"
    "  SW  : GPO for Switches\n"
    "  ADC : A/D/ converter input\n"
    "  SRV : Servo control output, x=0..5\n"
    "\n"
    "----------------------------\n"
    "EVB-Pusher command list\n"
    "----------------------------\n"
    "  PUSH <pin_name> [time] [repeat] [interval] : Do action to push the switche\n"
    "    [time] : If given integer N, hold on or off during N msec only.\n"
    "             If not given, use saved/default time.\n"
    "  PUSH2X <pin_name> [time] : Alias to PUSH for double click\n"
    "  PUSH3X <pin_name> [time] : Alias to PUSH for tripple click\n"
    "  PULL <pin_name> [time] [repeat] [interval] : Do action to pull the switche\n"
    "  ON <pin_name> : Do acton to turn ON the switche\n"
    "  OFF <pin_name> : Do acton to turn OFF the switche\n"
    "  DOWN <pin_name> [time] [repeat]\n"
    "    <port> : name of servo port (See w/ SERVO all list)\n"
    "        ex. pkey1 vol1\n"
    "    [time] :\n"
    "        If given integer N, hold down or up position during N msec only.\n"
    "        If given \"hold\", hold given position forever.\n"
    "        If not given, use default time (250msec).\n"
    "  UP <pin_name> [time] [repeat]\n"
    "  N <pin_name>\n"
    "  DUTY ,<pin_name> <duty> : Control PWM duty. Duty range is 0-100.\n"
    "  LIST : List up all controlable pin setting information\n"
    "  HELP : This command\n"
    "  SAVE [--with-debug] : Save tuning parameers into EEPROM especially servo\n"
    "  LOAD [--with-debug] : Reload tuning parameers from EEPROM especially servo\n"
    "  RESET: Reset all switch port and servo port\n"
    "  EEPROM-CLEAR: Clear all saved data on EEPROM\n"
    "\n"
    "--------------------------\n"
    "Debug config command list\n"
    "--------------------------\n"
    "  DEBUG=ON: Enable debug message\n"
    "  DEBUG=OFF: Disable debug message\n"
    "  ECHO=ON: Turn on echo back to serial console(for teraterm debugging)\n"
    "  ECHO=OFF: Turn off echo back to serial console\n"
    "\n"
  ) );
  return 0;
}

/*
 * Command parser
 */
void do_parse_cmd() {
  int ret = -1;
  cmdline_string.trim();
  CMD_DBG_PRINTLN(String(F("#CMD: raw cmdline = ")) + cmdline_string);
  // Parse to argc/argv
  for (argc = 0; argc < MAX_ARGC; argc++) {
    int idx_delim = cmdline_string.indexOf(' ');
    if (-1 == idx_delim) {
      argv[argc] = cmdline_string;
      CMD_DBG_PRINTLN(String(F("#CMD: argv[")) + argc + String(F("] = ")) + argv[argc]);
      argc++;
      break;
    }
    else {
      argv[argc] = cmdline_string.substring(0, idx_delim);
      cmdline_string = cmdline_string.substring(idx_delim + 1);
      cmdline_string.trim();
    }
    CMD_DBG_PRINTLN(String(F("#CMD: argv[")) + argc + String(F("] = ")) + argv[argc]);
  }
  CMD_DBG_PRINTLN(String(F("#CMD: argc = ")) + argc);
  // do command (Sorted by usage rate)
  argv[0].toUpperCase();
  if (String(F("PUSH")) == argv[0]) ret = cmd_push();
  else if (String(F("ON")) == argv[0]) ret = cmd_on();
  else if (String(F("OFF")) == argv[0]) ret = cmd_off();
  else if (String(F("PULL")) == argv[0]) ret = cmd_pull();

  else if (String(F("N")) == argv[0]) ret = cmd_off();      // N is same to OFF
  else if (String(F("DOWN")) == argv[0]) ret = cmd_push();  // DOWN is same to PUSH
  else if (String(F("UP")) == argv[0]) ret = cmd_up();      // UP is similar to PULL

  else if (String(F("PUSH2X")) == argv[0]) ret = cmd_push2x();  // Like a alias
  else if (String(F("PUSH3X")) == argv[0]) ret = cmd_push3x();  // Like a alias

  else if (String(F("DUTY")) == argv[0]) ret = cmd_duty();

  else if (String(F("I2CR")) == argv[0]) ret = cmd_i2cr();
  else if (String(F("I2CW")) == argv[0]) ret = cmd_i2cw();

  else if (String(F("LIST")) == argv[0]) ret = cmd_list();
  else if (String(F("STATUS")) == argv[0]) ret = cmd_status();

  else if (String(F("CONFIG")) == argv[0]) ret = cmd_config();
  else if (String(F("RENAME")) == argv[0]) ret = cmd_rename();

  else if (String(F("RESET")) == argv[0]) ret = cmd_reset();
  else if (String(F("HELP")) == argv[0]) ret = cmd_help();
  else if (String(F("LOAD")) == argv[0]) ret = cmd_load();
  else if (String(F("SAVE")) == argv[0]) ret = cmd_save();
  else if (String(F("EEPROM-CLEAR")) == argv[0]) ret = cmd_eeprom_clear();

  else if (String(F("DEBUG=ON")) == argv[0]) ret = cmd_debug_on();
  else if (String(F("DEBUG=OFF")) == argv[0]) ret = cmd_debug_off();
  else if (String(F("INITDEBUG=ON")) == argv[0]) ret = cmd_init_debug_on();
  else if (String(F("INITDEBUG=OFF")) == argv[0]) ret = cmd_init_debug_off();
  else if (String(F("CMDDEBUG=ON")) == argv[0]) ret = cmd_cmd_debug_on();
  else if (String(F("CMDDEBUG=OFF")) == argv[0]) ret = cmd_cmd_debug_off();
  else if (String(F("TIMDEBUG=ON")) == argv[0]) ret = cmd_tim_debug_on();
  else if (String(F("TIMDEBUG=OFF")) == argv[0]) ret = cmd_tim_debug_off();
  else if (String(F("I2CDEBUG=ON")) == argv[0]) ret = cmd_i2c_debug_on();
  else if (String(F("I2CDEBUG=OFF")) == argv[0]) ret = cmd_i2c_debug_off();
  else if (String(F("ECHO=ON")) == argv[0]) ret = cmd_echo_on();
  else if (String(F("ECHO=OFF")) == argv[0]) ret = cmd_echo_off();
  else {
    // Error
    last_cmd_error_mesg = F("Unknown command !?!?");
  }
  cmdline_string = "";

  if (ret) {
    Serial.print(F("[Err] "));
    Serial.println(last_cmd_error_mesg);
  }
  else {
    Serial.println(F("[OK]"));
  }
}

void proc_receive_cmdline() {
  static char inByte = 0;
  static char inByte_last = 0;

  if (Serial.available() > 0) {
    inByte = Serial.read();
    if (inByte_last == 0x0D && inByte == 0x0A) {
      ; // Do nothing for Windows return encoding
    } else if (inByte == 0x0A || inByte == 0x0D) {
      // CR or LF
      if (dbg_flg.flags.echo_enabled) {
        Serial.print(inByte);
      }
      do_parse_cmd();
    } else if ( (inByte == '\b') && dbg_flg.flags.echo_enabled) {
      // Backspace
      Serial.print(F("\b \b") ); // Trick for multi-byte char instead of single "\b"...
      if (cmdline_string.length() ) {
        cmdline_string.remove(cmdline_string.length() - 1);
      }
    } else {
      // add received byte to cmdline buffer;
      if (cmdline_string.length() < MAX_CMDLINE_LEN) {
        cmdline_string += inByte;
        if (dbg_flg.flags.echo_enabled) {
          Serial.print(inByte);
        }
      }
    }
    inByte_last = inByte;
  }
}

void setup_cmdline() {
  Serial.begin(115200); // default is 115200.
  INIT_PRINTLN(F("#INIT: Cmdline and serial port") );
}
