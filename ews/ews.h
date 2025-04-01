// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdint.h>

struct _EWS_SERVER;
typedef struct _EWS_SERVER EWS_SERVER;

EWS_SERVER* ews_start(uint16_t port, const char* root);
void ews_stop(EWS_SERVER* ws);
