bool type_change_to_void(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  DBG_PRINTLN(String(F("#VOID: Changed type : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

bool type_change_from_void(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  // Nothings to do VOID pin
  DBG_PRINTLN(String(F("#VOID: Un-init : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_void(struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SW_PUSH_DURATION;
}

void setup_void_pin() {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_VOID == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize VOID pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_void(&pin_ctrl_tbl[i]);
      do_sw_off(&pin_ctrl_tbl[i]);
    }
  }
}
