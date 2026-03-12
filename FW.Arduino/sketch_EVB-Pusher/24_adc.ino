/*
    SPDX-License-Identifier: Apache-2.0
    SPDX-FileCopyrightText: 2026 Sakae OTAKI <niagara.ta.ki.no@gmail.com>
 */

/*
 * Implementation for ADC
 */
void print_status_adc(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr) {
  sys->serial->print("0x");
  sys->serial->println(analogRead(pin_ctrl_ptr->pin), HEX);
}

bool type_change_to_adc(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  for (int i=0; i < sizeof(pin_white_list_adc)/sizeof(pin_white_list_adc[0]); i++) {
    if (pin_white_list_adc[i] == pin_ctrl_ptr->pin) {
      pin_ctrl_ptr->tunable.type = PIN_TYPE_ADC;
      analogRead(pin_ctrl_ptr->pin);  // Read once to setup
      DBG_PRINTLN(String(F("#ADC: Changed : ")) + pin_ctrl_ptr->name);
      return true;  // Success
    }
  }
  DBG_PRINTLN(String(F("#ADC: Failed change : ")) + pin_ctrl_ptr->name);
  sys->last_cmd_error_mesg = F("The pin is not in white-list !?!?");
  return false;
}

bool type_change_from_adc(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.type = PIN_TYPE_VOID;
  DBG_PRINTLN(String(F("#ADC: Un-init : ")) + pin_ctrl_ptr->name);
  return true;  // Success
}

void factory_reset_tunables_adc(struct sysvar_per_core_t * sys, struct pin_ctrl_entry_t* pin_ctrl_ptr)
{
  pin_ctrl_ptr->tunable.duration = DEFAULT_SW_PUSH_DURATION;
}

void setup_adc(struct sysvar_per_core_t * sys) {
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    if (PIN_TYPE_ADC == pin_ctrl_tbl[i].tunable.type) {
      INIT_PRINT(F("#Init: Initialize ADC pin ") );
      INIT_PRINT(pin_ctrl_tbl[i].name);
      INIT_PRINT(F(", Pin = ") );
      INIT_PRINTLN(pin_ctrl_tbl[i].pin, DEC);
      type_change_to_adc(sys, &pin_ctrl_tbl[i]);
    }
  }
}
