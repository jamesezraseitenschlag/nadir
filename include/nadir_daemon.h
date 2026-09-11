// James Ezra Seitenschlag
// 11.09.2026
//
// nadir runtime api daemon header

#ifndef NADIR_DAEMON_H
#define NADIR_DAEMON_H

#include "common.h"
#include "eval.h"
#include "sobject.h"

#ifdef __cplusplus
extern "C" {
#endif

// Starts the Nadir API daemon HTTP server on the given port (e.g. 3333).
// If port <= 0, defaults to 3333.
// If project_dir is non-NULL, initializes project metadata and classes from that directory.
int nadir_daemon_start(int port, const char* project_dir, Interpreter* interp);

// Converts a Nadir Value into a dynamically allocated JSON string (caller frees).
char* val_to_json_string(Value v);

#ifdef __cplusplus
}
#endif

#endif // NADIR_DAEMON_H
