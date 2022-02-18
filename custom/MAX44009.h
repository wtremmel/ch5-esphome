#include "esphome.h"
#include "Max44009.h"

using namespace esphome;

class MAX44009 : public sensor::PollingSensorComponent {
 public:
    Max44009 tsl = Max44009(0x4A, 21, 22); //these are the i2c pins being used

  MAX44009(const std::string &name, uint32_t update_interval) : sensor::PollingSensorComponent(
    name, update_interval) {}
  
  void update() override {
    float lux = tsl.getLux(); // in lux
    int err = tsl.getError();
    if(err) 
      ESP_LOGI("luxmeter", "Got error code: %1i", err);  
    else   
      //ESP_LOGD(TAG, "Got lux=%.1flux", lux);   
      publish_state(lux); 
  }

   std::string icon() override {
    return "mdi:theme-light-dark";
  }

    std::string unit_of_measurement() override { return "Lux"; }
  int8_t accuracy_decimals() override { return 2; } // 2 decimal places of accuracy.
};

