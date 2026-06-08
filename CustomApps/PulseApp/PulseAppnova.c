#include <nuttx/config.h>

#include <sys/ioctl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <nuttx/ioexpander/gpio.h>

static void show_usage(const char *progname)
{
  fprintf(stderr, "USAGE: %s [-t <time>] <driver-path>\n", progname);
  fprintf(stderr, "Where:\n");
  fprintf(stderr, "\t<driver-path>: The full path to the GPIO pin driver.\n");
  fprintf(stderr, "\t-t <time>:     Time window in seconds (default: 1)\n");
}

int pulse_app_main(int argc, char *argv[])
{
  char *devpath    = NULL;
  uint32_t time_window = 1;
  int ndx;
  int fd;
  bool val;
  bool last_val = false;
  uint32_t pulse_count = 0;
  uint32_t pulses_window;
  uint32_t rpm;
  const uint32_t ppr = 2;
  time_t t_start;

  if (argc < 2)
    {
      fprintf(stderr, "ERROR: Missing required arguments\n");
      show_usage(argv[0]);
      return EXIT_FAILURE;
    }

  ndx = 1;

  if (ndx < argc && strcmp(argv[ndx], "-t") == 0)
    {
      if (++ndx >= argc)
        {
          fprintf(stderr, "ERROR: Missing argument to -t\n");
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }

      time_window = (uint32_t)atoi(argv[ndx]);

      if (++ndx >= argc)
        {
          fprintf(stderr, "ERROR: Missing required <driver-path>\n");
          show_usage(argv[0]);
          return EXIT_FAILURE;
        }
    }

  devpath = argv[ndx];
  printf("Driver: %s\n", devpath);

  fd = open(devpath, O_RDONLY);
  if (fd < 0)
    {
      fprintf(stderr, "ERROR: Failed to open %s\n", devpath);
      return EXIT_FAILURE;
    }

  printf("Pulse Counter: Listening on %s...\n", devpath);

  ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&val));
  last_val = val;
  t_start  = time(NULL);

  while (1)
    {
      usleep(100); 

      ioctl(fd, GPIOC_READ, (unsigned long)((uintptr_t)&val));

      if (val && !last_val)
        {
          pulse_count++;
        }

      last_val = val;

      if ((time(NULL) - t_start) >= time_window)
        {
          pulses_window = pulse_count;
          rpm           = (pulses_window * 60) / (time_window * ppr);

          printf("RPM= %lu\n",         (unsigned long)rpm);
          printf("Frequency= %luHz\n", (unsigned long)pulses_window);

          pulse_count = 0;
          t_start     = time(NULL);
        }
    }

  close(fd);
  return EXIT_SUCCESS;
}