#include "hid_msc.h"

bool current_switch_state = false;
bool Hid_disconnected = false;

const char *TAG = "example";
QueueHandle_t hid_host_event_queue;
bool user_shutdown = false;

usb_tasks cusb_task = {usb_tasks::HID, false};

/**
   @brief Makes new line depending on report output protocol type

   @param[in] proto Current protocol to output
*/
static void hid_print_new_device_report_header(hid_protocol_t proto)
{
    static hid_protocol_t prev_proto_output = HID_PROTOCOL_MAX;

    if (prev_proto_output != proto)
    {
        prev_proto_output = proto;
        printf("\r\n");
        if (proto == HID_PROTOCOL_MOUSE)
        {
            printf("Mouse\r\n");
        }
        else if (proto == HID_PROTOCOL_KEYBOARD)
        {
            printf("Keyboard\r\n");
        }
        else
        {
            printf("Generic\r\n");
        }
        fflush(stdout);
    }
}

/**
   @brief HID Keyboard modifier verification for capitalization application
   (right or left shift)

   @param[in] modifier
   @return true  Modifier was pressed (left or right shift)
   @return false Modifier was not pressed (left or right shift)

*/
static inline bool hid_keyboard_is_modifier_shift(uint8_t modifier)
{
    if (((modifier & HID_LEFT_SHIFT) == HID_LEFT_SHIFT) ||
        ((modifier & HID_RIGHT_SHIFT) == HID_RIGHT_SHIFT))
    {
        return true;
    }
    return false;
}

/**
   @brief HID Keyboard get char symbol from key code

   @param[in] modifier  Keyboard modifier data
   @param[in] key_code  Keyboard key code
   @param[in] key_char  Pointer to key char data

   @return true  Key scancode converted successfully
   @return false Key scancode unknown
*/
static inline bool hid_keyboard_get_char(uint8_t modifier, uint8_t key_code,
                                         unsigned char *key_char)
{
    uint8_t mod = (hid_keyboard_is_modifier_shift(modifier)) ? 1 : 0;

    if ((key_code >= HID_KEY_A) && (key_code <= HID_KEY_SLASH))
    {
        *key_char = keycode2ascii[key_code][mod];
    }
    else
    {
        // All other key pressed
        return false;
    }

    return true;
}

/**
   @brief HID Keyboard print char symbol

   @param[in] key_char  Keyboard char to stdout
*/
static inline void hid_keyboard_print_char(unsigned int key_char)
{
    if (!!key_char)
    {
        putchar(key_char);
#if (KEYBOARD_ENTER_LF_EXTEND)
        if (KEYBOARD_ENTER_MAIN_CHAR == key_char)
        {
            putchar('\n');
        }
#endif // KEYBOARD_ENTER_LF_EXTEND
        fflush(stdout);
    }
}

/**
   @brief Key Event. Key event with the key code, state and modifier.

   @param[in] key_event Pointer to Key Event structure

*/
static void key_event_callback(key_event_t *key_event)
{
    unsigned char key_char;

    hid_print_new_device_report_header(HID_PROTOCOL_KEYBOARD);

    if (key_event->KEY_STATE_PRESSED == key_event->state)
    {
        hid_keyboard_get_char(key_event->modifier, key_event->key_code,
                              &key_char);

        // Serial.println("[" + String(__FUNCTION__) + "] : " + "key_char: " + String(key_char));
        // Serial.println("[" + String(__FUNCTION__) + "] : " + "modifier: " + String(key_event->modifier));
        // Serial.println("[" + String(__FUNCTION__) + "] : " + "key_code: " + String(key_event->key_code));

        onKeyboardKey(key_char, key_event->key_code, key_event->modifier);

        // {

        //     hid_keyboard_print_char(key_char);
        // }
    }
}

/**
   @brief Key buffer scan code search.

   @param[in] src       Pointer to source buffer where to search
   @param[in] key       Key scancode to search
   @param[in] length    Size of the source buffer
*/
static inline bool key_found(const uint8_t *const src, uint8_t key,
                             unsigned int length)
{
    for (unsigned int i = 0; i < length; i++)
    {
        if (src[i] == key)
        {
            return true;
        }
    }
    return false;
}

/**
   @brief USB HID Host Keyboard Interface report callback handler

   @param[in] data    Pointer to input report data buffer
   @param[in] length  Length of input report data buffer
*/
static void hid_host_keyboard_report_callback(const uint8_t *const data,
                                              const int length)
{
    hid_keyboard_input_report_boot_t *kb_report =
        (hid_keyboard_input_report_boot_t *)data;

    if (length < sizeof(hid_keyboard_input_report_boot_t))
    {
        return;
    }

    static uint8_t prev_keys[HID_KEYBOARD_KEY_MAX] = {0};
    key_event_t key_event;

    for (int i = 0; i < HID_KEYBOARD_KEY_MAX; i++)
    {

        // key has been released verification
        if (prev_keys[i] > HID_KEY_ERROR_UNDEFINED &&
            !key_found(kb_report->key, prev_keys[i], HID_KEYBOARD_KEY_MAX))
        {
            key_event.key_code = prev_keys[i];
            key_event.modifier = 0;
            key_event.state = key_event.KEY_STATE_RELEASED;
            key_event_callback(&key_event);
        }

        // key has been pressed verification
        if (kb_report->key[i] > HID_KEY_ERROR_UNDEFINED &&
            !key_found(prev_keys, kb_report->key[i], HID_KEYBOARD_KEY_MAX))
        {
            key_event.key_code = kb_report->key[i];
            key_event.modifier = kb_report->modifier.val;
            key_event.state = key_event.KEY_STATE_PRESSED;
            key_event_callback(&key_event);
        }
    }

    memcpy(prev_keys, &kb_report->key, HID_KEYBOARD_KEY_MAX);
}

#ifdef CONFIG_HID_HOST_MOUSE_SUPPORT

/**
   @brief USB HID Host Mouse Interface report callback handler

   @param[in] data    Pointer to input report data buffer
   @param[in] length  Length of input report data buffer
*/
static void hid_host_mouse_report_callback(const uint8_t *const data,
                                           const int length)
{
    hid_mouse_input_report_boot_t *mouse_report =
        (hid_mouse_input_report_boot_t *)data;

    if (length < sizeof(hid_mouse_input_report_boot_t))
    {
        return;
    }

    static int x_pos = 0;
    static int y_pos = 0;

    // Calculate absolute position from displacement
    x_pos += mouse_report->x_displacement;
    y_pos += mouse_report->y_displacement;

    hid_print_new_device_report_header(HID_PROTOCOL_MOUSE);

    printf("X: %06d\tY: %06d\t|%c|%c|\r", x_pos, y_pos,
           (mouse_report->buttons.button1 ? 'o' : ' '),
           (mouse_report->buttons.button2 ? 'o' : ' '));
    fflush(stdout);
}

#endif // CONFIG_HID_HOST_MOUSE_SUPPORT

/**
   @brief USB HID Host Generic Interface report callback handler

   'generic' means anything else than mouse or keyboard

   @param[in] data    Pointer to input report data buffer
   @param[in] length  Length of input report data buffer
*/
static void hid_host_generic_report_callback(const uint8_t *const data,
                                             const int length)
{
    hid_print_new_device_report_header(HID_PROTOCOL_NONE);
    for (int i = 0; i < length; i++)
    {
        printf("%02X", data[i]);
    }
    putchar('\r');
    putchar('\n');
    fflush(stdout);
}

/**
   @brief USB HID Host interface callback

   @param[in] hid_device_handle  HID Device handle
   @param[in] event              HID Host interface event
   @param[in] arg                Pointer to arguments, does not used
*/
void hid_host_interface_callback(hid_host_device_handle_t hid_device_handle,
                                 const hid_host_interface_event_t event,
                                 void *arg)
{
    uint8_t data[64] = {0};
    size_t data_length = 0;
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));

    switch (event)
    {
    case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
        ESP_ERROR_CHECK(hid_host_device_get_raw_input_report_data(
            hid_device_handle, data, 64, &data_length));

        if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class)
        {
            if (HID_PROTOCOL_KEYBOARD == dev_params.proto)
            {
                hid_host_keyboard_report_callback(data, data_length);
            }
            else if (HID_PROTOCOL_MOUSE == dev_params.proto)
            {
#ifdef CONFIG_HID_HOST_MOUSE_SUPPORT
                hid_host_mouse_report_callback(data, data_length);
#else
                ESP_LOGI(TAG, "No Mouse Support here");
#endif
            }
        }
        else
        {
            hid_host_generic_report_callback(data, data_length);
        }

        break;
    case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HID Device, protocol '%s' DISCONNECTED",
                 hid_proto_name_str[dev_params.proto]);
        ESP_ERROR_CHECK(hid_host_device_close(hid_device_handle));
        Hid_disconnected = true;
        break;
    case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
        ESP_LOGI(TAG, "HID Device, protocol '%s' TRANSFER_ERROR",
                 hid_proto_name_str[dev_params.proto]);
        Hid_disconnected = false;
        break;
    default:
        ESP_LOGE(TAG, "HID Device, protocol '%s' Unhandled event",
                 hid_proto_name_str[dev_params.proto]);
        Hid_disconnected = false;
        break;
    }
}

/**
   @brief USB HID Host Device event

   @param[in] hid_device_handle  HID Device handle
   @param[in] event              HID Host Device event
   @param[in] arg                Pointer to arguments, does not used
*/
void hid_host_device_event(hid_host_device_handle_t hid_device_handle,
                           const hid_host_driver_event_t event, void *arg)
{
    hid_host_dev_params_t dev_params;
    ESP_ERROR_CHECK(hid_host_device_get_params(hid_device_handle, &dev_params));
    const hid_host_device_config_t dev_config = {
        .callback = hid_host_interface_callback, .callback_arg = NULL};

    switch (event)
    {
    case HID_HOST_DRIVER_EVENT_CONNECTED:
        ESP_LOGI(TAG, "HID Device, protocol '%s' CONNECTED",
                 hid_proto_name_str[dev_params.proto]);

        ESP_ERROR_CHECK(hid_host_device_open(hid_device_handle, &dev_config));
        if (HID_SUBCLASS_BOOT_INTERFACE == dev_params.sub_class)
        {
            ESP_ERROR_CHECK(hid_class_request_set_protocol(hid_device_handle,
                                                           HID_REPORT_PROTOCOL_BOOT));
            if (HID_PROTOCOL_KEYBOARD == dev_params.proto)
            {
                ESP_ERROR_CHECK(hid_class_request_set_idle(hid_device_handle, 0, 0));
            }
        }
        ESP_ERROR_CHECK(hid_host_device_start(hid_device_handle));
        break;
    default:
        break;
    }
}

/**
   @brief Start USB Host install and handle common USB host library events while
   app pin not low

   @param[in] arg  Not used
*/
static void usb_lib_task(void *arg)
{

    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };

    ESP_ERROR_CHECK(usb_host_install(&host_config));
    xTaskNotifyGive((TaskHandle_t)arg);

    while (digitalRead(APP_QUIT_PIN) != 0)
    {
        uint32_t event_flags;
        esp_err_t err = usb_host_lib_handle_events(100, &event_flags);
        // Release devices once all clients has deregistered
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            usb_host_device_free_all();
            ESP_LOGI(TAG, "USB Event flags: NO_CLIENTS");
        }
        // All devices were removed
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            ESP_LOGI(TAG, "USB Event flags: ALL_FREE");
        }
    }
    // App Button was pressed, trigger the flag
    user_shutdown = true;
    ESP_LOGI(TAG, "USB shutdown");
    // Clean up USB Host
    vTaskDelay(1000); // Short delay to allow clients clean-up
    esp_err_t err = usb_host_uninstall();
    if (err != ESP_OK)
    {
        Serial.println("USB host uninstall error");
        Serial.println(err);
    };

    cusb_task.status = false;

    vTaskDelete(NULL);
}

esp_err_t sendHIDReport(hid_host_device_handle_t hid_dev_handle, uint8_t report_type, uint8_t report_id, uint8_t *report, size_t report_length)
{
    return hid_class_request_set_report(hid_dev_handle, report_type, report_id, report, report_length);
}

/**
   @brief HID Host main task

   Creates queue and get new event from the queue

   @param[in] pvParameters Not used
*/
void hid_host_task(void *pvParameters)
{
    hid_host_event_queue_t evt_queue;
    // Create queue
    hid_host_event_queue = xQueueCreate(10, sizeof(hid_host_event_queue_t));

    // Wait queue
    while (!user_shutdown)
    {
        if (xQueueReceive(hid_host_event_queue, &evt_queue, pdMS_TO_TICKS(50)))
        {
            hid_host_device_event(evt_queue.hid_device_handle, evt_queue.event,
                                  evt_queue.arg);
        }
    }
    ESP_LOGI(TAG, "HID Driver uninstall");

    esp_err_t err = hid_host_uninstall();

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HID Host uninstall error %d", err);
    }
    else
    {
        ESP_LOGI(TAG, "HID Host uninstalled successfully");
    }

    xQueueReset(hid_host_event_queue);
    vQueueDelete(hid_host_event_queue);
    cusb_task.status = false;
    vTaskDelete(NULL);
}

/**
   @brief HID Host Device callback

   Puts new HID Device event to the queue

   @param[in] hid_device_handle HID Device handle
   @param[in] event             HID Device event
   @param[in] arg               Not used
*/
void hid_host_device_callback(hid_host_device_handle_t hid_device_handle,
                              const hid_host_driver_event_t event, void *arg)
{
    const hid_host_event_queue_t evt_queue = {
        .hid_device_handle = hid_device_handle, .event = event, .arg = arg};
    xQueueSend(hid_host_event_queue, &evt_queue, 0);
}

void app_main(void)
{
    BaseType_t task_created;
    ESP_LOGI(TAG, "HID Host example");

    /*
       Create usb_lib_task to:
       - initialize USB Host library
       - Handle USB Host events while APP pin in in HIGH state
    */
    task_created =
        xTaskCreatePinnedToCore(usb_lib_task, "usb_events", 4096,
                                xTaskGetCurrentTaskHandle(), 2, NULL, 0);
    assert(task_created == pdTRUE);

    // Wait for notification from usb_lib_task to proceed
    ulTaskNotifyTake(false, 1000);

    /*
       HID host driver configuration
       - create background task for handling low level event inside the HID driver
       - provide the device callback to get new HID Device connection event
    */
    const hid_host_driver_config_t hid_host_driver_config = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = 0,
        .callback = hid_host_device_callback,
        .callback_arg = NULL};

    ESP_ERROR_CHECK(hid_host_install(&hid_host_driver_config));

    // Task is working until the devices are gone (while 'user_shutdown' if false)
    user_shutdown = false;

    /*
       Create HID Host task process for handle events
       IMPORTANT: Task is necessary here while there is no possibility to interact
       with USB device from the callback.
    */
    task_created =
        xTaskCreate(&hid_host_task, "hid_task", 4 * 1024, NULL, 2, NULL);
    assert(task_created == pdTRUE);

    cusb_task = {usb_tasks::HID, true};
}

// MSC TASKS

// #define USB_DISCONNECT_PIN GPIO_NUM_10

#define READY_TO_UNINSTALL (HOST_NO_CLIENT | HOST_ALL_FREE)

typedef enum
{                            ///
    MSC_DEVICE_CONNECTED,    /**< MSC device has been connected to the system.*/
    MSC_DEVICE_DISCONNECTED, /**< MSC device has been disconnected from the system.*/
} event;                     ///

typedef enum
{
    HOST_NO_CLIENT = 0x1,
    HOST_ALL_FREE = 0x2,
    DEVICE_CONNECTED = 0x4,
    DEVICE_DISCONNECTED = 0x8,
    DEVICE_ADDRESS_MASK = 0xFF0,
} app_event_t;

static EventGroupHandle_t usb_flags;

static void msc_event_cb(const msc_host_event_t *event, void *arg)
{
    if (event->event == MSC_DEVICE_CONNECTED)
    {
        ESP_LOGI(TAG, "MSC device connected");
        // Obtained USB device address is placed after application events
        xEventGroupSetBits(usb_flags, DEVICE_CONNECTED | (event->device.address << 4));
    }
    else if (event->event == MSC_DEVICE_DISCONNECTED)
    {
        xEventGroupSetBits(usb_flags, DEVICE_DISCONNECTED);
        ESP_LOGI(TAG, "MSC device disconnected");
    }
}

static void print_device_info(msc_host_device_info_t *info)
{
    const size_t megabyte = 1024 * 1024;
    uint64_t capacity = ((uint64_t)info->sector_size * info->sector_count) / megabyte;

    printf("Device info:\n");
    printf("\t Capacity: %llu MB\n", capacity);
    printf("\t Sector size: %" PRIu32 "\n", info->sector_size);
    printf("\t Sector count: %" PRIu32 "\n", info->sector_count);
    printf("\t PID: 0x%4X \n", info->idProduct);
    printf("\t VID: 0x%4X \n", info->idVendor);
    wprintf(L"\t iProduct: %S \n", info->iProduct);
    wprintf(L"\t iManufacturer: %S \n", info->iManufacturer);
    wprintf(L"\t iSerialNumber: %S \n", info->iSerialNumber);
}

static bool file_exists(const char *file_path)
{
    struct stat buffer;
    return stat(file_path, &buffer) == 0;
}

static void file_operations(void)
{
    const char *directory = "/usb/esp";
    const char *file_path = "/usb/esp/test.txt";

    struct stat s = {0};
    bool directory_exists = stat(directory, &s) == 0;
    if (!directory_exists)
    {
        if (mkdir(directory, 0775) != 0)
        {
            ESP_LOGE(TAG, "mkdir failed with errno: %s\n", strerror(errno));
        }
    }

    if (!file_exists(file_path))
    {
        ESP_LOGI(TAG, "Creating file");
        FILE *f = fopen(file_path, "w");
        if (f == NULL)
        {
            ESP_LOGE(TAG, "Failed to open file for writing");
            return;
        }
        fprintf(f, "Hello World!\n");
        fclose(f);
    }

    FILE *f;
    ESP_LOGI(TAG, "Reading file");
    f = fopen(file_path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);
}

// Handles common USB host library events
static void handle_usb_events(void *args)
{
    while (1)
    {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

        // Release devices once all clients has deregistered
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            usb_host_device_free_all();
            xEventGroupSetBits(usb_flags, HOST_NO_CLIENT);
        }
        // Give ready_to_uninstall_usb semaphore to indicate that USB Host library
        // can be deinitialized, and terminate this task.
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            xEventGroupSetBits(usb_flags, HOST_ALL_FREE);
        }
    }

    vTaskDelete(NULL);
}

static uint8_t wait_for_msc_device(void)
{
    EventBits_t event;

    ESP_LOGI(TAG, "Waiting for USB stick to be connected");
    event = xEventGroupWaitBits(usb_flags, DEVICE_CONNECTED | DEVICE_ADDRESS_MASK,
                                pdTRUE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "connection...");
    // Extract USB device address from event group bits
    return (event & DEVICE_ADDRESS_MASK) >> 4;
}

static bool wait_for_event(EventBits_t event, TickType_t timeout)
{
    return xEventGroupWaitBits(usb_flags, event, pdTRUE, pdTRUE, timeout) & event;
}

void msc_device_loop(void *param)
{
    msc_host_device_handle_t msc_device;
    msc_host_vfs_handle_t vfs_handle;
    msc_host_device_info_t info;

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
        .allocation_unit_size = 1024,
    };

    while (digitalRead(APP_QUIT_PIN) != 0)
    {
        uint8_t device_address = wait_for_msc_device();

        ESP_ERROR_CHECK(msc_host_install_device(device_address, &msc_device));
        msc_host_print_descriptors(msc_device);

        ESP_ERROR_CHECK(msc_host_get_device_info(msc_device, &info));
        print_device_info(&info);

        ESP_ERROR_CHECK(msc_host_vfs_register(msc_device, "/usb", &mount_config, &vfs_handle));

        while (!wait_for_event(DEVICE_DISCONNECTED, 200))
        {
            Serial.println(".");
            delay(100);
        }

        xEventGroupClearBits(usb_flags, READY_TO_UNINSTALL);
        ESP_ERROR_CHECK(msc_host_vfs_unregister(vfs_handle));
        ESP_ERROR_CHECK(msc_host_uninstall_device(msc_device));
    }

    // Cleanup after quitting the loop
    ESP_LOGI(TAG, "Uninitializing USB ...");
    esp_err_t err = msc_host_uninstall();
    if (err != ESP_OK)
    {
        Serial.println("Err : " + String(err));
    }
    wait_for_event(READY_TO_UNINSTALL, portMAX_DELAY);
    vTaskDelay(1000); // Short delay to allow clients clean-up
    err = usb_host_uninstall();
    if (err != ESP_OK)
    {
        Serial.println("Err : " + String(err));
    }
    cusb_task.status = false;
    ESP_LOGI(TAG, "Done");

    vTaskDelete(NULL); // Delete this task when done
}

void msc_setup(void)
{
    pinMode(APP_QUIT_PIN, INPUT_PULLUP);

    usb_flags = xEventGroupCreate();
    assert(usb_flags);

    const usb_host_config_t host_config = {.intr_flags = ESP_INTR_FLAG_LEVEL1};
    ESP_ERROR_CHECK(usb_host_install(&host_config));
    assert(xTaskCreate(handle_usb_events, "usb_events", 2048, NULL, 2, NULL));

    const msc_host_driver_config_t msc_config = {
        .create_backround_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .callback = msc_event_cb,
    };
    ESP_ERROR_CHECK(msc_host_install(&msc_config));

    assert(xTaskCreate(msc_device_loop, "msc_loop", 4096, NULL, 4, NULL));

    cusb_task = {usb_tasks::MSC, true};
}

void hub_off()
{
    digitalWrite(RELAY_GPIO, LOW);
    Serial.println("Hub toggled to OFF");
    delay(100);
    current_switch_state = false;
}

void hub_on()
{
    digitalWrite(RELAY_GPIO, HIGH);
    Serial.println("Hub toggled to ON");
    delay(100);
    current_switch_state = true;
}

bool wait_for_app_quit(long int timeout)
{
    unsigned long current_time = millis();
    Serial.println("Waiting " + String(timeout) + " millis for app quit...");
    while (digitalRead(APP_QUIT_PIN) != 0 && millis() - current_time < timeout)
    {
        delay(100);
        Serial.println("Remain : " + String(timeout - (millis() - current_time)));
    }
    return digitalRead(APP_QUIT_PIN) == 0;
}
