#pragma once

/**
 * @file bt_controller.hpp
 * @brief Helper utilities that wrap the BlueZ D-Bus interface for device and
 * adapter management.
 */

#include <cstdint>
#include <vector>
#include <string>

#include <gio/gio.h>

/** @brief D-Bus service name for BlueZ. */
#define BLUEZ_SERVICE "org.bluez"
/** @brief Adapter interface exposed by BlueZ. */
#define BLUEZ_ADAPTER_IFACE "org.bluez.Adapter1"
/** @brief Device interface exposed by BlueZ. */
#define BLUEZ_DEVICE_IFACE "org.bluez.Device1"
/** @brief Default adapter path for the primary controller. */
#define BLUEZ_ADAPTER_PATH "/org/bluez/hci0"

/** @brief Maximum string length for friendly device names. */
#define BUFFER_SIZE_L 256
/** @brief Maximum string length for Bluetooth device addresses. */
#define BUFFER_SIZE_S 32

using namespace std;

/**
 * @brief Plain-old-data structure that stores the information required to
 * operate on a BlueZ device.
 * @ingroup configuration_module
 */
typedef struct BlueZDevice_ {
	char name[BUFFER_SIZE_L];   /**< Null-terminated device friendly name. */
	char address[BUFFER_SIZE_S];/**< Bluetooth MAC address formatted as XX:XX. */
} BlueZDevice;

/**
 * @class BTController
 * @ingroup configuration_module
 * @brief Base class containing reusable primitives to query and control BlueZ
 * adapters and devices via GDBus.
 */
class BTController {
public:
  /**
   * @brief Create a synchronous connection to the system D-Bus.
   * @return Pointer to the established connection or nullptr on error.
   */
  GDBusConnection* create_system_bus_connection();

  /**
   * @brief Build a proxy targeting the BlueZ object manager.
   * @return Proxy object used for enumerating managed objects.
   */
  GDBusProxy* create_object_manager_proxy();

  /**
   * @brief Build a generic properties proxy for BlueZ managed objects.
   * @return Pointer to the newly created proxy or nullptr on failure.
   */
  GDBusProxy* create_properties_proxy();

  /**
   * @brief Build a proxy bound to the default adapter interface.
   * @return Pointer to the adapter proxy or nullptr on failure.
   */
  GDBusProxy* create_adapter_proxy();

  /**
   * @brief Build a device proxy for the provided Bluetooth address.
   * @param device Device descriptor containing the target address.
   * @return Pointer to the device proxy or nullptr on failure.
   */
  GDBusProxy* create_device_proxy(BlueZDevice& device);

  /**
   * @brief Fetch the collection of managed objects exposed by BlueZ.
   * @param proxy Object manager proxy to query.
   * @return Variant containing the managed objects dictionary.
   */
  GVariant* get_managed_objects(GDBusProxy *proxy);

  /**
   * @brief Retrieve a single property value from a BlueZ-managed object.
   * @param proxy Properties proxy pointing to the object of interest.
   * @param interface Interface name holding the property.
   * @param property Desired property key.
   * @return Variant containing the property payload or nullptr on error.
   */
  GVariant* get_proxy_property(GDBusProxy *proxy, const char *interface, const char *property);

  /**
   * @brief Update a property value on a BlueZ-managed object.
   * @param proxy Properties proxy pointing to the object of interest.
   * @param interface Interface name holding the property.
   * @param property Desired property key.
  * @param value Variant to be written.
   * @return 0 on success, -1 on error.
   */
  int64_t set_proxy_property(GDBusProxy *proxy, const char *interface, const char *property, GVariant *value);

  /**
   * @brief Convert the managed object response into a vector of devices.
   * @param devices_variant Variant returned by `GetManagedObjects`.
   * @param dest Output parameter to store parsed device descriptors.
   * @return Number of devices parsed or -1 on error.
   */
  int64_t parse_devices(GVariant *devices_variant, vector<BlueZDevice>& dest);
  
  /**
   * @brief Convert a Bluetooth MAC address into a DBus object path suffix.
   * @param addr Input address (colon separated).
   * @param dest Output buffer that receives the formatted address.
   * @param dest_size Size of the output buffer.
   */
  void addr_to_path(char *addr, char *dest, uint64_t dest_size);

  /**
   * @brief Print the list of discovered devices to stdout.
   * @param devices Vector of devices previously returned by @ref parse_devices.
   */
  void print_devices(vector<BlueZDevice>& devices);

  /**
   * @brief Zero-out cached device descriptors.
   * @param devices Vector of devices to clear.
   */
  void clear_devices(vector<BlueZDevice>& devices);
};
