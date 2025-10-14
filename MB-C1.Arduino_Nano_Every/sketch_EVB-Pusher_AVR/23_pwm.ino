/*
 * Implementation for PWM
 */
void do_pwm_on(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  analogWrite(pin_ctrl_ptr->pin, pin_ctrl_ptr->tunable.pwm.duty);
  pin_ctrl_ptr->hold_value = pin_ctrl_ptr->tunable.pwm.duty;
  DBG_PRINTLN(String(F("#PWM: On : ")) + pin_ctrl_ptr->name);
}

void do_pwm_off(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  analogWrite(pin_ctrl_ptr->pin, 0);
  pin_ctrl_ptr->hold_value = 0;
  DBG_PRINTLN(String(F("#PWM: Off : ")) + pin_ctrl_ptr->name);
}

void do_pwm_toggle(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  if (pin_ctrl_ptr->hold_value) {
    do_pwm_off(pin_ctrl_ptr);
  }
  else {
    do_pwm_on(pin_ctrl_ptr);
  }
  DBG_PRINTLN(String(F("#PWM: Toggle : ")) + pin_ctrl_ptr->name);
}

void do_pwm_set_duty(struct pin_ctrl_entry_t* pin_ctrl_ptr, int percent_duty) {
  // Scaling 0-100 to 0-255
  int duty = percent_duty * 255;
  duty = duty / 100;
  // Satulation
  if (duty < 0) duty = 0;
  if (255 < duty) duty = 255;
  // Apply
  pin_ctrl_ptr->tunable.pwm.duty = duty;
  DBG_PRINTLN(String(F("#PWM: PWM raw duty = ")) + duty);
  DBG_PRINTLN(String(F("#PWM: Set duty : ")) + pin_ctrl_ptr->name);

  // Update output, if required.
  if (pin_ctrl_ptr->cdn_toggle) {
    analogWrite(pin_ctrl_ptr->pin, pin_ctrl_ptr->hold_value);
  }
}

void print_status_pwm(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  Serial.print("0x");
  Serial.println(pin_ctrl_ptr->tunable.pwm.duty, HEX);
}

bool type_change_to_pwm(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  for (int i=0; i < sizeof(pin_white_list_pwm)/sizeof(pin_white_list_pwm[0]); i++) {
    if (pin_white_list_pwm[i] == pin_ctrl_ptr->pin) {
      pin_ctrl_ptr->tunable.type = PIN_TYPE_PWM;
      pinMode(pin_ctrl_ptr->pin, OUTPUT);
      do_pwm_off(pin_ctrl_ptr);
      DBG_PRINTLN(String(F("#PWM: Changed : ")) + pin_ctrl_ptr->name);
      return true;  // Success
    }
  }
  DBG_PRINTLN(String(F("#PWM: Failed change : ")) + pin_ctrl_ptr->name);
  last_cmd_error_mesg = F("The pin is not in white-list !?!?");
  return false;
}

bool type_change_from_pwm(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  do_pwm_off(pin_ctrl_ptr);
  pinMode(pin_ctrl_ptr->pin, INPUT);
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  DBG_PRINTLN(String(F("#PWM: Un-init : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_pwm(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SW_PUSH_DURATION;
  pin_ctrl_ptr->tunable.pwm.duty = 0;
}

void setup_pwm() {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_PWM == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize PWM pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_pwm(&pin_ctrl_tbl[i]);
    }
  }
}
