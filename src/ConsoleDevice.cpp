#include "ConsoleDevice.hpp"
#include <iostream>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <csignal>
    #include <sys/select.h>
#endif

#ifndef _WIN32
namespace {
    termios origTermios{};
    bool     termiosSaved = false;

    void restoreTerminal() {
        if (termiosSaved) {
            tcsetattr(STDIN_FILENO, TCSANOW, &origTermios);
            termiosSaved = false;
        }
    }

    void signalHandler(int sig) {
        restoreTerminal();
        std::_Exit(128 + sig);
    }
}
#endif

ConsoleDevice::ConsoleDevice(Bus&) {
#ifndef _WIN32
    enableRawMode();
#endif
}

ConsoleDevice::~ConsoleDevice() {
#ifndef _WIN32
    disableRawMode();
#endif
}

#ifndef _WIN32
void ConsoleDevice::enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &origTermios) != 0) return;

    termios raw = origTermios;
    raw.c_lflag &= static_cast<unsigned int>(~(ECHO | ICANON | ISIG));
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
        rawModeEnabled = true;
        termiosSaved   = true;
        std::signal(SIGINT,  signalHandler);
        std::signal(SIGTERM, signalHandler);
    }
}

void ConsoleDevice::disableRawMode() {
    if (rawModeEnabled) {
        restoreTerminal();
        rawModeEnabled = false;
    }
}
#endif

void ConsoleDevice::poll() {
#ifdef _WIN32
    while (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 0xE0) {
            // Extended key (arrows, F-keys, etc.) — consume and discard
            // the follow-up scan code so it doesn't get fed to the kernel
            // as garbage.
            if (_kbhit()) _getch();
            continue;
        }
        inputQueue.push(static_cast<uint8_t>(c));
    }
#else
    fd_set fds;
    timeval tv{0, 0};
    for (;;) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int r = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
        if (r <= 0) break;

        uint8_t c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0) break;
        inputQueue.push(c);
    }
#endif
}

uint8_t ConsoleDevice::readData() {
    poll();
    if (inputQueue.empty()) return 0;
    uint8_t c = inputQueue.front();
    inputQueue.pop();
    return c;
}

uint8_t ConsoleDevice::readStatus() {
    poll();
    uint8_t status = 0x01;
    if (!inputQueue.empty()) status |= 0x02;
    return status;
}

void ConsoleDevice::writeData(uint8_t v) {
    std::cout << static_cast<char>(v);
    std::cout.flush();
}

void ConsoleDevice::writeControl(uint8_t) {
    // reserved for future use
}