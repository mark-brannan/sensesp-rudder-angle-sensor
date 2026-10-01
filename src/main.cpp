// Rudder angle sensor for Signal K, built on SensESP.
//
// A resistive rudder angle sensor is read through a voltage divider on an
// analog input pin. The chain is:
//
//   analog input (V) -> VoltageDividerR2 (Ohms) -> Linear (degrees)
//     -> DegreesToRadians (rad) -> SKOutput "steering.rudderAngle"
//
// Every intermediate value is shown on the status page, and a listener
// echoes the value the Signal K server reports back, so the whole chain can
// be validated from the web UI.

#include <memory>

#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/voltagedivider.h"
#include "sensesp/ui/config_item.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp_app_builder.h"

#include "linear.h"
#include "radians.h"

using namespace sensesp;

namespace {

constexpr char kLogTag[] = "rudder_angle";

// https://signalk.org/specification/1.7.0/doc/vesselsBranch.html#vesselsregexpsteeringrudderangle
constexpr char kSKPath[] = "steering.rudderAngle";
constexpr char kUIGroup[] = "Rudder Angle Sensor";

// GPIO number to use for the analog input
constexpr uint8_t kAnalogInputPin = 36;
// Define how often (in milliseconds) new samples are acquired
constexpr unsigned int kAnalogInputReadInterval = 500;
constexpr float kAnalogInputScale = 3.3f;
// Ohms, adjust to match your voltage divider
constexpr float kFixedResistorValue = 47.0f;

// Using measured min/max resistance values through esp32 ADC and sensesp;
// which differ by ~10~20 Ohms compared with externally verified values
// (0-190 Ohms). Check the status page (or logs) for your installation and
// update accordingly.
constexpr float kMinSensorResistance = 2.113363f;
constexpr float kMaxSensorResistance = 223.0f;

constexpr float kMinSensorDegrees = -40.0f;
constexpr float kMaxSensorDegrees = 40.0f;

// Make a status page item for a float value in this app's UI group. The
// returned item is kept alive by the producer it is connected to.
std::shared_ptr<StatusPageItem<float>> status_item(const char* name,
                                                   int order) {
  return std::make_shared<StatusPageItem<float>>(name, -1.0f, kUIGroup,
                                                 order);
}

}  // namespace

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

  analogSetPinAttenuation(kAnalogInputPin, ADC_ATTENDB_MAX);

  auto analog_input = std::make_shared<RepeatSensor<float>>(
      kAnalogInputReadInterval,
      []() { return analogReadMilliVolts(kAnalogInputPin) / 1000.0f; });

  analog_input->connect_to(std::make_shared<LambdaConsumer<float>>([](float v) {
    ESP_LOGD(kLogTag, "Rudder angle sensor analog input value: %f", v);
  }));

  auto voltage_divider = std::make_shared<VoltageDividerR2>(
      kFixedResistorValue, kAnalogInputScale,
      "/Sensors/Rudder Angle/VoltageDividerR2");

  ConfigItem(voltage_divider)
      ->set_title("Voltage Divider")
      ->set_description(
          "Voltage divider for rudder angle sensor and analog input")
      ->set_sort_order(100);

  voltage_divider->connect_to(
      std::make_shared<LambdaConsumer<float>>([](float v) {
        ESP_LOGD(kLogTag, "Rudder angle sensor resistance value: %f", v);
      }));

  auto transform_to_degrees = linear_transform_of(
      XYPair(kMinSensorResistance, kMinSensorDegrees),
      XYPair(kMaxSensorResistance, kMaxSensorDegrees),
      "/Sensors/Rudder Angle/LinearTransform");

  ConfigItem(transform_to_degrees)
      ->set_title("Linear Transform to Degrees")
      ->set_description(
          "Maps the measured sensor resistance (Ohms) to rudder angle "
          "(degrees)")
      ->set_sort_order(200);

  auto degrees_to_radians_transform = std::make_shared<DegreesToRadians>();

  auto sk_output = std::make_shared<SKOutput<float>>(
      kSKPath, "", std::make_shared<SKMetadata>("rad", "Rudder Angle"));

  sk_output->connect_to(std::make_shared<LambdaConsumer<float>>([](float rad) {
    ESP_LOGD(kLogTag, "Final '%s' value: %f radians (%f degrees)", kSKPath,
             rad, radians_to_degrees(rad));
  }));

  analog_input->connect_to(voltage_divider)
      ->connect_to(transform_to_degrees)
      ->connect_to(degrees_to_radians_transform)
      ->connect_to(sk_output);

  // Listen to values sent back *from* the Signal K server in order to
  // easily validate or troubleshoot.
  auto sk_listener = std::make_shared<FloatSKListener>(kSKPath, 500);

  // Display intermediate values on the status page for ease of
  // validation/troubleshooting.
  analog_input->connect_to(status_item("analog input", 1));
  voltage_divider->connect_to(
      status_item("voltage divider conversion to resistance", 2));
  transform_to_degrees->connect_to(
      status_item("linear conversion to degrees", 3));
  sk_output->connect_to(
      status_item("Value sent to SK for 'steering.rudderAngle'", 4));
  sk_listener->connect_to(
      status_item("SignalK value for 'steering.rudderAngle'", 5));

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
