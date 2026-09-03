#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <linux/input.h>

#define PCSPKR_PATH "/dev/input/by-path/platform-pcspkr-event-spkr"

static int g_fd = -1;

static void send_tone(int fd, int freq) {
    struct input_event e;
    memset(&e, 0, sizeof(e));
    e.type = EV_SND;
    e.code = SND_TONE;
    e.value = freq;
    if (write(fd, &e, sizeof(e)) != (ssize_t)sizeof(e)) {
        perror("write(pcspkr)");
    }
}

static void sleep_ms(long ms) {
    if (ms <= 0) return;
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void on_sigint(int sig) {
    (void)sig;
    if (g_fd >= 0) send_tone(g_fd, 0);
    _exit(130);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <notes.txt> [device_path]\n", argv[0]);
        fprintf(stderr, "  Each line in notes.txt: '<freq_hz> <duration_ms>' (freq 0 = rest)\n");
        fprintf(stderr, "  Default device: %s\n", PCSPKR_PATH);
        return 1;
    }

    const char *notes_path = argv[1];
    const char *device_path = (argc >= 3) ? argv[2] : PCSPKR_PATH;

    int fd = open(device_path, O_WRONLY);
    if (fd < 0) {
        perror("open pcspkr device");
        fprintf(stderr,
            "\nCould not open %s\n"
            "Things to check:\n"
            "  1. Load the driver:      sudo modprobe pcspkr\n"
            "  2. Run with sudo, or set up a udev rule for write access\n"
            "  3. Confirm the node exists: ls -l %s\n",
            device_path, device_path);
        return 1;
    }
    g_fd = fd;
    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    FILE *f = fopen(notes_path, "r");
    if (!f) {
        perror("open notes file");
        close(fd);
        return 1;
    }

    int freq, dur;
    while (fscanf(f, "%d %d", &freq, &dur) == 2) {
        if (freq > 0) {
            send_tone(fd, freq);
            sleep_ms((dur * 9) / 10);
            send_tone(fd, 0);
            sleep_ms(dur - (dur * 9) / 10);
        } else {
            send_tone(fd, 0);
            sleep_ms(dur);
        }
    }

    send_tone(fd, 0);
    fclose(f);
    close(fd);
    return 0;
}
