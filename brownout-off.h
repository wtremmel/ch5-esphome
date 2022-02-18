#include "esphome.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

class NoBrownout : public Component {
 public:
  void setup() override {
    // This will be called once to set up the component
    // think of it as the setup() call in Arduino
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  }
  void loop() override {
  }
};
