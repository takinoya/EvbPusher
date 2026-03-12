/*
    SPDX-License-Identifier: Apache-2.0
    SPDX-FileCopyrightText: 2026 Sakae OTAKI <niagara.ta.ki.no@gmail.com>
 */

/*
 * Implementation for CLI
 */
#define PUSHPULL_MODE_OFF   0
#define PUSHPULL_MODE_PUSH  1
#define PUSHPULL_MODE_PULL  3

/******************************************************************************
 * Command line intermediate function
 *****************************************************************************/
int _cmd_pushpull(struct sysvar_per_core_t * sys, int repeat, int duration, int interval, int mode)
{
  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  // Do push action by pin type
  if (PUSHPULL_MODE_PULL == mode) {  // PULL operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("PULL:SW"));
        do_sw_off(sys, pin_ctrl_ptr);      // For SW, OFF is equivalent to PULL.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("PULL:SERVO"));
        do_servo_up(sys, pin_ctrl_ptr);    // For SERVO, UP is wquivalent to PULL.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("PULL:PWM"));
        do_pwm_off(sys, pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("PULL:Unsupported"));
        sys->last_cmd_error_mesg = F("PULL: Unsupported for this pin_type !?!?");
        return -1;
        break;
    }
  }
  else if (PUSHPULL_MODE_PUSH == mode){  // PUSH operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("PUSH:SW"));
        do_sw_on(sys, pin_ctrl_ptr);
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("PUSH:SERVO"));
        do_servo_down(sys, pin_ctrl_ptr);    // For Serbo, DOWN operation is same to PUSH.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("PUSH:PWM"));
        do_pwm_on(sys, pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("PUSH:Unsupported"));
        sys->last_cmd_error_mesg = F("PUSH: Unsupported for this pin_type !?!?");
        return -1;
        break;
    }
  }
  else {  // OFF operation
    switch (pin_ctrl_ptr->tunable.type) {
      case PIN_TYPE_SW:
        CMD_DBG_PRINTLN(F("OFF:SW"));
        do_sw_off(sys, pin_ctrl_ptr);
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_SRV:
        CMD_DBG_PRINTLN(F("OFF:SERVO"));
        do_servo_n(sys, pin_ctrl_ptr);    // For SERVO, N is equivalent to OFF.
        if (0 > duration) {
          duration = pin_ctrl_ptr->tunable.duration;
        }
        if (0 > interval) {
          interval = pin_ctrl_ptr->tunable.duration;
        }
        break;
      case PIN_TYPE_PWM:
        CMD_DBG_PRINTLN(F("OFF:PWM"));
        do_pwm_off(sys, pin_ctrl_ptr);
        break;
      default:
        // Error
        CMD_DBG_PRINTLN(F("OFF:Unsupported"));
        sys->last_cmd_error_mesg = F("OFF: Unsupported for this pin_type !?!?");
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

void print_list_entry_header(struct sysvar_per_core_t * sys)
{
  sys->serial->println(F("|Pin|Name|Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|"));
  sys->serial->println(F("|:---|:---|:---|:---|:---|:---|:---|"));
}

void print_list_entry_common(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  sys->serial->print(F("|"));
  sys->serial->print(pin_ctrl_ptr->pin, DEC);
  sys->serial->print(F("|"));
  sys->serial->print(pin_ctrl_ptr->name);
  sys->serial->print(F("|"));
}

void print_list_entry_in(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->print(F("IN|"));
  sys->serial->print(pin_ctrl_ptr->tunable.duration, DEC);
  sys->serial->print(F("|"));
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.in.polarity) {
    sys->serial->println(F("ACTIVE_LO|-|-|"));
  }
  else {
    sys->serial->println(F("ACTIVE_HI|-|-|"));
  }
}

void print_list_entry_sw(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->print(F("SW|"));
  sys->serial->print(pin_ctrl_ptr->tunable.duration, DEC);
  sys->serial->print(F("|"));
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    sys->serial->println(F("ACTIVE_LO|-|-|"));
  }
  else {
    sys->serial->println(F("ACTIVE_HI|-|-|"));
  }
}

void print_list_entry_srv(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->print(F("SRV|"));
  sys->serial->print(pin_ctrl_ptr->tunable.duration, DEC);
  sys->serial->print(F("|"));
  sys->serial->print(pin_ctrl_ptr->tunable.srv.deg_dn, DEC);
  sys->serial->print(F("|"));
  sys->serial->print(pin_ctrl_ptr->tunable.srv.deg_n, DEC);
  sys->serial->print(F("|"));
  sys->serial->print(pin_ctrl_ptr->tunable.srv.deg_up, DEC);
  sys->serial->println(F("|"));
}

void print_list_entry_pwm(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->print(F("PWM|"));
  sys->serial->print(pin_ctrl_ptr->tunable.duration, DEC);
  sys->serial->print(F("|0x"));
  sys->serial->print(pin_ctrl_ptr->tunable.pwm.duty, HEX);
  sys->serial->println(F("|-|-|"));
}

void print_list_entry_adc(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->println(F("ADC|-|-|-|-|"));
}

void print_list_entry_void(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->println(F("VOID|-|-|-|-|"));
}

void print_list_entry_unknown(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // |Pin|Name|
  print_list_entry_common(sys, pin_ctrl_ptr);
  
  // Type|duration|deg_dn/Polarity/duty|deg_n|deg_up|
  sys->serial->println(F("???|-|-|-|-|"));
}

void print_list_entry(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      print_list_entry_in(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      print_list_entry_sw(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      print_list_entry_srv(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      print_list_entry_pwm(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      print_list_entry_adc(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_VOID:
      print_list_entry_void(sys, pin_ctrl_ptr);
      break;
    default:
      print_list_entry_unknown(sys, pin_ctrl_ptr);
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
int cmd_push(struct sysvar_per_core_t * sys) {
  int arg_repeat = 1;     // Default: 1 push
  int arg_duration = -1;  // Negative value means will be override default value
  int arg_interval = -1;  // Negative value means will be override default value

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  if (4 <= sys->argc) {
    // Has repeat
    arg_repeat = sys->argv[3].toInt();
  };
  if (5 <= sys->argc) {
    // Has off_duration
    arg_interval = sys->argv[4].toInt();
  };

  return _cmd_pushpull(sys, arg_repeat, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : PUSH2X <pin_label> [duration]
 *
 * Note:
 *   - Concept : Double click
 */
int cmd_push2x(struct sysvar_per_core_t * sys) {
  int arg_duration = -1;
  int arg_interval = -1;

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  if (4 <= sys->argc) {
    // Has off_duration
    arg_interval = sys->argv[3].toInt();
  };

  return _cmd_pushpull(sys, 2, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : PUSH3X <pin_label> [duration]
 *
 * Note:
 *   - Concept : Double click
 */
int cmd_push3x(struct sysvar_per_core_t * sys) {
  int arg_duration = -1;
  int arg_interval = -1;

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  if (4 <= sys->argc) {
    // Has interval
    arg_interval = sys->argv[3].toInt();
  };

  return _cmd_pushpull(sys, 3, arg_duration, arg_interval, PUSHPULL_MODE_PUSH);
};

/* Command : ON <pin_label> [duration]
 *
 * Note:
 *   Equivalent to PUSH
 *   - repeat   : Fixed 1
 *   - duration : Default - Infinite
 *   - interval : Do not care
 */
int cmd_on(struct sysvar_per_core_t * sys) {
  int arg_duration = 0;  // Default : Infinite

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  return _cmd_pushpull(sys, 1, arg_duration, 0, PUSHPULL_MODE_PUSH);  // Convert to infinite push once
}

/* Command : OFF <pin_label> [duration]
 *
 * Note:
 *   Equivalent to PUSH
 *   - repeat   : Fixed 1
 *   - duration : Default - Infinite
 *   - interval : Do not care
 */
int cmd_off(struct sysvar_per_core_t * sys) {
  int arg_duration = 0;  // Default : Infinite

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  return _cmd_pushpull(sys, 1, arg_duration, 0, PUSHPULL_MODE_OFF);  // Convert to infinite pull once
}

/* Command : PULL <pin_label> [duration] [[repeat] [interval]]
 *
 * Note:
 *   Equivalent to PUSH
 *   - is_pull : true (pull mode)
 */
int cmd_pull(struct sysvar_per_core_t * sys)
{
  int arg_repeat = 1;     // Default: 1 push
  int arg_duration = -1;  // Negative value means will be override default value
  int arg_interval = -1;  // Negative value means will be override default value

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  if (4 <= sys->argc) {
    // Has repeat
    arg_repeat = sys->argv[3].toInt();
  };
  if (5 <= sys->argc) {
    // Has off_duration
    arg_interval = sys->argv[4].toInt();
  };

  return _cmd_pushpull(sys, arg_repeat, arg_duration, arg_interval, PUSHPULL_MODE_PULL);
}

#if 0
/* Command : N <pin_label>
 *
 * Note:
 *   - SERVO only
 *   - Move and keep to N position
 */
int cmd_up(struct sysvar_per_core_t * sys)
{
  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  do_servo_n(sys, pin_ctrl_ptr);
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
int cmd_up(struct sysvar_per_core_t * sys) {
  int arg_duration = 0;  // Default : Infinite

  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }
  if (3 <= sys->argc) {
    // Has duration
    arg_duration = sys->argv[2].toInt();
  };
  return _cmd_pushpull(sys, 1, arg_duration, 0, PUSHPULL_MODE_PULL);  // Convert to infinite pull once
}

/* Command : DUTY <pin_label> <duty>
 *
 * Note:
 *   TBD
 */
int cmd_duty(struct sysvar_per_core_t * sys) {
  if (sys->argc < 3) {
    sys->last_cmd_error_mesg = F("At least 3 arguments required !?!?");
    return -1;
  }
  int duty = sys->argv[2].toInt();
  if ((duty < 0) || (100 < duty)) {
    sys->last_cmd_error_mesg = F("Range error (should 0-100) !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  do_pwm_set_duty(sys, pin_ctrl_ptr, duty);

  return 0;
}

/* Command : I2CR
 *
 * Note:
 *   - TBD
 */
int cmd_i2cr(struct sysvar_per_core_t * sys) {
  return 0; // To be implemented
}

/* Command : I2CW
 *
 * Note:
 *   - TBD
 */
int cmd_i2cw(struct sysvar_per_core_t * sys) {
  return 0; // To be implemented
}

/* Command : LIST
 *
 * Note:
 *   - Print pin control settings as Markdown table.
 */
int cmd_list(struct sysvar_per_core_t * sys) {
  print_list_entry_header(sys);

  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    print_list_entry(sys, &pin_ctrl_tbl[i]);
  }
  return 0;
}

/* Command : STATUS <pin_label>
 *
 * Note:
 *   - ToDo
 */
int cmd_status(struct sysvar_per_core_t * sys) {
  if (sys->argc < 2) {
    sys->last_cmd_error_mesg = F("At least 1 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }

  // branch per pin type
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      print_status_in(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      print_status_sw(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      print_status_servo(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      print_status_pwm(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      print_status_adc(sys, pin_ctrl_ptr);
      break;
    default:
      sys->last_cmd_error_mesg = F("Unsupported pin type !?!?");
      return -1;
      break;
  }
  return 0;
}

bool type_change_to(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr, String& typeStr)
{
  bool (*type_change_func)(sysvar_per_core_t*, struct pin_ctrl_entry_t* pin_ctrl_ptr) = nullptr;
  sys->argv[3].toUpperCase();
  if (String(F("VOID")) == sys->argv[3]) {
    type_change_func = type_change_to_void;
  }
  else if (String(F("ADC")) == sys->argv[3]) {
    type_change_func = type_change_to_adc;
  }
  else if (String(F("IN")) == sys->argv[3]) {
    type_change_func = type_change_to_in;
  }
  else if (String(F("SW")) == sys->argv[3]) {
    type_change_func = type_change_to_sw;
  }
  else if (String(F("SRV")) == sys->argv[3]) {
    type_change_func = type_change_to_srv;
  }
  else if (String(F("PWM")) == sys->argv[3]) {
    type_change_func = type_change_to_pwm;
  }
  else {
    sys->last_cmd_error_mesg = F("Unknown pin type !?!?");
    return -1;
  }
  // Pre operation per pin type
  switch (pin_ctrl_ptr->tunable.type) {
    case PIN_TYPE_IN:
      type_change_from_in(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SW:
      type_change_from_sw(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_SRV:
      type_change_from_srv(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_PWM:
      type_change_from_pwm(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_ADC:
      type_change_from_adc(sys, pin_ctrl_ptr);
      break;
    case PIN_TYPE_VOID:
      type_change_from_void(sys, pin_ctrl_ptr);
      break;
    default:
      sys->last_cmd_error_mesg = F("Unexpected error !?!?");
      return -1;
      break;
  }
  
  // Do it with check
  return type_change_func(sys, pin_ctrl_ptr);
}

/* Command : CONFIG <pin_label> [<key> <value>]
 *
 * Note:
 *   - ToDo
 */
int cmd_config(struct sysvar_per_core_t * sys) {
  bool is_key_value_mode = true;

  if (sys->argc == 2) {
    is_key_value_mode = false;
  }
  else if (sys->argc < 4) {
    sys->last_cmd_error_mesg = F("At least 1 or 3 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }
  
  if (is_key_value_mode) {
    // <key> <value> operation
    if (String(F("type")) == sys->argv[2]) {
      if (false == type_change_to(sys, pin_ctrl_ptr, sys->argv[3]) ) {
        // sys->last_cmd_error_mesg is already stored in type_chage_to_*()
        return -1;
      }
    }
    else if (String(F("duration")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.duration = sys->argv[3].toInt();
    }
    else if (String(F("polarity")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.sw.polarity = sys->argv[3].toInt();
    }
    else if (String(F("deg_dn")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_dn = sys->argv[3].toInt();
    }
    else if (String(F("deg_n")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_n = sys->argv[3].toInt();
    }
    else if (String(F("deg_up")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.srv.deg_up = sys->argv[3].toInt();
    }
    else if (String(F("duty")) == sys->argv[2]) {
      pin_ctrl_ptr->tunable.pwm.duty = sys->argv[3].toInt();
    }
    else {
      sys->last_cmd_error_mesg = F("Unknown key !?!?");
      return -1;
    }
  }
  // Print result
  print_list_entry_header(sys);
  print_list_entry(sys, pin_ctrl_ptr);
  return 0;
}

/* Command : RENAME <pin_label> <new_pin_label>
 *
 * Note:
 *   - ToDo
 */
int cmd_rename(struct sysvar_per_core_t * sys) {
  if (sys->argc < 3) {
    sys->last_cmd_error_mesg = F("At least 2 arguments required !?!?");
    return -1;
  }

  // Determine SW Port
  struct pin_ctrl_entry_t* pin_ctrl_ptr = search_pin_ctrl_entry_by_name(sys->argv[1]);
  if (pin_ctrl_ptr == nullptr) {
    // Error
    sys->last_cmd_error_mesg = F("Unknown pin name !?!?");
    return -1;
  }
  // Do rename
  strlcpy(pin_ctrl_ptr->name, sys->argv[2].c_str(), FIXED_PIN_LABEL_STR_LEN + 1);
  return 0;
}

/* Command : RESET
 *
 * Note:
 *   - Use WDT to reset
 */
#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_NANO_EVERY)

#include <avr/wdt.h>

int cmd_reset(struct sysvar_per_core_t * sys) {
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

#elif defined(ARDUINO_SEEED_XIAO_RP2040) // ...#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_NANO_EVERY)

int cmd_reset(struct sysvar_per_core_t * sys) {
  INIT_PRINTLN(F("#INIT: Call HW reset."));
  rp2040.reboot();
  return 0;
}

#endif // ...#if defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_NANO_EVERY)

/*
 * Debug commands
 */

/* Command : DEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on debug message.") );
  dbg_flg.flags.debug_enabled = 1;  // true
  return 0;
}

/* Command : DEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off debug message.") );
  dbg_flg.flags.debug_enabled = 0;  // false
  return 0;
}

/* Command : INITDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_init_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on init debug message.") );
  dbg_flg.flags.init_debug_enabled = 1;  // true
  return 0;
}

/* Command : INITDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_init_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off init debug message.") );
  dbg_flg.flags.init_debug_enabled = 0;  // false
  return 0;
}

/* Command : CMDDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_cmd_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on cmdline debug message.") );
  IS_CMD_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : CMDDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_cmd_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off cmdline debug message.") );
  IS_CMD_DEBUG_ENABLED = 0;  // false
  return 0;
}

/* Command : TIMDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_tim_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on timing debug message.") );
  IS_TIM_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : TIMDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_tim_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off timing debug message.") );
  IS_TIM_DEBUG_ENABLED = 0;  // false
  return 0;
}

/* Command : I2CDEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_i2c_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on I2C debug message.") );
  IS_I2C_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : I2CDEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_i2c_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off I2C debug message.") );
  IS_I2C_DEBUG_ENABLED = 0;  // false
  return 0;
}
#if 0
/* Command : *DEBUG=ON
 *
 * Note:
 *   TBD
 */
int cmd_*_debug_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on * debug message.") );
  IS_*_DEBUG_ENABLED = 1;  // true
  return 0;
}

/* Command : *DEBUG=OFF
 *
 * Note:
 *   TBD
 */
int cmd_*_debug_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off * debug message.") );
  IS_*_DEBUG_ENABLED = 0;  // false
  return 0;
}
#endif //#if 0

/* Command : ECHO=ON
 *
 * Note:
 *   TBD
 */
int cmd_echo_on(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn on echo back to serial.") );
  dbg_flg.flags.echo_enabled = 1; // true
  return 0;
}

/* Command : ECHO=OFF
 *
 * Note:
 *   TBD
 */
int cmd_echo_off(struct sysvar_per_core_t * sys) {
  sys->serial->println(F("Turn off echo back to serial.") );
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
int cmd_load(struct sysvar_per_core_t * sys) {
  if (2 <= sys->argc) {
    // Has option
    if (String(F("--with-debug")) == sys->argv[1]) {
      CMD_DBG_PRINTLN("#DBG: LOAD : load with debug flags\n");
      return load_from_eeprom(sys, true);
    }
    else {
      CMD_DBG_PRINTLN("#DBG: LOAD : Skip unknown option\n");
    }
  }
  return load_from_eeprom(sys, false);
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
int cmd_save(struct sysvar_per_core_t * sys) {
  if (2 <= sys->argc) {
    // Has option
    if (String(F("--with-debug")) == sys->argv[1]) {
      CMD_DBG_PRINTLN("#DBG: SAVE : save with debug flags\n");
      return save_to_eeprom(sys, true);
    }
    else {
      CMD_DBG_PRINTLN("#DBG: SAVE : Unknown option\n");
    }
  }
  return save_to_eeprom(sys, false);
}

/* Command : EEPROM-CLEAR
 *
 * Note:
 *   TBD
 */
int cmd_eeprom_clear(struct sysvar_per_core_t * sys) {
  fill_eeprom_data(sys, 0);
  return 0;
}

/* Command : HELP
 *
 * Note:
 *   TBD
 */
int cmd_help(struct sysvar_per_core_t * sys) {
  sys->serial->println(F(
    "This is EVB-Pusher firmware.\n"
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
void do_parse_cmd(struct sysvar_per_core_t * sys) {
  int ret = -1;
  sys->cmdline_string.trim();
  CMD_DBG_PRINTLN(String(F("#CMD: raw cmdline = ")) + sys->cmdline_string);
  // Parse to argc/argv
  for (sys->argc = 0; sys->argc < MAX_ARGC; sys->argc++) {
    int idx_delim = sys->cmdline_string.indexOf(' ');
    if (-1 == idx_delim) {
      sys->argv[sys->argc] = sys->cmdline_string;
      CMD_DBG_PRINTLN(String(F("#CMD: argv[")) + sys->argc + String(F("] = ")) + sys->argv[sys->argc]);
      sys->argc++;
      break;
    }
    else {
      sys->argv[sys->argc] = sys->cmdline_string.substring(0, idx_delim);
      sys->cmdline_string = sys->cmdline_string.substring(idx_delim + 1);
      sys->cmdline_string.trim();
    }
    CMD_DBG_PRINTLN(String(F("#CMD: argv[")) + sys->argc + String(F("] = ")) + sys->argv[sys->argc]);
  }
  CMD_DBG_PRINTLN(String(F("#CMD: argc = ")) + sys->argc);
  // do command (Sorted by usage rate)
  sys->argv[0].toUpperCase();
  if (String(F("PUSH")) == sys->argv[0]) ret = cmd_push(sys);
  else if (String(F("ON")) == sys->argv[0]) ret = cmd_on(sys);
  else if (String(F("OFF")) == sys->argv[0]) ret = cmd_off(sys);
  else if (String(F("PULL")) == sys->argv[0]) ret = cmd_pull(sys);

  else if (String(F("N")) == sys->argv[0]) ret = cmd_off(sys);      // N is same to OFF
  else if (String(F("DOWN")) == sys->argv[0]) ret = cmd_push(sys);  // DOWN is same to PUSH
  else if (String(F("UP")) == sys->argv[0]) ret = cmd_up(sys);      // UP is similar to PULL

  else if (String(F("PUSH2X")) == sys->argv[0]) ret = cmd_push2x(sys);  // Like a alias
  else if (String(F("PUSH3X")) == sys->argv[0]) ret = cmd_push3x(sys);  // Like a alias

  else if (String(F("DUTY")) == sys->argv[0]) ret = cmd_duty(sys);

  else if (String(F("I2CR")) == sys->argv[0]) ret = cmd_i2cr(sys);
  else if (String(F("I2CW")) == sys->argv[0]) ret = cmd_i2cw(sys);

  else if (String(F("LIST")) == sys->argv[0]) ret = cmd_list(sys);
  else if (String(F("STATUS")) == sys->argv[0]) ret = cmd_status(sys);

  else if (String(F("CONFIG")) == sys->argv[0]) ret = cmd_config(sys);
  else if (String(F("RENAME")) == sys->argv[0]) ret = cmd_rename(sys);

  else if (String(F("RESET")) == sys->argv[0]) ret = cmd_reset(sys);
  else if (String(F("HELP")) == sys->argv[0]) ret = cmd_help(sys);
  else if (String(F("LOAD")) == sys->argv[0]) ret = cmd_load(sys);
  else if (String(F("SAVE")) == sys->argv[0]) ret = cmd_save(sys);
  else if (String(F("EEPROM-CLEAR")) == sys->argv[0]) ret = cmd_eeprom_clear(sys);

  else if (String(F("DEBUG=ON")) == sys->argv[0]) ret = cmd_debug_on(sys);
  else if (String(F("DEBUG=OFF")) == sys->argv[0]) ret = cmd_debug_off(sys);
  else if (String(F("INITDEBUG=ON")) == sys->argv[0]) ret = cmd_init_debug_on(sys);
  else if (String(F("INITDEBUG=OFF")) == sys->argv[0]) ret = cmd_init_debug_off(sys);
  else if (String(F("CMDDEBUG=ON")) == sys->argv[0]) ret = cmd_cmd_debug_on(sys);
  else if (String(F("CMDDEBUG=OFF")) == sys->argv[0]) ret = cmd_cmd_debug_off(sys);
  else if (String(F("TIMDEBUG=ON")) == sys->argv[0]) ret = cmd_tim_debug_on(sys);
  else if (String(F("TIMDEBUG=OFF")) == sys->argv[0]) ret = cmd_tim_debug_off(sys);
  else if (String(F("I2CDEBUG=ON")) == sys->argv[0]) ret = cmd_i2c_debug_on(sys);
  else if (String(F("I2CDEBUG=OFF")) == sys->argv[0]) ret = cmd_i2c_debug_off(sys);
  else if (String(F("ECHO=ON")) == sys->argv[0]) ret = cmd_echo_on(sys);
  else if (String(F("ECHO=OFF")) == sys->argv[0]) ret = cmd_echo_off(sys);
  else {
    // Error
    sys->last_cmd_error_mesg = F("Unknown command !?!?");
  }
  sys->cmdline_string = "";

  if (ret) {
    sys->serial->print(F("[Err] "));
    sys->serial->println(sys->last_cmd_error_mesg);
  }
  else {
    sys->serial->println(F("[OK]"));
  }
}

void proc_receive_cmdline(struct sysvar_per_core_t * sys) {
  static char inByte = 0;
  static char inByte_last = 0;

  if (sys->serial->available() > 0) {
    inByte = sys->serial->read();
    if (inByte_last == 0x0D && inByte == 0x0A) {
      ; // Do nothing for Windows return encoding
    } else if (inByte == 0x0A || inByte == 0x0D) {
      // CR or LF
      if (dbg_flg.flags.echo_enabled) {
        sys->serial->print(inByte);
      }
      do_parse_cmd(sys);
    } else if ( (inByte == '\b') && dbg_flg.flags.echo_enabled) {
      // Backspace
      sys->serial->print(F("\b \b") ); // Trick for multi-byte char instead of single "\b"...
      if (sys->cmdline_string.length() ) {
        sys->cmdline_string.remove(sys->cmdline_string.length() - 1);
      }
    } else {
      // add received byte to cmdline buffer;
      if (sys->cmdline_string.length() < MAX_CMDLINE_LEN) {
        sys->cmdline_string += inByte;
        if (dbg_flg.flags.echo_enabled) {
          sys->serial->print(inByte);
        }
      }
    }
    inByte_last = inByte;
  }
}

void setup_cmdline(struct sysvar_per_core_t * sys) {
  sys->serial->begin(115200); // default is 115200.
  while (!sys->serial) {
    ;
  }
  INIT_PRINTLN(F("#INIT: Cmdline and serial port") );
}
