#ifndef FLOPPYSELECTOR_H_
#define FLOPPYSELECTOR_H_

#include <sys/types.h>
#include <stdio.h>

#include "commands.h"
#include "screen.h"
#include "helper.h"
#include "storage.h"

__uint16_t floppy_selector_ro();
__uint16_t floppy_selector_rw();

#endif /* FLOPPYSELECTOR_H_ */