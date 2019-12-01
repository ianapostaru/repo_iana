#include "handle_device.h"
#include <string.h>              /* strlen    */
#include <stdlib.h>
#include <stdio.h>

/* the file names for each device, from where are sent data */
/* each line represent a new input for measurements */
#define TEMPERATURE_INPUT "temperature_device.txt"
#define HUMIDITY_INPUT "humidity_device.txt"

/* the maximum length of one input from a device, 
equivalent to maximum number of charactes from one line from txt file
the max value of int on 4 bytes is: 2147483647;
10 + one byte for sign (negative values) + one byte for \n */
#define MAX_VALUE_LENGTH 12

/* when is received a bigger value than 99 degrees from temperature device,
is stopped monitoring process for it */
#define MAX_TEMPERATURE_AVAILABLE 99

/* when is received a bigger value than 500 from humidity device,
is stopped monitoring process for it */
#define MAX_HUMIDITY_AVAILABLE 500

/* tests are running for 100 seconds, at each 5 seconds are prints stored data */
#define RUNNING_TIME 100

/**
 * @brief - callback function on device thread creation
 * @param[in] 
 * @return gpointer
 */
typedef gpointer (*thread_creation_function_cb)(gpointer data);

static GThread *temperature_thread = NULL;
static GThread *humidity_thread = NULL;
static GThread *providing_data_thread = NULL;
static GMutex manage_data_mutex;
static GHashTable *table_with_received_data = NULL;

/**
 * @brief Method used to destroy the hash table
 * @param[in]  key:           key from hash table
 * @param[in]  value:         pointer to the list, that is value of provided key
 * @param[in]  data:          first elemnt from the list
 * @return none
 */
static void destroy_one_list(gpointer key, gpointer value, gpointer data);

/**
 * @brief Method used to print the hash table
 * @param[in]  key:           key from hash table
 * @param[in]  value:         pointer to the list, that is value of provided key
 * @param[in]  data:          first elemnt from the list
 * @return none
 */
static void print_one_list(gpointer key, gpointer value, gpointer data);

/**
 * @brief Method used to show number of messages received from each monitored device
 * @return none.
 */
static gpointer analyze_measurements_values(gpointer data);

/**
 * @brief Method used to initialize preview handler for received data
 * @return TRUE if the initialization was OK, FALSE otherwise
 */
static gboolean initialize_providing_data_handler(void);

/**
 * @brief Method used to deinitialize the preview handler for received data
 * @return none
 */
static void deinitialize_providing_data_handler(void);

/**
 * @brief Method used to initialize one device
 * @param[in]  thread_name:   the thread name to be created
 * @param[in]  cb_function:   the callback function to be called on thread creation
 * @param[out] device_thread: the resulted running thread, or NULL if creation failed
 * @return none
 */
static gboolean initialize_one_device(const gchar *const thread_name, 
									  thread_creation_function_cb cb_function, 
									  GThread *device_thread);

/**
 * @brief Method used to deinitialize one device
 * @param[in]  device_thread: the device thread to deinitialize
 * @return none
 */
void deinitialize_one_device(GThread *device_thread);

/**
 * @brief     Function used as a callback for the temperature thread creation
 * @param[in] data - Data passed to the callback method.
 * @return    Thread callback method, so it will return always NULL.
 */
static gpointer temperature_device_cb(gpointer data);

/**
 * @brief     Function used as a callback for the humidity thread creation
 * @param[in] data - Data passed to the callback method.
 * @return    Thread callback method, so it will return always NULL.
 */
static gpointer humidity_device_cb(gpointer data);

/**
 * @brief     Function used as a callback for the temperature thread creation
 * @param[in] data - Data passed to the callback method.
 * @return    Thread callback method, so it will return always NULL.
 */
static gpointer temperature_device_cb(gpointer data)
{
	FILE *file = fopen(TEMPERATURE_INPUT, "r");
	gboolean continue_check = FALSE;

	printf("temperature_device_cb(): Start monitoring temperature device.\n"); 

	if(NULL == file)
	{
		printf("temperature_device_cb(): unable to open %s file\n", TEMPERATURE_INPUT);
    }
	else
	{
		g_mutex_lock(&manage_data_mutex);

		if (NULL == table_with_received_data)
		{
			printf("temperature_device_cb(): NULL index table. Please call initialize_devices.\n");
		}
		else
		{
			gchar char_data[MAX_VALUE_LENGTH];

			printf("temperature_device_cb(): Start parsing %s file.\n", TEMPERATURE_INPUT);
			continue_check = TRUE;

			while ((TRUE == continue_check) && (NULL != fgets(char_data, sizeof(char_data), file)))
			{
				gint int_data = 0;

				strtok(char_data, "\n");
				int_data = (gint)strtol(char_data, (char **) NULL, 10);

				if(MAX_TEMPERATURE_AVAILABLE < int_data)
				{
					printf("temperature_device_cb(): !!!! temperature device is in danger !!!! last recived value is: %d\n", 
						   int_data);
					continue_check = FALSE;
				}
				else
				{
					gchar* temperature_key_from_hash_table = "temperature";

					printf("temperature_device_cb(): registered %d value\n", int_data);
					g_hash_table_insert(table_with_received_data, temperature_key_from_hash_table,
							g_slist_append(g_hash_table_lookup(table_with_received_data, temperature_key_from_hash_table), char_data));
				}
			}
		}

		g_mutex_unlock(&manage_data_mutex);
	}
 
    fclose(file);

    return NULL;
}

/**
 * @brief     Function used as a callback for the humidity thread creation
 * @param[in] data - Data passed to the callback method.
 * @return    Thread callback method, so it will return always NULL.
 */
static gpointer humidity_device_cb(gpointer data)
{
    FILE *file = fopen(HUMIDITY_INPUT, "r");
	gboolean continue_check = FALSE;

	printf("humidity_device_cb(): Start monitoring temperature device.\n"); 

	if(NULL == file)
	{
		printf("humidity_device_cb(): unable to open %s file\n", TEMPERATURE_INPUT);
    }
	else
	{
		g_mutex_lock(&manage_data_mutex);

		if (NULL == table_with_received_data)
		{
			printf("humidity_device_cb(): NULL index table. Please call initialize_devices.\n");
		}
		else
		{
			gchar char_data[MAX_VALUE_LENGTH];

			printf("humidity_device_cb(): Start parsing %s file.\n", TEMPERATURE_INPUT);
			continue_check = TRUE;

			while ((TRUE == continue_check) && (NULL != fgets(char_data, sizeof(char_data), file)))
			{
				gint int_data = 0;

				strtok(char_data, "\n");
				int_data = (gint)strtol(char_data, (char **) NULL, 10);

				if(MAX_HUMIDITY_AVAILABLE < int_data)
				{
					printf("humidity_device_cb(): !!!! humidity device is in danger !!!! last recived value is: %d\n", 
						   int_data);
					continue_check = FALSE;
				}
				else
				{
					gchar* humidity_key_from_hash_table = "humidity";

					printf("humidity_device_cb(): registered %d value\n", int_data);
					g_hash_table_insert(table_with_received_data, humidity_key_from_hash_table,
							g_slist_append(g_hash_table_lookup(table_with_received_data, humidity_key_from_hash_table), char_data));
				}
			}
		}

		g_mutex_unlock(&manage_data_mutex);
	}
 
    fclose(file);

    return NULL;
}

/**
 * @brief Method used to initialize one device
 * @param[in]  thread_name:   the thread name to be created
 * @param[in]  cb_function:   the callback function to be called on thread creation
 * @param[out] device_thread: the resulted running thread, or NULL if creation failed
 * @return none
 */
static gboolean initialize_one_device(const gchar *const thread_name, 
									  thread_creation_function_cb cb_function, 
									  GThread *device_thread)
{
    gboolean is_device_initialized = FALSE;
	GError *error = NULL;

	device_thread = g_thread_try_new(thread_name, cb_function, NULL, &error);

	if (NULL == device_thread)
	{
		printf("initialize_one_device(): Failed to create thread, error %s\n", error->message);
	}
	else
	{
		printf("initialize_one_device(): Initialization succeeded\n");
		is_device_initialized = TRUE;
	}

    return is_device_initialized;
}

/**
 * @brief Method used to destroy the hash table
 * @param[in]  key:           key from hash table
 * @param[in]  value:         pointer to the list, that is value of provided key
 * @param[in]  data:          first elemnt from the list
 * @return none
 */
static void destroy_one_list(gpointer key, gpointer value, gpointer data)
{
	g_slist_free(value);
}

/**
 * @brief Method used to print the hash table
 * @param[in]  key:           key from hash table
 * @param[in]  value:         pointer to the list, that is value of provided key
 * @param[in]  data:          first elemnt from the list
 * @return none
 */
static void print_one_list(gpointer key, gpointer value, gpointer data)
{
	printf("print_one_list(): Value of %s key are: ", (gchar*)key);
	g_slist_foreach((GSList*)value, (GFunc)printf, NULL);
	printf("\n");
}

/**
 * @brief Method used to show number of messages received from each monitored device
 * @return none.
 */
static gpointer analyze_measurements_values(gpointer data)
{
	gint local_count = RUNNING_TIME;
	const guint period_of_analyze = 5;

	//while(0 < local_count)
	//{
		sleep(period_of_analyze);
		g_mutex_lock(&manage_data_mutex);

		if (NULL == table_with_received_data)
		{
			printf("analyze_measurements_values(): NULL index table. Please call initialize_devices.\n");
		}
		else
		{
			g_hash_table_foreach(table_with_received_data, print_one_list, NULL);
		}

		g_mutex_unlock(&manage_data_mutex);
		local_count -= period_of_analyze;
	//}

	return NULL;
}

/**
 * @brief Method used to initialize preview handler for received data
 * @return TRUE if the initialization was OK, FALSE otherwise
 */
static gboolean initialize_providing_data_handler(void)
{
    gboolean is_handler_initialized = FALSE;
	GError *error = NULL;
	const gchar *const thread_name = "view_thread";

	providing_data_thread = g_thread_try_new(thread_name, analyze_measurements_values, NULL, &error);

	if (NULL == providing_data_thread)
	{
		printf("initialize_providing_data_handler(): Failed to create thread, error %s\n", error->message);
	}
	else
	{
		printf("initialize_providing_data_handler(): Initialization succeeded\n");
		is_handler_initialized = TRUE;
	}

    return is_handler_initialized;
}

/**
 * @brief Method used to deinitialize the preview handler for received data
 * @return none
 */
static void deinitialize_providing_data_handler(void)
{
	printf("deinitialize_providing_data_handler(): Deinitialization started.\n");

	if (NULL != providing_data_thread)
	{
		(void)g_thread_join(providing_data_thread);
	}

    if (NULL != providing_data_thread)
    {
        g_thread_unref(providing_data_thread);
    }

    printf("deinitialize_providing_data_handler(): Deinitialization succeeded.\n");
}

/**
 * @brief Method used to deinitialize one device
 * @param[in]  device_thread: the device thread to deinitialize
 * @return none
 */
void deinitialize_one_device(GThread *device_thread)
{
	if (NULL != device_thread)
	{
		(void)g_thread_join(device_thread);
	}

    if (NULL != device_thread)
    {
        g_thread_unref(device_thread);
    }

    printf("deinitialize_one_device(): Deinitialization succeeded\n");
}

/**
 * @brief Method used to initialize one device
 * @param[in]  devices:        array with monitored devices to initialize
 * @param[in]  nb_ofdevices:   number of devices from array
 * @return TRUE if the initialization was OK, FALSE otherwise
 */
gboolean initialize_devices(device_types *devices, guint nb_of_devices)
{
    gboolean is_device_initialized = FALSE;
	guint devices_index = 0;

	g_mutex_lock(&manage_data_mutex);
	table_with_received_data = g_hash_table_new(g_str_hash, g_str_equal);
	g_mutex_unlock(&manage_data_mutex);

	if(TRUE == initialize_providing_data_handler())
	{
		for (devices_index = 0; devices_index < nb_of_devices; devices_index++)
		{

			switch (devices[devices_index])
			{
				case TEMPERATURE:
				{
					const gchar *const thread_name = "temperature_thread";
					
					printf("initialize_device(): Start initialization of temperature device.\n");
					is_device_initialized = initialize_one_device(thread_name, temperature_device_cb, temperature_thread);
				
					break;
				}
				case HUMIDITY:
				{
					const gchar *const thread_name = "humidity_thread";

					printf("initialize_device(): Start initialization of humidity device.\n");
					is_device_initialized = initialize_one_device(thread_name, humidity_device_cb, humidity_thread);

					break;
				}        
				default:
				{
					printf("initialize_device(): Invalid device type.\n");
					break;
				}
			}
		}
	}

    return is_device_initialized;
}

/**
 * @brief Method used to deinitialize one device
 * @param[in]  devices:        array with monitored devices to deinitialize
 * @param[in]  nb_ofdevices:   number of devices from array
 * @return none
 */
void deinitialize_devices(device_types *devices, guint nb_of_devices)
{    
	guint devices_index = 0;

	deinitialize_providing_data_handler();

	g_mutex_lock(&manage_data_mutex);

	if (NULL == table_with_received_data)
	{
		printf("deinitialize_devices(): NULL index table. Please call initialize_devices.\n");
	}
	else
	{
		g_hash_table_foreach(table_with_received_data, destroy_one_list, NULL);
 		g_hash_table_destroy(table_with_received_data);
	}

    g_mutex_unlock(&manage_data_mutex);

	for (devices_index = 0; devices_index < nb_of_devices; devices_index++)
	{
		switch (devices[devices_index])
		{
			case TEMPERATURE:
			{
				printf("deinitialize_device(): Start deinitialization of temperature device.\n");
				deinitialize_one_device(temperature_thread);

				break;
			}
			case HUMIDITY:
			{
				printf("deinitialize_device(): Start deinitialization of humidity device.\n");
				deinitialize_one_device(humidity_thread);

				break;
			}        
			default:
			{
				printf("deinitialize_device(): Invalid device type.");
				break;
			}
		}
	}
}