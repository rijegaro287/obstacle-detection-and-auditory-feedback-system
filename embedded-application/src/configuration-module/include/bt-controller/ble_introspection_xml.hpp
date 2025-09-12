#pragma once

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
