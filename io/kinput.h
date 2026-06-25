#pragma once

#include <stddef.h>

#define MAX_COMMAND_LEN 128

int input_poll(char *buffer, size_t buffer_size);
void input_prompt(void);
