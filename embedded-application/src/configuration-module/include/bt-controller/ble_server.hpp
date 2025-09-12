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
#define SERVICE_NAME "com.odafs.service"
#define SERVICE_UUID "9b19df40-4042-4479-0000-131cd24590be"
#define CHARACTERISTIC_UUID "9b19df40-4042-4479-0001-131cd24590be"

class BLEServer : public BTController {
public:
	static void start();
	static void cleanup();
private:
  static GMainLoop *main_loop;
  static GDBusConnection *connection;

  static GDBusNodeInfo *app_info;
  static GDBusNodeInfo *service_info;
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
