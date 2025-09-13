#pragma once

#include "bt_controller.hpp"

#define GATT_MANAGER_IFACE "org.bluez.GattManager1"
#define GATT_APPLICATION_IFACE "org.bluez.GattApplication1"
#define GATT_SERVICE_IFACE "org.bluez.GattService1"
#define GATT_CHARACTERISTIC_IFACE "org.bluez.GattCharacteristic1"
#define LE_ADVERTISING_MANAGER_IFACE "org.bluez.LEAdvertisingManager1"

#define APP_PATH "/com/odafs/app"
#define SERVICE_PATH "/com/odafs/app/service"
#define CHARACTERISTIC_PATH "/com/odafs/app/service/char"
#define ADVERTISING_PATH "/com/odafs/app/advertising"

#define DEVICE_NAME "odafs"
#define HID_APPEARANCE_CODE 0x03C0

#define SERVICE_NAME "com.odafs.service"
#define SERVICE_UUID "9b19df40-4042-4479-0000-131cd24590be"
#define CHARACTERISTIC_UUID "9b19df40-4042-4479-0001-131cd24590be"

static const char *APP_XML =
"<node>"
"  <interface name='org.freedesktop.DBus.ObjectManager'>"
"    <method name='GetManagedObjects'>"
"      <arg type='a{oa{sa{sv}}}' name='objects' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

static const char *ADV_XML =
"<node>"
"  <interface name='org.bluez.LEAdvertisement1'>"
"    <property name='Type' type='s' access='read'/>"
"    <property name='LocalName' type='s' access='read'/>"
"    <property name='Appearance' type='q' access='read'/>"
"    <property name='Discoverable' type='b' access='read'/>"
"    <property name='DiscoverableTimeout' type='q' access='read'/>"
"    <property name='ScanResponseServiceUUIDs' type='as' access='read'/>"
"  </interface>"
"</node>";

static const char *SERVICE_XML = 
"<node>"
" <interface name='org.bluez.GattService1'>"
" 	<property name='UUID' type='s' access='read'/>"
" 	<property name='Primary' type='b' access='read'/>"
" </interface>"
" <interface name='org.freedesktop.DBus.Properties'/>"
"</node>";

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

class BLEServer : public BTController {
public:
  BLEServer(const BLEServer&) = delete;
  BLEServer& operator=(const BLEServer&) = delete;
  BLEServer(BLEServer&&) = delete;
  BLEServer& operator=(BLEServer&&) = delete;

	void start();
	void cleanup();
private:
  GMainLoop *main_loop;
  GDBusConnection *connection;

  GDBusNodeInfo *adv_info;
  GDBusNodeInfo *app_info;
  GDBusNodeInfo *service_info;
  GDBusNodeInfo *char_info;

	static GVariant* handle_adv_get_property(GDBusConnection* connection,
																					 const gchar* sender,
																					 const gchar* object_path,
																					 const gchar* interface_name,
																					 const gchar* property_name,
																					 GError** error,
																					 gpointer user_data);

	static void handle_app_method_call(GDBusConnection* connection,
																		 const gchar* sender,
																		 const gchar* object_path,
																		 const gchar* interface_name,
																		 const gchar* method_name,
																		 GVariant* parameters,
																		 GDBusMethodInvocation* invocation,
																		 gpointer user_data);

	static GVariant* handle_service_get_property(GDBusConnection *connection,
																				const gchar *sender,
																				const gchar *object_path,
																				const gchar *interface_name,
																				const gchar *property_name,
																				GError** error,
																				gpointer user_data);
															
	static void handle_char_method_call(GDBusConnection* connection,
																			const gchar* sender,
																			const gchar* object_path,
																			const gchar* interface_name,
																			const gchar* method_name,
																			GVariant* parameters,
																			GDBusMethodInvocation* invocation,
																			gpointer user_data);

	static GVariant* handle_char_get_property(GDBusConnection *connection,
																						const gchar *sender,
																						const gchar *object_path,
																						const gchar *interface_name,
																						const gchar *property_name,
																						GError** error,
																						gpointer user_data);

	int64_t init();
	int64_t register_application();
	int64_t advertise_application();

	BLEServer();
	~BLEServer() { this->cleanup(); };
};
