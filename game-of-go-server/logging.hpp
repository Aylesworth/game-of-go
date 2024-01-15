//
// Created by Admin on 1/16/2024.
//

#ifndef GAME_OF_GO_SERVER_LOGGING_HPP
#define GAME_OF_GO_SERVER_LOGGING_HPP

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#define LOG_FILE "server.log"

void log(const char *format, ...) {
    // Construct the log message
    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);
    int length = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    char *log = (char *) malloc((length + 1) * sizeof(char));

    vsnprintf(log, length + 1, format, args);

    va_end(args);

    // Construct timestamp
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char timestamp[80];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Write to file
    FILE *f = fopen(LOG_FILE, "a+");
    fprintf(f, "[%s]: %s\n", timestamp, log);
    fclose(f);
}

#endif //GAME_OF_GO_SERVER_LOGGING_HPP
