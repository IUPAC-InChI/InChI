/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2024 IUPAC and InChI Trust
 */


#ifndef LOGGING_H
#define LOGGING_H

#define LOGGING_ENABLED 0            /*(@nnuk : Nauman Ullah Khan) :: Change to zero (0) if print outputs not needed otherwise keep it one (1)*/

#if LOGGING_ENABLED
#define LOG_NO_ARGS(message) printf("%s\n", message)
#define LOG_MULT_ARGS(format, ...) printf(format, __VA_ARGS__)
#else
#define LOG_NO_ARGS(message)
#define LOG_MULT_ARGS(format, ...)
#endif

#endif /* LOGGING_H */