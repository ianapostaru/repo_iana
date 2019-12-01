#ifndef HANDLE_DEVICE_H_
#define HANDLE_DEVICE_H_

#include <glib.h>
#include <glib-unix.h>

/* each device have a unique name, simillar to unique values from enum */
typedef enum {
	TEMPERATURE = 0,
	HUMIDITY = 1
} device_types;

/**
 * @brief Method used to initialize one device
 * @param[in]  devices:       array with monitored devices to initialize
 * @param[in]  nb_ofdevices:  number of devices from array
 * @return TRUE if the initialization was OK, FALSE otherwise
 */
gboolean initialize_devices(device_types *devices, guint nb_of_devices);

/**
 * @brief Method used to deinitialize one device
 * @param[in]  devices:       array with monitored devices to deinitialize
 * @param[in]  nb_ofdevices:  number of devices from array
 * @return none.
 */
void deinitialize_devices(device_types *devices, guint nb_of_devices);

#endif /* HANDLE_DEVICE_H_ */