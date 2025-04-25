// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <memory>
#include <functional>

#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp_app_builder.h"
#include "linear.h"
#include "radians.h"

using namespace sensesp;

using xyPair = std::pair<float, float>;


/*class RudderAngleSensor {
  public:
    RudderAngleSensor() {}

  private:
  AnalogInput
}*/

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("sensesp-rudder-angle-sensor")
                    // Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi_client("My WiFi SSID", "my_wifi_password")
                    //->set_wifi_access_point("My AP SSID", "my_ap_password")
                    //->set_sk_server("192.168.10.3", 80)
                    ->get_app();

  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html#vesselsregexpsteeringrudderangle
  const char* sk_path = "steering.rudderAngle";
  const char* kUIGroup ="Rudder Angle Sensor";

  // GPIO number to use for the analog input
  const uint8_t kAnalogInputPin = 36;
  // Define how often (in milliseconds) new samples are acquired
  const unsigned int kAnalogInputReadInterval = 1500;
  const float kAnalogInputScale = 3.3;
  const float kFixedResistorValue = 47; // 47 Ohms
  const float kMinSensorResistance = 0;
  const float kMaxSensorResistance = 190;
  const float minSensorDegrees = -35.0;
  const float maxSensorDegrees = 35.0;

  auto analog_input = std::make_shared<RepeatSensor<float>>(kAnalogInputReadInterval, [kAnalogInputPin]() {
    return analogReadMilliVolts(kAnalogInputPin) / 1000.;
  });

  analog_input->attach([analog_input]() {
    debugD("Rudder angle sensor analog input value: %f", analog_input->get());
  });

  auto voltageDivider = new  VoltageDividerR2(
    kMaxSensorResistance, kAnalogInputScale, "/Sensors/Rudder Angle/VoltageDividerR2");

  voltageDivider->attach([voltageDivider]() {
    //debugD("Rudder angle sensor resistance value: %f", voltageDivider->get());
  });


  auto transformToDegrees = linearTransformOf(
      xyPair(kMinSensorResistance, minSensorDegrees),
      xyPair(kMaxSensorResistance, maxSensorDegrees)
    );

  transformToDegrees->attach([transformToDegrees]() {
    //debugD("Rudder angle degrees value: %f", transformToDegrees->get());
  });

  auto degreesToRadians = std::make_shared<RadiansTransform>();

  auto metadata = std::make_shared<SKMetadata>("rad", "Rudder Angle");
  auto sk_output = std::make_shared<SKOutput<float>>(
      sk_path,  // Signal K path
      "/Sensors/RudderAngle/sk",
      metadata
  );

  analog_input
    ->connect_to(voltageDivider)
    ->connect_to(transformToDegrees)
    ->connect_to(degreesToRadians)
    ->connect_to(sk_output);

  auto makeStatusPageItemFor = [kUIGroup](const char* name, int order) {
    return std::make_shared<StatusPageItem<float>>(name, -1., kUIGroup, order);
  };

  auto spAnalogInput = makeStatusPageItemFor("analog input", 1);
  auto spVoldateDivider = makeStatusPageItemFor("voltage divider conversion to resistance", 2);
  auto spDegrees = makeStatusPageItemFor("linear conversion to degrees", 3);
  auto statusPageSkOut = makeStatusPageItemFor("output to SignalK", 4);
  //analog_input->connect_to(makeStatusPageItemFor("analog input"));
  analog_input->connect_to(spAnalogInput);
  voltageDivider->connect_to(spVoldateDivider);
  transformToDegrees->connect_to(spDegrees);
  sk_output->connect_to(statusPageSkOut);
  //voltageDivider->connect_to(makeStatusPageItemFor("voltage divider conversion to resistance"));
  //transformToDegrees->connect_to(makeStatusPageItemFor("linear conversion to degrees"));

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
