// This file is part of the tetris-table project. Copyright (c) Johan Duparc.

#pragma once

#include <Arduino.h>

enum {
	LogLevel_None,
	LogLevel_Fatal,
	LogLevel_Error,
	LogLevel_Warn,
	LogLevel_Info,
	LogLevel_Verbose,
};
constexpr const int logLevel = LogLevel_Info;

template <typename T> void logV  (const T& t) { if constexpr (logLevel>4) Serial.print(t);   }
template <typename T> void logVln(const T& t) { if constexpr (logLevel>4) Serial.println(t); }
template <typename T> void logI  (const T& t) { if constexpr (logLevel>3) Serial.print(t);   }
template <typename T> void logIln(const T& t) { if constexpr (logLevel>3) Serial.println(t); }
template <typename T> void logW  (const T& t) { if constexpr (logLevel>2) Serial.print(t);   }
template <typename T> void logWln(const T& t) { if constexpr (logLevel>2) Serial.println(t); }
template <typename T> void logE  (const T& t) { if constexpr (logLevel>1) Serial.print(t);   }
template <typename T> void logEln(const T& t) { if constexpr (logLevel>1) Serial.println(t); }
template <typename T> void logF  (const T& t) { if constexpr (logLevel>0) Serial.print(t);   }
template <typename T> void logFln(const T& t) { if constexpr (logLevel>0) Serial.println(t); }

