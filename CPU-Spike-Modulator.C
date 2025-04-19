#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BIT_DURATION_MS 500  // How long to represent each bit
#define CPU_BURN_INTENSITY 10000  // Tuning param for how hard to burn

// Pin process to single core (optional but useful for consistency)
void set_cpu_affinity() {
    HANDLE hProcess = GetCurrentProcess();
    DWORD_PTR affinityMask = 1 << 0; // Pin to CPU 0
    SetProcessAffinityMask(hProcess, affinityMask);
}

// High-resolution sleep using Waitable Timer
void precise_sleep_ms(int milliseconds) {
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (!timer) return;

    LARGE_INTEGER li;
    li.QuadPart = -((LONGLONG)milliseconds * 10000); // Convert to 100ns units (negative = relative time)
    SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}

// Busy-wait to simulate "1"
void burn_cpu_for(int milliseconds) {
    LARGE_INTEGER freq, start, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    do {
        // Burn some CPU cycles
        for (int i = 0; i < CPU_BURN_INTENSITY; i++) {
            double x = 1.0;
            x *= 1.000001;
        }
        QueryPerformanceCounter(&now);
    } while ((now.QuadPart - start.QuadPart) * 1000 / freq.QuadPart < milliseconds);
}

// Transmit one bit
void transmit_bit(int bit) {
    if (bit)
        burn_cpu_for(BIT_DURATION_MS);
    else
        precise_sleep_ms(BIT_DURATION_MS);
}

// Transmit a byte as 8 bits MSB-first
void transmit_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        int bit = (byte >> i) & 1;
        transmit_bit(bit);
    }
}

// Transmit entire string
void transmit_message(const char *msg) {
    for (int i = 0; msg[i] != '\0'; i++) {
        transmit_byte(msg[i]);
        precise_sleep_ms(BIT_DURATION_MS); // small gap between characters
    }
}

int main() {
    const char *message = "HELLO";
    printf("Transmitting message: %s\n", message);

    set_cpu_affinity(); // Optional but recommended
    transmit_message(message);

    printf("Done.\n");
    return 0;
}
