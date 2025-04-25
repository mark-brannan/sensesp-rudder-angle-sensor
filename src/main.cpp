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
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp_app_builder.h"
#include "linear.h"
#include "radians.h"

using namespace sensesp;

using xyPair = std::pair<float, float>;

  auto retained = std::vector<std::shared_ptr<void>>();

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
  const unsigned int kAnalogInputReadInterval = 500;
  const float kAnalogInputScale = 3.3;
  const float kFixedResistorValue = 47; // Ohms

  // Using measured min/max resistance values through esp32 ADC and sensesp;
  // which differ by ~10~20 Ohms compared with externally verified values (0-190 Ohms),
  const float kMinSensorResistance = 2.113363;
  const float kMaxSensorResistance = 223.;

  const float minSensorDegrees = -50.0;
  const float maxSensorDegrees = 50.0;

  analogSetPinAttenuation(kAnalogInputPin, ADC_ATTENDB_MAX);

  auto analog_input = std::make_shared<RepeatSensor<float>>(kAnalogInputReadInterval, [kAnalogInputPin]() {
    return analogReadMilliVolts(kAnalogInputPin) / 1000.;
  });

  analog_input->attach([analog_input]() {
    debugD("Rudder angle sensor analog input value: %f", analog_input->get());
  });

  auto voltageDivider = std::make_shared<VoltageDividerR2>(
    kFixedResistorValue, kAnalogInputScale, "/Sensors/Rudder Angle/VoltageDividerR2");

  voltageDivider->attach([voltageDivider]() {
    debugD("Rudder angle sensor resistance value: %f", voltageDivider->get());
  });

  auto transformToDegrees = linearTransformOf(
      xyPair(kMinSensorResistance, minSensorDegrees),
      xyPair(kMaxSensorResistance, maxSensorDegrees)
    );

  auto degreesToRadians = std::make_shared<RadiansTransform>();

  auto sk_output = std::make_shared<SKOutput<float>>(sk_path, "",
    std::make_shared<SKMetadata>("rad", "Rudder Angle"));

  analog_input
    ->connect_to(voltageDivider)
    ->connect_to(transformToDegrees)
    ->connect_to(degreesToRadians)
    ->connect_to(sk_output);

  auto skListener = std::make_shared<FloatSKListener>(sk_path, 500);

  auto makeStatusPageItemFor = [kUIGroup](const char* name, int order) {
    auto status_page_item = std::make_shared<StatusPageItem<float>>(name, -1., kUIGroup, order);
    retained.push_back(status_page_item);
    return status_page_item;
  };

  analog_input->connect_to(makeStatusPageItemFor("analog input", 1));
  voltageDivider->connect_to(makeStatusPageItemFor("voltage divider conversion to resistance", 2));
  transformToDegrees->connect_to(makeStatusPageItemFor("linear conversion to degrees", 3));
  sk_output->connect_to(makeStatusPageItemFor("Value sent to SK for 'steering.rudderAngle'", 4));
  skListener->connect_to(makeStatusPageItemFor("SignalK value for 'steering.rudderAngle'", 5));


  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
