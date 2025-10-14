/*
 * Implementation for save/load to EEPROM
 */

/******************************************************************************
 *
 * EEPROM Save/Load implementation
 *
 * EEPROM usage : Based on NanoEvery 256Byte
 * +=====================+==============+=====================================+
 * | Contents            | Offset range | Note                                |
 * +=====================+==============+=====================================+
 * | header.crc          | +0..3        | unisgned long                       |
 * +---------------------+--------------+-------------------------------------+
 * | header.major        | +4           | uint8_t (Up to 255)                 |
 * +---------------------+--------------+-------------------------------------+
 * | header.minor        | +5           | uint8_t                             |
 * +---------------------+--------------+-------------------------------------+
 * | padding6            | +6           | uint8_t for 4B align tunable_tbl    |
 * +---------------------+--------------+-------------------------------------+
 * | debug_flags         | +7           | uint8_t                             |
 * +---------------------+--------------+-------------------------------------+
 * | tunable_tbl[0]      | +8..         | 6byte                               |
 * +---------------------+--------------+-------------------------------------+
 * | tunable_tbl[1]      | +14..        |                                     |
 * +---------------------+--------------+-------------------------------------+
 * |     ....            |              |                                     |
 * +---------------------+--------------+-------------------------------------+
 * | tunable_tbl[16]     | +104..109    |                                     |
 * +---------------------+--------------+-------------------------------------+
 * | pin_name_str_tbl[0] | +110..117    | Fixed 8byte strings                 |
 * +---------------------+--------------+-------------------------------------+
 * | pin_name_str_tbl[1] | +118..125    |                                     |
 * +---------------------+--------------+-------------------------------------+
 * |     ....            |              |                                     |
 * +---------------------+--------------+-------------------------------------+
 * | pin_name_str_tbl[16]| +238..245    |                                     |
 * +---------------------+--------------+-------------------------------------+
 * | (Reserved)          | ...255       | Remaining area                      |
 * +---------------------+--------------+-------------------------------------+
 * | (Prohibitted)       | +256..       | Nano only. Unused.                  |
 * +=====================+==============+=====================================+
 *
 *****************************************************************************/
#include <EEPROM.h>

/*
 * Data structure version
 *
 * Caution:
 *   We need revise number if structure changed.
 */
#define SAVE_DATA_FORMAT_VERSION_MAJOR   1
#define SAVE_DATA_FORMAT_VERSION_MINOR   0

struct eeprom_save_header {
  unsigned long crc;
  uint8_t major;   /// Save data format version major
  uint8_t minor;   /// Save data format version minor
};

struct eeprom_save {
  struct eeprom_save_header header;
  uint8_t padding6;
  uint8_t debug_flags;
  struct tunable_entry tunable_tbl[NR_PIN_CTRL_ENTRY];
  char pin_name_str_tbl [NR_PIN_CTRL_ENTRY][FIXED_PIN_LABEL_STR_LEN];
};
static_assert(sizeof(struct eeprom_save) <= 256, "struct eeprom_save is broken (Over size).");

unsigned long cal_eeprom_crc(void) {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  unsigned long crc = ~0L;

  for (int index = sizeof(struct eeprom_save_header) ; index < EEPROM_SAVE_REGION_LEN  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

int save_to_eeprom(bool with_debug_flag)
{
  // Save padding6
  int eeAddr = offsetof(eeprom_save, padding6);
  EEPROM.update(eeAddr, 0); // ZERO fill for padding

  // Save debug_flags
  eeAddr = offsetof(eeprom_save, debug_flags);
  if (with_debug_flag) {
    EEPROM.update(eeAddr, dbg_flg.body);
  }
  else {
    EEPROM.update(eeAddr, dbg_flg_boot.body);
  }
  
  // Save tunables
  eeAddr = offsetof(eeprom_save, tunable_tbl);
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    EEPROM.put(eeAddr, pin_ctrl_tbl[i].tunable);
    eeAddr += sizeof(struct tunable_entry);
  }

  // Save pin name
  eeAddr = offsetof(eeprom_save, pin_name_str_tbl);
  for (int row = 0; row < NR_PIN_CTRL_ENTRY; row++) {
    for (int col = 0; col < FIXED_PIN_LABEL_STR_LEN; col++) {
      EEPROM.update(eeAddr, pin_ctrl_tbl[row].name[col]);
      eeAddr += 1;
    }
  }

  // Create header
  struct eeprom_save_header saved_header;
  saved_header.major = SAVE_DATA_FORMAT_VERSION_MAJOR;
  saved_header.minor = SAVE_DATA_FORMAT_VERSION_MINOR;
  saved_header.crc = cal_eeprom_crc();
  EEPROM.put(0, saved_header);

  return 0;
}

/*
 * load tunables and renamed pin name
 */
int load_from_eeprom(bool with_debug_flag)
{
  struct tunable_entry loaded_tunable;
  struct eeprom_save_header saved_header;
  struct eeprom_save* ep = nullptr; // for offset calc
  
  // Check header
  int eeAddr = 0;
  EEPROM.get(eeAddr, saved_header);
  unsigned long crc = cal_eeprom_crc();
  if (crc != saved_header.crc) {
    Serial.println(F("#Warning! EEPROM : CRC not matched --> Skip loading.") );
    return 0;
  }
  if (SAVE_DATA_FORMAT_VERSION_MAJOR != saved_header.major) {
    Serial.println(F("#Warning! EEPROM : Save-Format version major not matched --> Skip loadig.") );
    return 0;
  }
  if (SAVE_DATA_FORMAT_VERSION_MINOR != saved_header.minor) {
    Serial.println(F("#Warning! EEPROM : Save-Format version minor not matched --> Skip loadig.") );
    return 0;
  }

  // Load debug flags, if required.
  if (with_debug_flag) {
    dbg_flg.body = EEPROM.read(offsetof(eeprom_save, debug_flags));
  }

  // Load renamed pin_name string
  eeAddr = offsetof(eeprom_save, pin_name_str_tbl);
  for (int row = 0; row < NR_PIN_CTRL_ENTRY; row++) {
    for (int col = 0; col < FIXED_PIN_LABEL_STR_LEN; col++) {
      pin_ctrl_tbl[row].name[col] = EEPROM.read(eeAddr);;
      eeAddr += 1;
    }
    pin_ctrl_tbl[row].name[FIXED_PIN_LABEL_STR_LEN + 1] = '\0'; // Force null termination
  }

  // Load calibrated variables
  eeAddr = offsetof(eeprom_save, tunable_tbl);
  for (int i = 0; i < NR_PIN_CTRL_ENTRY; i++) {
    EEPROM.get(eeAddr, loaded_tunable);
    eeAddr += sizeof(loaded_tunable);
    pin_ctrl_tbl[i].tunable = loaded_tunable;
  }
  // Print entire pin_ctrl_tbl once, if enabled.
  if (IS_INIT_DEBUG_ENABLED) {
    Serial.println(F("Load: pin_ctrl_tbl is loaded from EEPROM:"));
    cmd_list();
  }
}

/*
 * For erase entire saved data
 */
void fill_eeprom_data(uint8_t data) {
  for (int i = 0 ; i < EEPROM_SAVE_REGION_LEN ; i++) {
    EEPROM.update(i, data);
  }
}
