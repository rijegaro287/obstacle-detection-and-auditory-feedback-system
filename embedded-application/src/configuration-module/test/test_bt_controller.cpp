#include <gtest/gtest.h>

#include <gio/gio.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "bt_controller.hpp"
#include "bt_audio.hpp"
#include "ble_server.hpp"

namespace {

constexpr const char kDeviceName[] = "Test Device";
constexpr const char kDeviceAddress[] = "12:34:56:78:9A:BC";
constexpr const char kDevicePath[] = "/org/bluez/hci0/dev_12_34_56_78_9A_BC";

struct StubBlueZState {
	bool adapter_powered = false;
	bool adapter_discoverable = false;
	guint32 adapter_discoverable_timeout = 0;
	bool adapter_pairable = false;
	bool discovery_active = false;
	bool device_paired = false;
	bool device_connected = false;
	unsigned start_discovery_calls = 0;
	unsigned stop_discovery_calls = 0;
	unsigned pair_calls = 0;
	unsigned connect_calls = 0;
	unsigned disconnect_calls = 0;
	unsigned register_app_calls = 0;
	unsigned register_adv_calls = 0;
	std::string device_name = kDeviceName;
	std::string device_address = kDeviceAddress;
};

static GVariant* make_managed_objects_variant(const StubBlueZState& state) {
	GError* error = nullptr;
	gchar* payload = g_strdup_printf(
		"{ '%s': { '%s': { 'Name': <'%s'>, 'Address': <'%s'> } } }",
		kDevicePath,
		BLUEZ_DEVICE_IFACE,
		state.device_name.c_str(),
		state.device_address.c_str());
	GVariant* objects = g_variant_parse(G_VARIANT_TYPE("a{oa{sa{sv}}}"), payload, nullptr, nullptr, &error);
	g_free(payload);
	if (!objects) {
		if (error) {
			g_error_free(error);
		}
		return g_variant_new_array(G_VARIANT_TYPE("{oa{sa{sv}}}"), nullptr, 0);
	}
	return objects;
}

static GVariant* wrap_bool(bool value) {
	return g_variant_new_boolean(value);
}

static GVariant* wrap_uint32(guint32 value) {
	return g_variant_new_uint32(value);
}

static GVariant* wrap_string(const std::string& value) {
	return g_variant_new_string(value.c_str());
}

static const char kAdapterXml[] =
"<node>"
"  <interface name='org.bluez.Adapter1'>"
"    <method name='StartDiscovery'/>"
"    <method name='StopDiscovery'/>"
"  </interface>"
"  <interface name='org.bluez.GattManager1'>"
"    <method name='RegisterApplication'>"
"      <arg type='o' name='application' direction='in'/>"
"      <arg type='a{sv}' name='options' direction='in'/>"
"    </method>"
"  </interface>"
"  <interface name='org.bluez.LEAdvertisingManager1'>"
"    <method name='RegisterAdvertisement'>"
"      <arg type='o' name='advertisement' direction='in'/>"
"      <arg type='a{sv}' name='options' direction='in'/>"
"    </method>"
"  </interface>"
"  <interface name='org.freedesktop.DBus.Properties'>"
"    <method name='Get'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='s' name='property' direction='in'/>"
"      <arg type='v' name='value' direction='out'/>"
"    </method>"
"    <method name='Set'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='s' name='property' direction='in'/>"
"      <arg type='v' name='value' direction='in'/>"
"    </method>"
"    <method name='GetAll'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='a{sv}' name='properties' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

static const char kDeviceXml[] =
"<node>"
"  <interface name='org.bluez.Device1'>"
"    <method name='Pair'/>"
"    <method name='ConnectProfile'>"
"      <arg type='s' name='uuid' direction='in'/>"
"    </method>"
"    <method name='Disconnect'/>"
"  </interface>"
"  <interface name='org.freedesktop.DBus.Properties'>"
"    <method name='Get'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='s' name='property' direction='in'/>"
"      <arg type='v' name='value' direction='out'/>"
"    </method>"
"    <method name='Set'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='s' name='property' direction='in'/>"
"      <arg type='v' name='value' direction='in'/>"
"    </method>"
"    <method name='GetAll'>"
"      <arg type='s' name='interface' direction='in'/>"
"      <arg type='a{sv}' name='properties' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

static const char kObjectManagerXml[] =
"<node>"
"  <interface name='org.freedesktop.DBus.ObjectManager'>"
"    <method name='GetManagedObjects'>"
"      <arg type='a{oa{sa{sv}}}' name='objects' direction='out'/>"
"    </method>"
"  </interface>"
"</node>";

static void adapter_method_call(GDBusConnection*,
		const gchar*,
		const gchar*,
		const gchar* interface_name,
		const gchar* method_name,
		GVariant* parameters,
		GDBusMethodInvocation* invocation,
		gpointer user_data) {
	StubBlueZState* state = static_cast<StubBlueZState*>(user_data);
	if (g_strcmp0(interface_name, "org.bluez.Adapter1") == 0) {
		if (g_strcmp0(method_name, "StartDiscovery") == 0) {
			state->discovery_active = true;
			state->start_discovery_calls++;
			g_dbus_method_invocation_return_value(invocation, nullptr);
			return;
		}
		if (g_strcmp0(method_name, "StopDiscovery") == 0) {
			state->discovery_active = false;
			state->stop_discovery_calls++;
			g_dbus_method_invocation_return_value(invocation, nullptr);
			return;
		}
	}
	if (g_strcmp0(interface_name, "org.bluez.GattManager1") == 0 &&
		g_strcmp0(method_name, "RegisterApplication") == 0) {
		g_print("RegisterApplication invoked\n");
		state->register_app_calls++;
		g_print("register_app_calls now %u\n", state->register_app_calls);
		g_dbus_method_invocation_return_value(invocation, nullptr);
		return;
	}
	if (g_strcmp0(interface_name, "org.bluez.LEAdvertisingManager1") == 0 &&
		g_strcmp0(method_name, "RegisterAdvertisement") == 0) {
		g_print("RegisterAdvertisement invoked\n");
		state->register_adv_calls++;
		g_print("register_adv_calls now %u\n", state->register_adv_calls);
		g_dbus_method_invocation_return_value(invocation, nullptr);
		return;
	}
	(void)parameters;
	g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED,
		"Unexpected method %s on %s", method_name, interface_name);
}

static void adapter_properties_method_call(GDBusConnection*,
		const gchar*,
		const gchar*,
		const gchar*,
		const gchar* method_name,
		GVariant* parameters,
		GDBusMethodInvocation* invocation,
		gpointer user_data) {
	g_print("adapter_properties_method_call: %s\n", method_name);
	StubBlueZState* state = static_cast<StubBlueZState*>(user_data);
	if (g_strcmp0(method_name, "Get") == 0) {
		g_print("adapter_properties_method_call:Get invoked\n");
		const gchar* iface = nullptr;
		const gchar* property = nullptr;
		g_variant_get(parameters, "(&s&s)", &iface, &property);
		if (g_strcmp0(iface, BLUEZ_ADAPTER_IFACE) != 0) {
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface);
			return;
		}
		if (g_strcmp0(property, "Powered") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_bool(state->adapter_powered)));
			return;
		}
		if (g_strcmp0(property, "Discoverable") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_bool(state->adapter_discoverable)));
			return;
		}
		if (g_strcmp0(property, "DiscoverableTimeout") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_uint32(state->adapter_discoverable_timeout)));
			return;
		}
		if (g_strcmp0(property, "Pairable") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_bool(state->adapter_pairable)));
			return;
		}
		g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
			"Unknown property %s", property);
		return;
	}
	if (g_strcmp0(method_name, "Set") == 0) {
		g_print("adapter_properties_method_call:Set invoked\n");
		const gchar* iface = nullptr;
		const gchar* property = nullptr;
		GVariant* value_variant = nullptr;
		g_variant_get(parameters, "(&s&sv)", &iface, &property, &value_variant);
		GVariant* inner = nullptr;
		if (value_variant) {
			if (g_variant_is_of_type(value_variant, G_VARIANT_TYPE_VARIANT)) {
				inner = g_variant_get_variant(value_variant);
			}
			else {
				inner = g_variant_ref(value_variant);
			}
		}
		if (g_strcmp0(iface, BLUEZ_ADAPTER_IFACE) != 0 || inner == nullptr) {
			if (inner) g_variant_unref(inner);
			if (value_variant) g_variant_unref(value_variant);
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface ? iface : "(null)");
			return;
		}
		if (g_strcmp0(property, "Powered") == 0) {
			state->adapter_powered = g_variant_get_boolean(inner);
		}
		else if (g_strcmp0(property, "Discoverable") == 0) {
			state->adapter_discoverable = g_variant_get_boolean(inner);
		}
		else if (g_strcmp0(property, "DiscoverableTimeout") == 0) {
			state->adapter_discoverable_timeout = g_variant_get_uint32(inner);
		}
		else if (g_strcmp0(property, "Pairable") == 0) {
			state->adapter_pairable = g_variant_get_boolean(inner);
		}
		else {
			g_variant_unref(inner);
			g_variant_unref(value_variant);
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
				"Unknown property %s", property);
			return;
		}
		g_variant_unref(inner);
		g_variant_unref(value_variant);
		g_dbus_method_invocation_return_value(invocation, nullptr);
		return;
	}
	if (g_strcmp0(method_name, "GetAll") == 0) {
		g_print("adapter_properties_method_call:GetAll invoked\n");
		const gchar* iface = nullptr;
		g_variant_get(parameters, "(&s)", &iface);
		if (g_strcmp0(iface, BLUEZ_ADAPTER_IFACE) != 0) {
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface);
			return;
		}
		GVariantDict dict;
		g_variant_dict_init(&dict, nullptr);
		g_variant_dict_insert(&dict, "Powered", "v", g_variant_new_boolean(state->adapter_powered));
		g_variant_dict_insert(&dict, "Discoverable", "v", g_variant_new_boolean(state->adapter_discoverable));
		g_variant_dict_insert(&dict, "DiscoverableTimeout", "v", g_variant_new_uint32(state->adapter_discoverable_timeout));
		g_variant_dict_insert(&dict, "Pairable", "v", g_variant_new_boolean(state->adapter_pairable));
		GVariant* dict_variant = g_variant_dict_end(&dict);
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(a{sv})", dict_variant));
		g_variant_unref(dict_variant);
		return;
	}
	g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
		"Unknown method %s", method_name);
}

static void device_method_call(GDBusConnection*,
		const gchar*,
		const gchar*,
		const gchar* interface_name,
		const gchar* method_name,
		GVariant* parameters,
		GDBusMethodInvocation* invocation,
		gpointer user_data) {
	StubBlueZState* state = static_cast<StubBlueZState*>(user_data);
	(void)parameters;
	if (g_strcmp0(interface_name, "org.bluez.Device1") == 0) {
		if (g_strcmp0(method_name, "Pair") == 0) {
			state->device_paired = true;
			state->pair_calls++;
			g_dbus_method_invocation_return_value(invocation, nullptr);
			return;
		}
		if (g_strcmp0(method_name, "ConnectProfile") == 0) {
			state->device_connected = true;
			state->connect_calls++;
			g_dbus_method_invocation_return_value(invocation, nullptr);
			return;
		}
		if (g_strcmp0(method_name, "Disconnect") == 0) {
			state->device_connected = false;
			state->disconnect_calls++;
			g_dbus_method_invocation_return_value(invocation, nullptr);
			return;
		}
	}
	g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED,
		"Unexpected method %s on %s", method_name, interface_name);
}

static void device_properties_method_call(GDBusConnection*,
		const gchar*,
		const gchar*,
		const gchar*,
		const gchar* method_name,
		GVariant* parameters,
		GDBusMethodInvocation* invocation,
		gpointer user_data) {
	StubBlueZState* state = static_cast<StubBlueZState*>(user_data);
	if (g_strcmp0(method_name, "Get") == 0) {
		const gchar* iface = nullptr;
		const gchar* property = nullptr;
		g_variant_get(parameters, "(&s&s)", &iface, &property);
		if (g_strcmp0(iface, BLUEZ_DEVICE_IFACE) != 0) {
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface);
			return;
		}
		if (g_strcmp0(property, "Paired") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_bool(state->device_paired)));
			return;
		}
		if (g_strcmp0(property, "Connected") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_bool(state->device_connected)));
			return;
		}
		if (g_strcmp0(property, "Name") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_string(state->device_name)));
			return;
		}
		if (g_strcmp0(property, "Address") == 0) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", wrap_string(state->device_address)));
			return;
		}
		g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
			"Unknown property %s", property);
		return;
	}
	if (g_strcmp0(method_name, "Set") == 0) {
		const gchar* iface = nullptr;
		const gchar* property = nullptr;
		GVariant* value_variant = nullptr;
		g_variant_get(parameters, "(&s&sv)", &iface, &property, &value_variant);
		GVariant* inner = nullptr;
		if (value_variant) {
			if (g_variant_is_of_type(value_variant, G_VARIANT_TYPE_VARIANT)) {
				inner = g_variant_get_variant(value_variant);
			}
			else {
				inner = g_variant_ref(value_variant);
			}
		}
		if (g_strcmp0(iface, BLUEZ_DEVICE_IFACE) != 0 || inner == nullptr) {
			if (inner) g_variant_unref(inner);
			if (value_variant) g_variant_unref(value_variant);
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface ? iface : "(null)");
			return;
		}
		if (g_strcmp0(property, "Paired") == 0) {
			state->device_paired = g_variant_get_boolean(inner);
		}
		else if (g_strcmp0(property, "Connected") == 0) {
			state->device_connected = g_variant_get_boolean(inner);
		}
		g_variant_unref(inner);
		g_variant_unref(value_variant);
		g_dbus_method_invocation_return_value(invocation, nullptr);
		return;
	}
	if (g_strcmp0(method_name, "GetAll") == 0) {
		const gchar* iface = nullptr;
		g_variant_get(parameters, "(&s)", &iface);
		if (g_strcmp0(iface, BLUEZ_DEVICE_IFACE) != 0) {
			g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_INTERFACE,
				"Unknown interface %s", iface);
			return;
		}
		GVariantBuilder* builder = g_variant_builder_new(G_VARIANT_TYPE("a{sv}"));
		g_variant_builder_add(builder, "{sv}", "Paired", wrap_bool(state->device_paired));
		g_variant_builder_add(builder, "{sv}", "Connected", wrap_bool(state->device_connected));
		g_variant_builder_add(builder, "{sv}", "Name", wrap_string(state->device_name));
		g_variant_builder_add(builder, "{sv}", "Address", wrap_string(state->device_address));
		GVariant* dict_variant = g_variant_builder_end(builder);
		g_variant_builder_unref(builder);
		g_dbus_method_invocation_return_value(invocation, g_variant_new("(a{sv})", dict_variant));
		g_variant_unref(dict_variant);
		return;
	}
	g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
		"Unknown method %s", method_name);
}

static void object_manager_method_call(GDBusConnection*,
		const gchar*,
		const gchar*,
		const gchar*,
		const gchar* method_name,
		GVariant*,
		GDBusMethodInvocation* invocation,
		gpointer user_data) {
	g_print("object_manager_method_call invoked for %s\n", method_name);
	if (g_strcmp0(method_name, "GetManagedObjects") != 0) {
		g_dbus_method_invocation_return_error(invocation, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD,
			"Unknown method %s", method_name);
		return;
	}
	StubBlueZState* state = static_cast<StubBlueZState*>(user_data);
	GVariant* objects = make_managed_objects_variant(*state);
	g_variant_ref_sink(objects);
	g_dbus_method_invocation_return_value(invocation, g_variant_new("(@a{oa{sa{sv}}})", objects));
	g_variant_unref(objects);
	g_print("object_manager_method_call completed\n");
}

static const GDBusInterfaceVTable kAdapterVTable = {adapter_method_call, nullptr, nullptr};
static const GDBusInterfaceVTable kAdapterPropertiesVTable = {adapter_properties_method_call, nullptr, nullptr};
static const GDBusInterfaceVTable kDeviceVTable = {device_method_call, nullptr, nullptr};
static const GDBusInterfaceVTable kDevicePropertiesVTable = {device_properties_method_call, nullptr, nullptr};
static const GDBusInterfaceVTable kObjectManagerVTable = {object_manager_method_call, nullptr, nullptr};

class TestBlueZEnvironment {
public:
	TestBlueZEnvironment() = default;
	~TestBlueZEnvironment() { this->Teardown(); }

	bool Init() {
		if (initialized_) return true;
		test_dbus_ = g_test_dbus_new(G_TEST_DBUS_NONE);
		if (!test_dbus_) return false;
		g_test_dbus_up(test_dbus_);
		const gchar* address = g_test_dbus_get_bus_address(test_dbus_);
		prev_system_address_ = g_strdup(g_getenv("DBUS_SYSTEM_BUS_ADDRESS"));
		prev_g_system_address_ = g_strdup(g_getenv("G_DBUS_SYSTEM_BUS_ADDRESS"));
		g_setenv("DBUS_SYSTEM_BUS_ADDRESS", address, TRUE);
		g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", address, TRUE);

		GError* error = nullptr;
		connection_ = g_dbus_connection_new_for_address_sync(
			address,
			static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
				G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION),
			nullptr,
			nullptr,
			&error);
		if (!connection_) {
			if (error) g_error_free(error);
			return false;
		}
		if (error) g_error_free(error);
		g_dbus_connection_set_exit_on_close(connection_, FALSE);

		if (!this->RegisterObjects()) {
			return false;
		}
		main_loop_ = g_main_loop_new(nullptr, FALSE);
		if (!main_loop_) {
			return false;
		}
		main_loop_thread_ = g_thread_new("bluez-test-loop", &TestBlueZEnvironment::RunLoop, this);
		if (!main_loop_thread_) {
			return false;
		}
		name_owner_id_ = g_bus_own_name_on_connection(connection_, BLUEZ_SERVICE, G_BUS_NAME_OWNER_FLAGS_NONE, nullptr, nullptr, nullptr, nullptr);
		if (name_owner_id_ == 0) {
			return false;
		}
		initialized_ = true;
		return true;
	}

	StubBlueZState& state() { return state_; }
	GDBusConnection* connection() const { return connection_; }

private:
	bool RegisterObjects() {
		GError* error = nullptr;
		adapter_info_ = g_dbus_node_info_new_for_xml(kAdapterXml, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* adapter_iface = g_dbus_node_info_lookup_interface(adapter_info_, "org.bluez.Adapter1");
		adapter_id_ = g_dbus_connection_register_object(connection_, BLUEZ_ADAPTER_PATH, const_cast<GDBusInterfaceInfo*>(adapter_iface), &kAdapterVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* adapter_props_iface = g_dbus_node_info_lookup_interface(adapter_info_, "org.freedesktop.DBus.Properties");
		adapter_props_id_ = g_dbus_connection_register_object(connection_, BLUEZ_ADAPTER_PATH, const_cast<GDBusInterfaceInfo*>(adapter_props_iface), &kAdapterPropertiesVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* gatt_iface = g_dbus_node_info_lookup_interface(adapter_info_, "org.bluez.GattManager1");
		gatt_manager_id_ = g_dbus_connection_register_object(connection_, BLUEZ_ADAPTER_PATH, const_cast<GDBusInterfaceInfo*>(gatt_iface), &kAdapterVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* adv_iface = g_dbus_node_info_lookup_interface(adapter_info_, "org.bluez.LEAdvertisingManager1");
		adv_manager_id_ = g_dbus_connection_register_object(connection_, BLUEZ_ADAPTER_PATH, const_cast<GDBusInterfaceInfo*>(adv_iface), &kAdapterVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}

		device_info_ = g_dbus_node_info_new_for_xml(kDeviceXml, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* device_iface = g_dbus_node_info_lookup_interface(device_info_, "org.bluez.Device1");
		device_id_ = g_dbus_connection_register_object(connection_, kDevicePath, const_cast<GDBusInterfaceInfo*>(device_iface), &kDeviceVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* device_props_iface = g_dbus_node_info_lookup_interface(device_info_, "org.freedesktop.DBus.Properties");
		device_props_id_ = g_dbus_connection_register_object(connection_, kDevicePath, const_cast<GDBusInterfaceInfo*>(device_props_iface), &kDevicePropertiesVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}

		object_manager_info_ = g_dbus_node_info_new_for_xml(kObjectManagerXml, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		const GDBusInterfaceInfo* object_manager_iface = g_dbus_node_info_lookup_interface(object_manager_info_, "org.freedesktop.DBus.ObjectManager");
		object_manager_id_ = g_dbus_connection_register_object(connection_, "/", const_cast<GDBusInterfaceInfo*>(object_manager_iface), &kObjectManagerVTable, &state_, nullptr, &error);
		if (error) {
			g_error_free(error);
			return false;
		}
		return true;
	}

	void Teardown() {
		if (connection_) {
			if (object_manager_id_) {
				g_dbus_connection_unregister_object(connection_, object_manager_id_);
				object_manager_id_ = 0;
			}
			if (device_props_id_) {
				g_dbus_connection_unregister_object(connection_, device_props_id_);
				device_props_id_ = 0;
			}
			if (device_id_) {
				g_dbus_connection_unregister_object(connection_, device_id_);
				device_id_ = 0;
			}
			if (adv_manager_id_) {
				g_dbus_connection_unregister_object(connection_, adv_manager_id_);
				adv_manager_id_ = 0;
			}
			if (gatt_manager_id_) {
				g_dbus_connection_unregister_object(connection_, gatt_manager_id_);
				gatt_manager_id_ = 0;
			}
			if (adapter_props_id_) {
				g_dbus_connection_unregister_object(connection_, adapter_props_id_);
				adapter_props_id_ = 0;
			}
			if (adapter_id_) {
				g_dbus_connection_unregister_object(connection_, adapter_id_);
				adapter_id_ = 0;
			}
			if (name_owner_id_) {
				g_bus_unown_name(name_owner_id_);
				name_owner_id_ = 0;
			}
			g_object_unref(connection_);
			connection_ = nullptr;
		}
		if (adapter_info_) {
			g_dbus_node_info_unref(adapter_info_);
			adapter_info_ = nullptr;
		}
		if (device_info_) {
			g_dbus_node_info_unref(device_info_);
			device_info_ = nullptr;
		}
		if (object_manager_info_) {
			g_dbus_node_info_unref(object_manager_info_);
			object_manager_info_ = nullptr;
		}
		if (test_dbus_) {
			g_test_dbus_down(test_dbus_);
			g_object_unref(test_dbus_);
			test_dbus_ = nullptr;
		}
		if (main_loop_) {
			g_main_loop_quit(main_loop_);
		}
		if (main_loop_thread_) {
			g_thread_join(main_loop_thread_);
			main_loop_thread_ = nullptr;
		}
		if (main_loop_) {
			g_main_loop_unref(main_loop_);
			main_loop_ = nullptr;
		}
		if (prev_system_address_) {
			g_setenv("DBUS_SYSTEM_BUS_ADDRESS", prev_system_address_, TRUE);
			g_free(prev_system_address_);
			prev_system_address_ = nullptr;
		}
		else {
			g_unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
		}
		if (prev_g_system_address_) {
			g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", prev_g_system_address_, TRUE);
			g_free(prev_g_system_address_);
			prev_g_system_address_ = nullptr;
		}
		else {
			g_unsetenv("G_DBUS_SYSTEM_BUS_ADDRESS");
		}
		initialized_ = false;
	}

	StubBlueZState state_{};
	GTestDBus* test_dbus_ = nullptr;
	gchar* prev_system_address_ = nullptr;
	gchar* prev_g_system_address_ = nullptr;
	GDBusConnection* connection_ = nullptr;
	GDBusNodeInfo* adapter_info_ = nullptr;
	GDBusNodeInfo* device_info_ = nullptr;
	GDBusNodeInfo* object_manager_info_ = nullptr;
	guint adapter_id_ = 0;
	guint adapter_props_id_ = 0;
	guint gatt_manager_id_ = 0;
	guint adv_manager_id_ = 0;
	guint device_id_ = 0;
	guint device_props_id_ = 0;
	guint object_manager_id_ = 0;
	guint name_owner_id_ = 0;
	bool initialized_ = false;
	GMainLoop* main_loop_ = nullptr;
	GThread* main_loop_thread_ = nullptr;
	static gpointer RunLoop(gpointer data) {
		TestBlueZEnvironment* self = static_cast<TestBlueZEnvironment*>(data);
		g_main_loop_run(self->main_loop_);
		return nullptr;
	}
};

static BlueZDevice make_device(const StubBlueZState& state) {
	BlueZDevice device{};
	strncpy(device.name, state.device_name.c_str(), BUFFER_SIZE_L - 1);
	device.name[BUFFER_SIZE_L - 1] = '\0';
	strncpy(device.address, state.device_address.c_str(), BUFFER_SIZE_S - 1);
	device.address[BUFFER_SIZE_S - 1] = '\0';
	return device;
}

} // namespace

class BluetoothControllerTest : public ::testing::Test {
protected:
	void SetUp() override {
		env = std::make_unique<TestBlueZEnvironment>();
		ASSERT_TRUE(env->Init());
		BTController::set_test_system_connection(env->connection());
		BTAudioController::reset_test_overrides();
		BTAudioController::get_instance().connected_device = nullptr;
		BLEServer::reset_test_hooks();
	}

	void TearDown() override {
		BTController::clear_test_system_connection();
		BLEServer::get_instance().cleanup();
		BLEServer::reset_test_hooks();
		env.reset();
	}

	std::unique_ptr<TestBlueZEnvironment> env;
};

TEST_F(BluetoothControllerTest, CreateSystemBusConnectionSucceeds) {
	BTController controller;
	GDBusConnection* connection = controller.create_system_bus_connection();
	ASSERT_NE(connection, nullptr);
	g_object_unref(connection);
}

TEST(BluetoothControllerErrors, CreateSystemBusConnectionFailsWithInvalidAddress) {
	BTController controller;
	gchar* prev_system = g_strdup(g_getenv("DBUS_SYSTEM_BUS_ADDRESS"));
	gchar* prev_g_system = g_strdup(g_getenv("G_DBUS_SYSTEM_BUS_ADDRESS"));
	g_setenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/nonexistent-bus", TRUE);
	g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/nonexistent-bus", TRUE);
	GDBusConnection* connection = controller.create_system_bus_connection();
	EXPECT_EQ(connection, nullptr);
	if (prev_system) {
		g_setenv("DBUS_SYSTEM_BUS_ADDRESS", prev_system, TRUE);
		g_free(prev_system);
	}
	else {
		g_unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
	}
	if (prev_g_system) {
		g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", prev_g_system, TRUE);
		g_free(prev_g_system);
	}
	else {
		g_unsetenv("G_DBUS_SYSTEM_BUS_ADDRESS");
	}
}

TEST_F(BluetoothControllerTest, CreateObjectManagerProxySucceeds) {
	BTController controller;
	GDBusProxy* proxy = controller.create_object_manager_proxy();
	ASSERT_NE(proxy, nullptr);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, CreatePropertiesProxySucceeds) {
	BTController controller;
	GDBusProxy* proxy = controller.create_properties_proxy();
	ASSERT_NE(proxy, nullptr);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, CreateAdapterProxySucceeds) {
	BTController controller;
	GDBusProxy* proxy = controller.create_adapter_proxy();
	ASSERT_NE(proxy, nullptr);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, CreateDeviceProxySucceeds) {
	BTController controller;
	BlueZDevice device = make_device(env->state());
	GDBusProxy* proxy = controller.create_device_proxy(device);
	ASSERT_NE(proxy, nullptr);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, GetManagedObjectsReturnsData) {
	BTController controller;
	GDBusProxy* proxy = controller.create_object_manager_proxy();
	ASSERT_NE(proxy, nullptr);
	GVariant* managed_objects = controller.get_managed_objects(proxy);
	ASSERT_NE(managed_objects, nullptr);
	g_variant_unref(managed_objects);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, GetProxyPropertyReadsAdapterState) {
	BTController controller;
	GDBusProxy* proxy = controller.create_adapter_proxy();
	ASSERT_NE(proxy, nullptr);
	GVariant* property = controller.get_proxy_property(proxy, BLUEZ_ADAPTER_IFACE, "Powered");
	ASSERT_NE(property, nullptr);
	GVariant* container = g_variant_get_child_value(property, 0);
	ASSERT_NE(container, nullptr);
	GVariant* value = g_variant_get_variant(container);
	ASSERT_NE(value, nullptr);
	EXPECT_FALSE(g_variant_get_boolean(value));
	g_variant_unref(value);
	g_variant_unref(container);
	g_variant_unref(property);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, GetProxyPropertyRejectsInvalidInput) {
	BTController controller;
	GDBusProxy* proxy = controller.create_adapter_proxy();
	ASSERT_NE(proxy, nullptr);
	EXPECT_EQ(controller.get_proxy_property(proxy, BLUEZ_ADAPTER_IFACE, nullptr), nullptr);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, SetProxyPropertyUpdatesAdapterState) {
	BTController controller;
	GDBusProxy* proxy = controller.create_adapter_proxy();
	ASSERT_NE(proxy, nullptr);
	EXPECT_EQ(controller.set_proxy_property(proxy, BLUEZ_ADAPTER_IFACE, "Powered", g_variant_new_boolean(TRUE)), 0);
	EXPECT_TRUE(env->state().adapter_powered);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, SetProxyPropertyRejectsNullVariant) {
	BTController controller;
	GDBusProxy* proxy = controller.create_adapter_proxy();
	ASSERT_NE(proxy, nullptr);
	EXPECT_EQ(controller.set_proxy_property(proxy, BLUEZ_ADAPTER_IFACE, "Powered", nullptr), -1);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, ParseDevicesPopulatesVector) {
	BTController controller;
	GDBusProxy* proxy = controller.create_object_manager_proxy();
	ASSERT_NE(proxy, nullptr);
	GVariant* managed_objects = controller.get_managed_objects(proxy);
	ASSERT_NE(managed_objects, nullptr);
	GVariant* devices_variant = g_variant_get_child_value(managed_objects, 0);
	std::vector<BlueZDevice> devices;
	int64_t count = controller.parse_devices(devices_variant, devices);
	EXPECT_EQ(count, 0);
	ASSERT_EQ(devices.size(), 1u);
	EXPECT_STREQ(devices[0].address, env->state().device_address.c_str());
	g_variant_unref(devices_variant);
	g_variant_unref(managed_objects);
	g_object_unref(proxy);
}

TEST_F(BluetoothControllerTest, ParseDevicesRejectsInvalidVariant) {
	BTController controller;
	std::vector<BlueZDevice> devices;
	GVariant* invalid = g_variant_new("as", nullptr);
	EXPECT_EQ(controller.parse_devices(invalid, devices), -1);
}

TEST_F(BluetoothControllerTest, AddrToPathFormatsAddress) {
	BTController controller;
	char dest[BUFFER_SIZE_L] = {0};
	controller.addr_to_path(const_cast<char*>(kDeviceAddress), dest, sizeof(dest));
	EXPECT_STREQ(dest, "12_34_56_78_9A_BC");
}

TEST_F(BluetoothControllerTest, PrintDevicesEmitsOutput) {
	BTController controller;
	std::vector<BlueZDevice> devices;
	devices.push_back(make_device(env->state()));
	testing::internal::CaptureStdout();
	controller.print_devices(devices);
	std::string output = testing::internal::GetCapturedStdout();
	EXPECT_NE(output.find("Found device"), std::string::npos);
}

TEST_F(BluetoothControllerTest, ClearDevicesZeroesEntries) {
	BTController controller;
	std::vector<BlueZDevice> devices;
	devices.push_back(make_device(env->state()));
	controller.clear_devices(devices);
	EXPECT_EQ(devices[0].name[0], '\0');
	EXPECT_EQ(devices[0].address[0], '\0');
}

TEST_F(BluetoothControllerTest, StartDiscoveryTriggersAdapter) {
	BTAudioController& audio = BTAudioController::get_instance();
	EXPECT_EQ(audio.start_discovery(), 0);
	EXPECT_TRUE(env->state().discovery_active);
	EXPECT_EQ(env->state().start_discovery_calls, 1u);
}

TEST_F(BluetoothControllerTest, StopDiscoveryTriggersAdapter) {
	BTAudioController& audio = BTAudioController::get_instance();
	audio.start_discovery();
	EXPECT_EQ(audio.stop_discovery(), 0);
	EXPECT_FALSE(env->state().discovery_active);
	EXPECT_EQ(env->state().stop_discovery_calls, 1u);
}

TEST_F(BluetoothControllerTest, GetDiscoveredDevicesPopulatesVector) {
	BTAudioController& audio = BTAudioController::get_instance();
	std::vector<BlueZDevice> devices;
	int64_t result = audio.get_discovered_devices(devices);
	EXPECT_EQ(result, 0);
	ASSERT_EQ(devices.size(), 1u);
	EXPECT_STREQ(devices[0].name, kDeviceName);
}

TEST_F(BluetoothControllerTest, FindDeviceIdxLocatesDevice) {
	BTAudioController& audio = BTAudioController::get_instance();
	std::vector<BlueZDevice> devices(1, make_device(env->state()));
	EXPECT_EQ(audio.find_device_idx(devices, kDeviceAddress), 0);
	EXPECT_EQ(audio.find_device_idx(devices, "00:00:00:00:00:00"), -1);
}

TEST_F(BluetoothControllerTest, PairDeviceMarksState) {
	BTAudioController& audio = BTAudioController::get_instance();
	BlueZDevice device = make_device(env->state());
	EXPECT_EQ(audio.pair_device(device), 0);
	EXPECT_TRUE(env->state().device_paired);
	EXPECT_EQ(env->state().pair_calls, 1u);
}

TEST_F(BluetoothControllerTest, ConnectDeviceMarksState) {
	BTAudioController& audio = BTAudioController::get_instance();
	BlueZDevice device = make_device(env->state());
	audio.pair_device(device);
	EXPECT_EQ(audio.connect_device(device), 0);
	EXPECT_TRUE(env->state().device_connected);
	EXPECT_EQ(env->state().connect_calls, 1u);
}

TEST_F(BluetoothControllerTest, DisconnectDeviceClearsConnection) {
	BTAudioController& audio = BTAudioController::get_instance();
	BlueZDevice device = make_device(env->state());
	audio.pair_device(device);
	audio.connect_device(device);
	EXPECT_EQ(audio.disconnect_device(device), 0);
	EXPECT_FALSE(env->state().device_connected);
	EXPECT_EQ(env->state().disconnect_calls, 1u);
}

TEST_F(BluetoothControllerTest, IsPairedReflectsState) {
	BTAudioController& audio = BTAudioController::get_instance();
	BlueZDevice device = make_device(env->state());
	EXPECT_FALSE(audio.is_paired(device));
	env->state().device_paired = true;
	EXPECT_TRUE(audio.is_paired(device));
}

TEST_F(BluetoothControllerTest, IsConnectedReflectsState) {
	BTAudioController& audio = BTAudioController::get_instance();
	BlueZDevice device = make_device(env->state());
	EXPECT_FALSE(audio.is_connected(device));
	env->state().device_connected = true;
	EXPECT_TRUE(audio.is_connected(device));
}

TEST_F(BluetoothControllerTest, GetBooleanValueHandlesVariants) {
	BTAudioController& audio = BTAudioController::get_instance();
	GVariant* true_variant = g_variant_new("(v)", g_variant_new_variant(g_variant_new_boolean(TRUE)));
	EXPECT_TRUE(audio.get_boolean_value(true_variant));
	g_variant_unref(true_variant);
	GVariant* false_variant = g_variant_new("(v)", g_variant_new_variant(g_variant_new_boolean(FALSE)));
	EXPECT_FALSE(audio.get_boolean_value(false_variant));
	g_variant_unref(false_variant);
	EXPECT_FALSE(audio.get_boolean_value(nullptr));
	GVariant* empty_tuple = g_variant_new("()");
	EXPECT_FALSE(audio.get_boolean_value(empty_tuple));
}

TEST_F(BluetoothControllerTest, CleanupClearsState) {
	BTAudioController& audio = BTAudioController::get_instance();
	std::vector<BlueZDevice> devices;
	devices.push_back(make_device(env->state()));
	audio.connected_device = &devices[0];
	audio.cleanup(devices);
	EXPECT_TRUE(devices.empty());
	EXPECT_EQ(audio.connected_device, nullptr);
}

TEST_F(BluetoothControllerTest, BleServerInitSucceeds) {
	BLEServer& server = BLEServer::get_instance();
	EXPECT_EQ(server.init_for_test(), 0);
	server.cleanup();
}

TEST(BLEServerInitFailure, ReturnsErrorWhenBusUnavailable) {
	BLEServer& server = BLEServer::get_instance();
	gchar* prev_system = g_strdup(g_getenv("DBUS_SYSTEM_BUS_ADDRESS"));
	gchar* prev_g_system = g_strdup(g_getenv("G_DBUS_SYSTEM_BUS_ADDRESS"));
	g_setenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/nonexistent-bus", TRUE);
	g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/tmp/nonexistent-bus", TRUE);
	EXPECT_EQ(server.init_for_test(), -1);
	server.cleanup();
	if (prev_system) {
		g_setenv("DBUS_SYSTEM_BUS_ADDRESS", prev_system, TRUE);
		g_free(prev_system);
	}
	else {
		g_unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
	}
	if (prev_g_system) {
		g_setenv("G_DBUS_SYSTEM_BUS_ADDRESS", prev_g_system, TRUE);
		g_free(prev_g_system);
	}
	else {
		g_unsetenv("G_DBUS_SYSTEM_BUS_ADDRESS");
	}
}

TEST_F(BluetoothControllerTest, RegisterApplicationExportsInterfaces) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application_for_test(), 0);
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, SERVICE_PATH,
		"org.freedesktop.DBus.Properties", "Get", g_variant_new("(ss)", GATT_SERVICE_IFACE, "UUID"),
		G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	ASSERT_NE(result, nullptr);
	g_variant_unref(result);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, AdvertiseApplicationConfiguresAdapter) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application_for_test(), 0);
	EXPECT_EQ(server.advertise_application_for_test(), 0);
	EXPECT_TRUE(env->state().adapter_powered);
	EXPECT_TRUE(env->state().adapter_discoverable);
	EXPECT_TRUE(env->state().adapter_pairable);
	EXPECT_EQ(env->state().register_app_calls, 1u);
	EXPECT_EQ(env->state().register_adv_calls, 1u);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, CharacteristicReadUsesTestProvider) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application_for_test(), 0);
	BLEServer::set_test_response_provider([]() { return std::string("OK"); });
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
	GVariant* options = g_variant_builder_end(&builder);
	GVariant* params = g_variant_new("(@a{sv})", g_variant_ref(options));
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, CHARACTERISTIC_PATH,
		GATT_CHARACTERISTIC_IFACE, "ReadValue", params, G_VARIANT_TYPE("(ay)"), G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	ASSERT_NE(result, nullptr);
	GVariant* byte_array = g_variant_get_child_value(result, 0);
	gsize length = 0;
	const guint8* data = static_cast<const guint8*>(g_variant_get_fixed_array(byte_array, &length, sizeof(guint8)));
	ASSERT_EQ(length, 2u);
	EXPECT_EQ(std::string(reinterpret_cast<const char*>(data), length), "OK");
	g_variant_unref(byte_array);
	g_variant_unref(result);
	g_variant_unref(options);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, CharacteristicWriteInvokesHandler) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application_for_test(), 0);
	std::string captured;
	BLEServer::set_test_command_handler([&captured](const std::string& command) {
		captured = command;
	});
	GVariantBuilder value_builder;
	g_variant_builder_init(&value_builder, G_VARIANT_TYPE("ay"));
	const std::string payload = "CMD";
	for (char c : payload) {
		g_variant_builder_add(&value_builder, "y", (guint8)c);
	}
	GVariant* value = g_variant_builder_end(&value_builder);
	g_variant_builder_init(&value_builder, G_VARIANT_TYPE("a{sv}"));
	GVariant* options = g_variant_builder_end(&value_builder);
	GVariant* params = g_variant_new("(@ay@a{sv})", g_variant_ref(value), g_variant_ref(options));
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, CHARACTERISTIC_PATH,
		GATT_CHARACTERISTIC_IFACE, "WriteValue", params, nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	EXPECT_TRUE(error == nullptr);
	if (result) {
		EXPECT_EQ(g_variant_n_children(result), 0u);
		g_variant_unref(result);
	}
	EXPECT_EQ(captured, payload);
	g_variant_unref(value);
	g_variant_unref(options);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, AdvertisementUnknownPropertyReturnsError) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application_for_test(), 0);
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, ADVERTISING_PATH,
		"org.freedesktop.DBus.Properties", "Get", g_variant_new("(ss)", "org.bluez.LEAdvertisement1", "Unknown"),
		nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	EXPECT_EQ(result, nullptr);
	EXPECT_NE(error, nullptr);
	g_clear_error(&error);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, ServiceUnknownPropertyReturnsError) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application(), 0);
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, SERVICE_PATH,
		"org.freedesktop.DBus.Properties", "Get", g_variant_new("(ss)", GATT_SERVICE_IFACE, "Unknown"),
		nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	EXPECT_EQ(result, nullptr);
	EXPECT_NE(error, nullptr);
	g_clear_error(&error);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, CharacteristicUnknownPropertyReturnsError) {
	BLEServer& server = BLEServer::get_instance();
	ASSERT_EQ(server.init_for_test(), 0);
	ASSERT_EQ(server.register_application(), 0);
	GError* error = nullptr;
	GVariant* result = g_dbus_connection_call_sync(env->connection(), BLUEZ_SERVICE, CHARACTERISTIC_PATH,
		"org.freedesktop.DBus.Properties", "Get", g_variant_new("(ss)", GATT_CHARACTERISTIC_IFACE, "Unknown"),
		nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
	EXPECT_EQ(result, nullptr);
	EXPECT_NE(error, nullptr);
	g_clear_error(&error);
	server.cleanup();
}

TEST_F(BluetoothControllerTest, BleServerStartRunsSingleIteration) {
	BLEServer& server = BLEServer::get_instance();
	BLEServer::set_loop_iterations(1);
	BLEServer::skip_main_loop_for_tests(true);
	ASSERT_NO_FATAL_FAILURE(server.start());
	EXPECT_TRUE(env->state().adapter_powered);
	server.cleanup();
}

