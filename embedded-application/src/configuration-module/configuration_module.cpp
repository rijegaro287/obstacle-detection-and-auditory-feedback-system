#include "configuration_module.hpp"
#include "configuration_iface.hpp"

#include <iostream>
#include <thread>
#include <chrono>

// Characteristic::Characteristic(GDBusConnection* conn,
// 															 const string& uuid,
// 															 const string& path) {
// 	this->char_uuid = uuid;
// 	this->object_path = path;
// 	this->connection = conn;
// 	this->value = vector<uint8_t>();
// }

// void Characteristic::set_value(const vector<uint8_t>& value) {
// 	this->value = value;
// }

// vector<uint8_t> Characteristic::get_value() {
// 	return this->value;
// }

// void Characteristic::notify_value_changed() {
// 	printf("Characteristic %s changed value\n", this->char_uuid.c_str());
// }

// Service::Service(GDBusConnection* conn, const string& uuid, const string& path) {
// 	this->service_uuid = uuid;
// 	this->object_path = path;
// 	this->connection = conn;
// 	this->characteristics = vector<Characteristic*>();
// }

// void Service::add_characteristic(Characteristic* characteristic) {
// 	this->characteristics.push_back(characteristic);
// }

ConfigModule& ConfigModule::get_instance() {
	static ConfigModule instance;
	return instance;
}

ConfigModule::ConfigModule() {
	// Load configuration settings
}


void ConfigModule::start() {
	printf("Starting configuration module...\n");
	BLEServer::start_server();
	

	// loop = g_main_loop_new(NULL, FALSE);

	// GDBusConnection* connection = create_system_bus_connection();
	// if (!connection) {
	// 	printf("Failed to create D-Bus connection\n");
	// 	return;
	// }

	// GDBusProxy* adapter_proxy = create_adapter_proxy();
	// if (adapter_proxy == NULL) {
	// 	printf("Failed to create adapter proxy\n");
	// 	return;
	// }
	
	// GDBusProxy* gatt_manager_proxy = create_gatt_manager_proxy();
	// if (gatt_manager_proxy == NULL) {
	// 	printf("Failed to create GATT manager proxy\n");
	// 	return;
	// }

	// guint owner_id = g_bus_own_name(
	// 	G_BUS_TYPE_SYSTEM,
	// 	APP_SERVICE,
	// 	G_BUS_NAME_OWNER_FLAGS_NONE,
	// 	on_bus_acquired,
	// 	on_name_acquired,
	// 	on_name_lost,
	// 	NULL,
	// 	NULL
	// );

	// if (set_proxy_property(adapter_proxy, 
	// 											 BLUEZ_ADAPTER_IFACE,
	// 											 "Discoverable",
	// 											 g_variant_new_boolean(true)) < 0) {
	// 	printf("Failed to set adapter property\n");
	// 	return;
	// }

	
	// ========================================================================
	// GError* error = NULL;

	// GDBusNodeInfo* adv_info = g_dbus_node_info_new_for_xml(ADV_XML, &error);
	// if (error) {
	// 	printf("Failed to parse advertising XML: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// GDBusNodeInfo* app_info = g_dbus_node_info_new_for_xml(APP_XML, &error);
	// if (error) {
	// 	printf("Failed to parse app XML: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }
	
	// GDBusNodeInfo* service_info = g_dbus_node_info_new_for_xml(SERVICE_XML, &error);
	// if (error) {
	// 	printf("Failed to parse service introspection XML: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// GDBusNodeInfo* char_info = g_dbus_node_info_new_for_xml(CHARACTERISTIC_XML, &error);
	// if (error) {
	// 	printf("Failed to parse characteristic introspection XML: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// ========================================================================
	// g_dbus_connection_register_object(
	// 	connection,
	// 	APP_PATH,
	// 	app_info->interfaces[0],
	// 	&app_vtable,
	// 	NULL,
	// 	NULL,
	// 	&error
	// );

	// if (error) {
	// 	printf("Failed to register D-Bus object: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// g_dbus_connection_register_object(
	// 	connection,
	// 	SERVICE_PATH,
	// 	service_info->interfaces[0],
	// 	&service_vtable,
	// 	NULL,
	// 	NULL,
	// 	&error
	// );

	// if (error) {
	// 	printf("Failed to register D-Bus object: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// g_dbus_connection_register_object(
	// 	connection,
	// 	CHARACTERISTIC_PATH,
	// 	char_info->interfaces[0],
	// 	&char_vtable,
	// 	NULL,
	// 	NULL,
	// 	&error
	// );

	// if (error) {
	// 	printf("Failed to register D-Bus object: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// g_dbus_connection_register_object(
	// 	connection,
	// 	ADVERTISING_PATH,
	// 	adv_info->interfaces[0],
	// 	&adv_vtable,
	// 	NULL,
	// 	NULL,
	// 	&error
	// );

	// if (error) {
	// 	printf("Failed to register D-Bus object: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }

	// ========================================================================
	// g_dbus_proxy_call_sync(
	// 	gatt_manager_proxy,
	// 	"RegisterApplication",
	// 	g_variant_new("(oa{sv})", APP_PATH, NULL),
	// 	G_DBUS_CALL_FLAGS_NONE,
	// 	-1,
	// 	NULL,
	// 	&error
	// );

	// if (error) {
	// 	printf("Failed to register application: %s\n", error->message);
	// 	g_error_free(error);
	// 	return;
	// }
	// if (set_proxy_property(adapter_proxy, 
	// 											 BLUEZ_ADAPTER_IFACE,
	// 											 "Powered",
	// 											 g_variant_new_boolean(true)) < 0) {
	// 	printf("Failed to set adapter property\n");
	// 	return;
	// }

	

	// Service service(connection, "service_uuid", "/org/example/test_service");
	// Characteristic char(connection, "char_uuid", "/org/example/test_service/char");
	
	// service.add_characteristic(&char);

	// vector<uint8_t> initial_value = {0x00, 0x01, 0x02};
	// char.set_value(initial_value);
	// char.notify_value_changed();
}
