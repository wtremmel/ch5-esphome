
substitutions:
  rssi_present: id(harssi_present).state
  rssi_not_present: id(harssi_not_present).state

sensor:
  - platform: template
    id: apple_watch_rssi
    name: "Apple Watch $uroom RSSI"
    device_class: signal_strength
    unit_of_measurement: dBm
    accuracy_decimals: 0
    filters:
      - exponential_moving_average:
          alpha: 0.3
          send_every: 1
    on_value:
      then:
        - lambda: |-
            if (id(apple_watch_rssi).state > $rssi_present) {
              id(room_presence_debounce).publish_state(1);
            } else if (id(apple_watch_rssi).state < $rssi_not_present) {
              id(room_presence_debounce).publish_state(0);
            }
        - script.execute: presence_timeout  # Publish 0 if no rssi received
  - platform: template
    id: room_presence_debounce
    filters:
      - sliding_window_moving_average:
          window_size: 3
          send_every: 1
  - platform: homeassistant
    name: HA RSSI Present Value
    entity_id: input_number.rssi_present  #insert your input.number entity id here
    id: harssi_present
  - platform: homeassistant
    name: HA RSSI Not Present Value
    entity_id: input_number.rssi_not_present  #insert your input.number entity id here
    id: harssi_not_present

esp32_ble_tracker:
  scan_parameters:
    interval: 320ms
    window: 120ms
    active: false
  on_ble_advertise:
    - then:
      - lambda: |-
          optional<int16_t> best_rssi = nullopt;
          for (auto data : x.get_manufacturer_datas()) {
            // Guard against non-Apple datagrams, or those that are too small.
            if (data.data.size() < 4 || data.uuid.to_string() != "0x004C" || data.data[0] != 0x10 || data.data[1] < 5) {
              continue;
            }
            const int16_t rssi = x.get_rssi();
            const uint8_t status_flags = data.data[2] >> 4;  // High nibble
            const uint8_t data_flags = data.data[3];
            if (data_flags == 0x98) {  // Match unlocked Apple Watches
              if (status_flags == 0 || status_flags == 2) {
                best_rssi = max(rssi, best_rssi.value_or(rssi));
                // ESP_LOGD("ble_adv", "Found Apple Watch (mac %s), name %s, rssi %i", x.address_str().c_str(),x.get_name().c_str(), rssi);
              } else {
                // ESP_LOGD("ble_adv", "Possible Apple Watch? (mac %s), name %s, rssi %i, unrecognised status/action flags %#04x", x.address_str().c_str(),x.get_name().c_str(), rssi, data.data[2]);
              }
            }
          }
          if (best_rssi) {
            id(apple_watch_rssi).publish_state(*best_rssi);
          }

script:
  # Publish event every 30 seconds when no rssi received
  - id: presence_timeout
    mode: restart
    then:
      - delay: 30s
      - lambda: |-
          id(room_presence_debounce).publish_state(0);
      - script.execute: presence_timeout

binary_sensor:
  - platform: template
    id: room_presence
    name: "$uroom presence"
    device_class: occupancy
    lambda: |-
      if (id(room_presence_debounce).state > 0.99) {
        return true;
      } else if (id(room_presence_debounce).state < 0.01) {
        return false;
      } else {
        // return id(room_presence).state;
        return {};
      }

    

