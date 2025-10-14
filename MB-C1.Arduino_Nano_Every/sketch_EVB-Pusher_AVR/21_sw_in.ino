/*
 * Implementation for Digital Input/Output pin
 */

/*
 * SW: Switch operation implementation
 */
void do_sw_on(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    digitalWrite(pin_ctrl_ptr->pin, LOW);
  } else {
    digitalWrite(pin_ctrl_ptr->pin, HIGH);
  }
  DBG_PRINTLN(String(F("#SW: SW On : ")) + pin_ctrl_ptr->name);
}

void do_sw_off(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    digitalWrite(pin_ctrl_ptr->pin, HIGH);
  } else {
    digitalWrite(pin_ctrl_ptr->pin, LOW);
  }
  DBG_PRINTLN(String(F("#SW: SW Off : ")) + pin_ctrl_ptr->name);
}

void do_sw_toggle(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  int toggled;

  if (digitalRead(pin_ctrl_ptr->pin) == HIGH ) {
      toggled = LOW;
  } else {
      toggled = HIGH;
  }
  digitalWrite(pin_ctrl_ptr->pin, toggled);
  DBG_PRINTLN(String(F("#SW: SW Toggle : ")) + pin_ctrl_ptr->name);
}

/*
 * SW: For LIST/STATUS command
 */
void print_status_sw(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  int pin_stat = digitalRead(pin_ctrl_ptr->pin);
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    Serial.println( (pin_stat==HIGH)? F("OFF,1") : F("ON,0") );
  }
  else {
    Serial.println( (pin_stat==HIGH)? F("ON,1") : F("OFF,0") );
  }
}

/*
 * SW: Initialize/Configuration implementation
 */
bool type_change_to_sw(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  for (int i=0; i < sizeof(pin_black_list_digital)/sizeof(pin_black_list_digital[0]); i++) {
    if (pin_black_list_digital[i] == pin_ctrl_ptr->pin) {
      // In black list : NG
      last_cmd_error_mesg = F("The pin is in Black-list !?!?");
      return false;
    }
  }
  pinMode(pin_ctrl_ptr->pin, OUTPUT);
  // Set INACTIVE
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    digitalWrite(pin_ctrl_ptr->pin, HIGH);
  } else {
    digitalWrite(pin_ctrl_ptr->pin, LOW);
  }
  pin_ctrl_ptr->tunable.type = PIN_TYPE_SW;
  DBG_PRINTLN(String(F("#SW: Changed type to SW : ")) + pin_ctrl_ptr->name);
}

bool type_change_from_sw(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  do_sw_off(pin_ctrl_ptr);
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  pinMode(pin_ctrl_ptr->pin, INPUT);
  DBG_PRINTLN(String(F("#SW: Un-init SW : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_sw(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SW_PUSH_DURATION;
  pin_ctrl_ptr->tunable.sw.polarity = PIN_POL_ACTIVE_HI;
}

void setup_sw() {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_SW == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize SW pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_sw(&pin_ctrl_tbl[i]);
      do_sw_off(&pin_ctrl_tbl[i]);
    }
  }
}

/*
 * IN: For LIST/STATUS command
 */
void print_status_in(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  int pin_stat = digitalRead(pin_ctrl_ptr->pin);
  if (PIN_POL_ACTIVE_LO == pin_ctrl_ptr->tunable.sw.polarity) {
    Serial.println( (pin_stat==HIGH)? F("OFF,1") : F("ON,0") );
  }
  else {
    Serial.println( (pin_stat==HIGH)? F("ON,1") : F("OFF,0") );
  }
}

/*
 * IN: Initialize/Configuration
 */
bool type_change_to_in(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  for (int i=0; i < sizeof(pin_black_list_digital)/sizeof(pin_black_list_digital[0]); i++) {
    if (pin_black_list_digital[i] == pin_ctrl_ptr->pin) {
      // In black list : NG
      last_cmd_error_mesg = F("The pin is in Black-list !?!?");
      return false;
    }
  }
  pinMode(pin_ctrl_ptr->pin, INPUT);
  pin_ctrl_ptr->tunable.type = PIN_TYPE_IN;
  DBG_PRINTLN(String(F("#IN: Changed type to IN : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

bool type_change_from_in(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  DBG_PRINTLN(String(F("#IN: Un-init IN : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_in(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SW_PUSH_DURATION;
  pin_ctrl_ptr->tunable.sw.polarity = PIN_POL_ACTIVE_HI;
}

void setup_in() {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_IN == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize IN pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_in(&pin_ctrl_tbl[i]);
      do_sw_off(&pin_ctrl_tbl[i]);
    }
  }
}
