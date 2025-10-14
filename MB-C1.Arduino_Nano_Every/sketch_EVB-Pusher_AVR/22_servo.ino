/*
 * Implementation for SERVO
 */
#include <Servo.h>

Servo servo_obj[NR_SERVO_INSTANCE];

void do_servo_down(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  pin_ctrl_ptr->srv_obj->write(pin_ctrl_ptr->tunable.srv.deg_dn);  // Set down position
  pin_ctrl_ptr->hold_value = pin_ctrl_ptr->tunable.srv.deg_dn;
  DBG_PRINTLN(String(F("#SERVO: Down : ")) + pin_ctrl_ptr->name);
}

void do_servo_up(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  pin_ctrl_ptr->srv_obj->write(pin_ctrl_ptr->tunable.srv.deg_up);  // Set up position
  pin_ctrl_ptr->hold_value = pin_ctrl_ptr->tunable.srv.deg_up;
  DBG_PRINTLN(String(F("#SERVO: Up : ")) + pin_ctrl_ptr->name);
}

void do_servo_n(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  pin_ctrl_ptr->srv_obj->write(pin_ctrl_ptr->tunable.srv.deg_n);   // Set nutral position
  DBG_PRINTLN(String(F("#SERVO: N : ")) + pin_ctrl_ptr->name);
}

void do_servo_toggle(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  int d = pin_ctrl_ptr->srv_obj->read();
  if (d == pin_ctrl_ptr->tunable.srv.deg_n) {
    pin_ctrl_ptr->srv_obj->write(pin_ctrl_ptr->hold_value); // Set push position
  }
  else {
    pin_ctrl_ptr->srv_obj->write(pin_ctrl_ptr->tunable.srv.deg_n);  // Set nutral position
  }
  DBG_PRINTLN(String(F("#SERVO: SERVO Toggle : ")) + pin_ctrl_ptr->name);
}

void print_status_servo(struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  Serial.print(pin_ctrl_ptr->srv_obj->read(), DEC);
  Serial.print(F(",dn="));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_dn, DEC);
  Serial.print(F(",n="));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_n, DEC);
  Serial.print(F(",up="));
  Serial.print(pin_ctrl_ptr->tunable.srv.deg_up, DEC);
  Serial.print(F(",duration="));
  Serial.println(pin_ctrl_ptr->tunable.duration, DEC);
}

Servo* search_vacant_servo_instance()
{
  // Confirm vacancy
  for (int i = 0; i < NR_SERVO_INSTANCE; i++) {
    if (servo_obj[i].attached()) {
      DBG_PRINTLN(String(F("#SERVO: Occupied : ")) + i);
    }
    else {
      DBG_PRINTLN(String(F("#SERVO: Vacant : ")) + i);
      return &servo_obj[i];
    }
  }
  DBG_PRINTLN(F("#SERVO: No vacant srv_obj!?"));
  return nullptr;
}

bool type_change_to_srv(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  for (int i=0; i < sizeof(pin_black_list_digital)/sizeof(pin_black_list_digital[0]); i++) {
    if (pin_black_list_digital[i] == pin_ctrl_ptr->pin) {
      // In black list : NG
      last_cmd_error_mesg = F("Black listed pin !?!?");
      return false;
    }
  }
  Servo* vacant_srv_obj = search_vacant_servo_instance();
  if (nullptr == vacant_srv_obj) {
    // No vacant instance : NG
    last_cmd_error_mesg = F("No vacant Servo instance !?!?");
    return false;
  }
  pin_ctrl_ptr->tunable.type = PIN_TYPE_SRV;
  pin_ctrl_ptr->srv_obj = vacant_srv_obj;
  pin_ctrl_ptr->srv_obj->attach(pin_ctrl_ptr->pin);
  do_servo_n(pin_ctrl_ptr);
  DBG_PRINTLN(String(F("#SERVO: Changed to SRV : ")) + pin_ctrl_ptr->name);
  return true;
}

bool type_change_from_srv(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->srv_obj->detach();
  pin_ctrl_ptr->srv_obj = nullptr;
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  pinMode(pin_ctrl_ptr->pin, INPUT);
  DBG_PRINTLN(String(F("#SERVO: Un-init SRV : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_servo(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SERVO_HOLD_TIME;
  pin_ctrl_ptr->tunable.srv.deg_dn = 110; // degree 0..180
  pin_ctrl_ptr->tunable.srv.deg_n  = 90;  // degree 0..180
  pin_ctrl_ptr->tunable.srv.deg_up = 70;  // degree 0..180
}


void setup_servo() {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_SRV == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize SRV pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_srv(&pin_ctrl_tbl[i]);
    }
  }
}
