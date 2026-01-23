#ifndef DRIVER_LOADER_H
#define DRIVER_LOADER_H

#include <pe_loader.h>

DriverLoadResult *load_driver(const i8 *driver_path);
u0 execute_driver(u0* driver_entry_point);
i8 get_driver_status(u0);

#endif // DRIVER_LOADER_H