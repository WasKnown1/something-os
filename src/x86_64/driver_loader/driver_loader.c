#include <driver_loader.h>
#include <mono_fs.h>
#include <alloc.h>

DriverLoadResult load_driver(const char *driver_path) {
    return (DriverLoadResult){ .status = 0, .value = NULL };
}

u0 execute_driver(void* driver_entry_point) {

}

i8 get_driver_status(u0) {
    return 0;
}