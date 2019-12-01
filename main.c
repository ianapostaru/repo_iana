#include <glib.h>
#include <stdio.h>
#include "handle_device.h"

#define NB_OF_DEVICES 2

/*
The problem to solve is following:
                You monitor devices, which are sending data to you.
                Each device have a unique name.
                Each device produces measurements.

Challange is:
                Compute number of messages you got or read from the devices.

The solution can be in any language (preferably C/C++).
The scope is open, you must decide how the “devices” will work in your system.
The solution should be posted on GitHub or a similar page for a review.
Please add Makefile and documentation explaining us how to run your code.
*/

/**
 * @brief Static method used as a callback for SIGINT process signal
 * @param[in] user_data:  data passed to the callback method
 * @return always returns TRUE
 */
static gboolean measurement_signal_cb(gpointer user_data);

/**
 * @brief Static method used as a callback for SIGINT process signal
 * @param[in] user_data:  data passed to the callback method
 * @return always returns TRUE
 */
static gboolean measurement_signal_cb(gpointer user_data)
{
    printf("main(): Shutting down...\n");

    g_main_loop_quit((GMainLoop*) user_data);

    return TRUE;
}

/**
 * @brief Main method of the measurement process
 * @return EXIT_SUCCESS if everything went OK, EXIT_FAILURE otherwise
 */
int main(void)
{
    int exit_code = EXIT_FAILURE;
    GMainLoop* loop = NULL;
    device_types monitored_devices[NB_OF_DEVICES] = {TEMPERATURE, HUMIDITY};

    printf("main(): Measurement process started\n");

    loop = g_main_loop_new(NULL, FALSE);

    if (NULL == loop)
    {
        printf("main(): Failed to create main loop\n");
    }
    else
    {
        if (TRUE == initialize_devices(monitored_devices, sizeof(monitored_devices)/sizeof(monitored_devices[0])))
        {
            guint sigint_source_id = 0;

            sigint_source_id = g_unix_signal_add(SIGINT, &measurement_signal_cb, loop); /* ctrl+c to finish the running program */

            g_main_loop_run(loop);

            (void)g_source_remove(sigint_source_id);

            exit_code = EXIT_SUCCESS;
        }

        g_main_loop_unref(loop);
    }

    if(EXIT_SUCCESS == exit_code)
    {
        deinitialize_devices(monitored_devices, sizeof(monitored_devices)/sizeof(monitored_devices[0]));
    }

    printf("main(): Measurement process terminated\n");

    return exit_code;
}