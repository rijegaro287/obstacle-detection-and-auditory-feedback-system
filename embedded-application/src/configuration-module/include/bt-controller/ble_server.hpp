#pragma once

/**
 * @file ble_server.hpp
 * @brief Bluetooth Low-Energy GATT server that exposes the configuration
 * characteristic used for command/control of the embedded system.
 */

#include "bt_controller.hpp"

#ifdef UNIT_TESTING
#include <functional>
#endif

/** @brief Interface name for the BlueZ GATT manager. */
#define GATT_MANAGER_IFACE "org.bluez.GattManager1"
/** @brief Interface used to represent a GATT application. */
#define GATT_APPLICATION_IFACE "org.bluez.GattApplication1"
/** @brief Interface name for GATT services. */
#define GATT_SERVICE_IFACE "org.bluez.GattService1"
/** @brief Interface name for GATT characteristics. */
#define GATT_CHARACTERISTIC_IFACE "org.bluez.GattCharacteristic1"
/** @brief Interface name for BlueZ advertising manager. */
#define LE_ADVERTISING_MANAGER_IFACE "org.bluez.LEAdvertisingManager1"

/** @brief Object path for the custom BLE application. */
#define APP_PATH "/com/odafs/app"
/** @brief Object path for the custom BLE service. */
#define SERVICE_PATH "/com/odafs/app/service"
/** @brief Object path for the configuration characteristic. */
#define CHARACTERISTIC_PATH "/com/odafs/app/service/char"
/** @brief Object path used for LE advertising. */
#define ADVERTISING_PATH "/com/odafs/app/advertising"

/** @brief BLE advertising name broadcast by the system. */
#define DEVICE_NAME "odafs"
/** @brief Appearance code describing a HID-like peripheral. */
#define HID_APPEARANCE_CODE 0x03C0

/** @brief Service name exposed to BlueZ. */
#define SERVICE_NAME "com.odafs.service"
/** @brief UUID assigned to the main configuration service. */
#define SERVICE_UUID "9b19df40-4042-4479-0000-131cd24590be"
/** @brief UUID assigned to the configuration characteristic. */
#define CHARACTERISTIC_UUID "9b19df40-4042-4479-0001-131cd24590be"

/** @brief XML definition describing the application object hierarchy. */
static const char *APP_XML =
"<node>"
"  <interface name='org.freedesktop.DBus.ObjectManager'>"
"    <method name='GetManagedObjects'>"
"      <arg type='a{oa{sa{sv}}}' name='objects' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

/** @brief XML definition describing the advertisement object exposed to BlueZ. */
static const char *ADV_XML =
"<node>"
"  <interface name='org.bluez.LEAdvertisement1'>"
"    <property name='Type' type='s' access='read'/>"
"    <property name='LocalName' type='s' access='read'/>"
"    <property name='Appearance' type='q' access='read'/>"
"    <property name='Discoverable' type='b' access='read'/>"
"    <property name='DiscoverableTimeout' type='q' access='read'/>"
"    <property name='ScanResponseServiceUUIDs' type='as' access='read'/>"
"    <property name='ServiceUUIDs' type='as' access='read'/>"
"  </interface>"
"</node>";

/** @brief XML definition describing the custom GATT service. */
static const char *SERVICE_XML = 
"<node>"
" <interface name='org.bluez.GattService1'>"
" 	<property name='UUID' type='s' access='read'/>"
" 	<property name='Primary' type='b' access='read'/>"
" </interface>"
" <interface name='org.freedesktop.DBus.Properties'/>"
"</node>";

/** @brief XML definition describing the configuration characteristic. */
static const char *CHAR_XML = 
"<node>"
"  <interface name='org.bluez.GattCharacteristic1'>"
"    <method name='ReadValue'>"
"      <arg type='a{sv}' name='options' direction='in'/>"
"      <arg type='ay' name='value' direction='out'/>"
"    </method>"
"    <method name='WriteValue'>"
"      <arg type='ay' name='value' direction='in'/>"
"      <arg type='a{sv}' name='options' direction='in'/>"
"    </method>"
"  </interface>"
"</node>";

/**
 * @class BLEServer
 * @ingroup configuration_module
 * @brief Singleton responsible for exposing the configuration characteristic
 * over BLE using the BlueZ GATT server APIs.
 */
class BLEServer : public BTController {
public:
  BLEServer(const BLEServer&) = delete;
  BLEServer& operator=(const BLEServer&) = delete;
  BLEServer(BLEServer&&) = delete;
  BLEServer& operator=(BLEServer&&) = delete;

	/**
	 * @brief Retrieve the BLE server singleton instance.
	 * @return Reference to the server.
	 */
	static BLEServer& get_instance();

	/**
	 * @brief Start the BLE server main loop, registering objects and
	 * advertising the configuration service.
	 */
	void start();

	/**
	 * @brief Release all GLib and BlueZ resources associated with the server.
	 */
	void cleanup();
	int64_t init_for_test();
	int64_t register_application_for_test();
	int64_t advertise_application_for_test();

private:
  GMainLoop *main_loop;           /**< GLib event loop used by the server. */
  GDBusConnection *connection;    /**< Connection handle to the system bus. */

  GDBusNodeInfo *adv_info;        /**< Introspection data for the advertisement. */
  GDBusNodeInfo *app_info;        /**< Introspection data for the GATT application. */
  GDBusNodeInfo *service_info;    /**< Introspection data for the service. */
  GDBusNodeInfo *char_info;       /**< Introspection data for the characteristic. */

	#ifdef UNIT_TESTING
	public:
		/**
		 * @brief Limit the number of iterations the main loop executes during tests.
		 * @param iterations Number of iterations to run before exiting.
		 */
		static void set_loop_iterations(uint32_t iterations);

		/**
		 * @brief Disable the iteration limiter set for unit tests.
		 */
		static void disable_loop_iteration_limit();

		/**
		 * @brief Skip running the GLib main loop during tests to avoid blocking.
		 * @param skip Whether to skip the main loop invocation.
		 */
		static void skip_main_loop_for_tests(bool skip);

		/**
		 * @brief Provide a test-only response generator for characteristic reads.
		 * @param provider Callback that returns the payload to expose.
		 */
		static void set_test_response_provider(std::function<std::string()> provider);

		/**
		 * @brief Provide a test-only handler invoked when commands are written.
		 * @param handler Callback receiving the written command string.
		 */
		static void set_test_command_handler(std::function<void(const std::string&)> handler);

		/**
		 * @brief Reset all unit-testing hooks to their defaults.
		 */
		static void reset_test_hooks();

	private:
		static uint32_t loop_iteration_budget;
		static bool loop_limit_enabled;
		static bool skip_main_loop;
		static std::function<std::string()> test_response_provider;
		static std::function<void(const std::string&)> test_command_handler;
	#endif

	/**
	 * @brief Handle property reads on the advertisement object.
	 * @param connection Active D-Bus connection.
	 * @param sender Caller unique name.
	 * @param object_path Object path targeted by the request.
	 * @param interface_name Interface containing the property.
	 * @param property_name Requested property key.
	 * @param error Output parameter receiving GLib errors.
	 * @param user_data Pointer to the BLE server instance.
	 * @return Variant representing the property value.
	 */
	static GVariant* handle_adv_get_property(GDBusConnection* connection,
																 const gchar* sender,
																 const gchar* object_path,
																 const gchar* interface_name,
																 const gchar* property_name,
																 GError** error,
																 gpointer user_data);

	/**
	 * @brief Dispatch method calls targeting the BLE application object.
	 * @param connection Active D-Bus connection.
	 * @param sender Caller unique name.
	 * @param object_path Object path targeted by the request.
	 * @param interface_name Interface declaring the invoked method.
	 * @param method_name Method identifier.
	 * @param parameters Serialized method parameters.
	 * @param invocation GLib invocation context used to send replies.
	 * @param user_data Pointer to the BLE server instance.
	 */
	static void handle_app_method_call(GDBusConnection* connection,
													 const gchar* sender,
													 const gchar* object_path,
													 const gchar* interface_name,
													 const gchar* method_name,
													 GVariant* parameters,
													 GDBusMethodInvocation* invocation,
													 gpointer user_data);

	/**
	 * @brief Handle property reads on the GATT service object.
	 * @param connection Active D-Bus connection.
	 * @param sender Caller unique name.
	 * @param object_path Object path targeted by the request.
	 * @param interface_name Interface containing the property.
	 * @param property_name Requested property key.
	 * @param error Output parameter receiving GLib errors.
	 * @param user_data Pointer to the BLE server instance.
	 * @return Variant representing the property value.
	 */
	static GVariant* handle_service_get_property(GDBusConnection *connection,
														 				 const gchar *sender,
														 				 const gchar *object_path,
														 				 const gchar *interface_name,
														 				 const gchar *property_name,
														 				 GError** error,
														 				 gpointer user_data);
														 
	/**
	 * @brief Dispatch method calls targeting the GATT characteristic.
	 * @param connection Active D-Bus connection.
	 * @param sender Caller unique name.
	 * @param object_path Object path targeted by the request.
	 * @param interface_name Interface declaring the invoked method.
	 * @param method_name Method identifier.
	 * @param parameters Serialized method parameters.
	 * @param invocation GLib invocation context used to send replies.
	 * @param user_data Pointer to the BLE server instance.
	 */
	static void handle_char_method_call(GDBusConnection* connection,
														 const gchar* sender,
														 const gchar* object_path,
														 const gchar* interface_name,
														 const gchar* method_name,
														 GVariant* parameters,
														 GDBusMethodInvocation* invocation,
														 gpointer user_data);

	/**
	 * @brief Handle property reads on the GATT characteristic object.
	 * @param connection Active D-Bus connection.
	 * @param sender Caller unique name.
	 * @param object_path Object path targeted by the request.
	 * @param interface_name Interface containing the property.
	 * @param property_name Requested property key.
	 * @param error Output parameter receiving GLib errors.
	 * @param user_data Pointer to the BLE server instance.
	 * @return Variant representing the property value.
	 */
	static GVariant* handle_char_get_property(GDBusConnection *connection,
																			 const gchar *sender,
																			 const gchar *object_path,
																			 const gchar *interface_name,
																			 const gchar *property_name,
																			 GError** error,
																			 gpointer user_data);

#ifdef UNIT_TESTING
public:
#endif
	/**
	 * @brief Set up the GLib main loop and parse the XML descriptors.
	 * @return 0 on success, -1 on failure.
	 */
	int64_t init();

	/**
	 * @brief Register the GATT application, service, and characteristic objects.
	 * @return 0 on success, -1 on failure.
	 */
	int64_t register_application();

	/**
	 * @brief Enable advertising for the configuration service.
	 * @return 0 on success, -1 on failure.
	 */
	int64_t advertise_application();

#ifdef UNIT_TESTING
private:
#endif
	BLEServer();
	~BLEServer() { this->cleanup(); };
};
