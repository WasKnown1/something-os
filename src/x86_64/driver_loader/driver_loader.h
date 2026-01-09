#ifndef DRIVER_LOADER_H
#define DRIVER_LOADER_H

#include <smthng_os.h>

typedef Result(void*, i8) DriverLoadResult;

DriverLoadResult load_driver(const char *driver_path);
u0 execute_driver(void* driver_entry_point);
i8 get_driver_status(u0);

#endif // DRIVER_LOADER_H