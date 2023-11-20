#pragma once

#include <windows.h>
#include <stdint.h>

typedef struct {
	LARGE_INTEGER start, end;
} zen_timer_t;

static LARGE_INTEGER zen_ticks_per_second = { .QuadPart = 1 };
static int64_t zen_ticks_per_microsecond = 1;

// You MUST call ZenTimer_Init() to use ZenTimer, otherwise the tick rate will be set at 1 and you'll get garbage.
static inline void ZenTimer_Init() {
	QueryPerformanceFrequency(&zen_ticks_per_second);
	zen_ticks_per_microsecond = zen_ticks_per_second.QuadPart / 1000000;
}

static inline zen_timer_t ZenTimer_Start() {
	zen_timer_t timer;
	QueryPerformanceCounter(&timer.start);
	return timer;
}

// Returns time in microseconds
static inline int64_t ZenTimer_End(zen_timer_t* timer) {
	QueryPerformanceCounter(&timer->end);
	return (timer->end.QuadPart - timer->start.QuadPart) / zen_ticks_per_microsecond;
}