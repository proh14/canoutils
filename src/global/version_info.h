#ifndef VERSION_INFO_H
#define VERSION_INFO_H

#if !defined(NAME) || !defined(VERSION) || !defined(AUTHOR)
#error "missing NAME, VERSION or AUTHOR definition"
#endif

#define print_version()                                                        \
  do {                                                                         \
    printf("%s\nversion: %s\nby: %s\n", NAME, VERSION, AUTHOR);                \
  } while (0)

#endif
