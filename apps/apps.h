#ifndef APPS_H
#define APPS_H

/* Each app creates its own task(s). main() calls these before starting the scheduler. */
void
blinky_start(void);

void
shell_start(void);

#endif // APPS_H
