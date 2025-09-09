#pragma once

#include "bt_controller.hpp"

#define GATT_MANAGER_IFACE "org.bluez.GattManager1"
#define GATT_APPLICATION_IFACE "org.bluez.GattApplication1"
#define GATT_SERVICE_IFACE "org.bluez.GattService1"
#define GATT_CHARACTERISTIC_IFACE "org.bluez.GattCharacteristic1"
#define LE_ADVERTISING_MANAGER_IFACE "org.bluez.LEAdvertisingManager1"

#define APP_PATH "/com/example/app"
#define SERVICE_PATH "/com/example/app/service"
#define CHARACTERISTIC_PATH "/com/example/app/service/char"
#define ADVERTISING_PATH "/com/example/app/advertising"

#define SERVICE_NAME "com.example.service"
#define SERVICE_UUID "12345678-9abc-def1-2345-6789abcdef00"
#define CHARACTERISTIC_UUID "12345678-9abc-def1-2345-6789abcdef01"

static const char *APP_XML =
"<node>"
"  <interface name='org.freedesktop.DBus.ObjectManager'>"
"    <method name='GetManagedObjects'>"
"      <arg type='a{oa{sa{sv}}}' name='objects' direction='out'/>"
"    </method>"
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

static const char *ADV_XML =
"<node>"
"  <interface name='org.bluez.LEAdvertisement1'>"
"    <property name='Type' type='s' access='read'/>"
"    <property name='ServiceUUIDs' type='as' access='read'/>"
"    <property name='LocalName' type='s' access='read'/>"
"    <property name='Includes' type='as' access='read'/>"
"  </interface>"
"</node>";

class BLEServer : public BTController {
public:
	static void start();
	static void cleanup();
private:
  static GMainLoop *main_loop;
  static GDBusConnection *connection;

  static GDBusNodeInfo *app_info;
  static GDBusNodeInfo *char_info;
  static GDBusNodeInfo *adv_info;

	static int64_t init();
	static int64_t register_application();
	static int64_t advertise_application();

	static void handle_app_method_call(GDBusConnection* connection,
																		 const gchar* sender,
																		 const gchar* object_path,
																		 const gchar* interface_name,
																		 const gchar* method_name,
																		 GVariant* parameters,
																		 GDBusMethodInvocation* invocation,
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

	static GVariant* handle_adv_get_property(GDBusConnection* connection,
																					 const gchar* sender,
																					 const gchar* object_path,
																					 const gchar* interface_name,
																					 const gchar* property_name,
																					 GError** error,
																					 gpointer user_data);
};
