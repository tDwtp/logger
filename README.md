# logger

sample usage:
```cpp
Trace name("NAME");
name << Log::debug << "Hello" << std::endl;
trace::global << Log::info << "world" << std::endl;
```

see `test_logger.cpp` for a compilable example.

## Trace
The trace class can be used to create logs, which can be turned on or of via
`log::activate` and `log:shutdown`. Traces give a grouping mechanism and can be
associate different parts of the program with logs. This way you can deactivate
logging for network while still logging parsing or whatever. The string is used
to identify the trace.

## Log
These define the available loglevels (in order). A global `log_level` is used to
determine if a message will be logged or ignored. They should give a fine enough
granularity.

