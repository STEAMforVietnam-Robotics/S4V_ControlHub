#include "Module.h"

// Head of the global module registry. Every Module registers itself here in
// its constructor (see Module.h), so ControlHub can walk the list without the
// user calling attach().
Module *Module::_head = nullptr;
