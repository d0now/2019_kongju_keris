2019 kongju keris stealien project.

All task compiled with Ubuntu 18.04 x86_64 w/ gcc (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0

--------------------------- x86_64 glibc version ---------------------------
GNU C Library (Ubuntu GLIBC 2.27-3ubuntu1) stable release version 2.27.
Compiled by GNU CC version 7.3.0.
----------------------------------------------------------------------------

---------------------------- i386 glibc version ----------------------------
GNU C Library (Ubuntu GLIBC 2.27-3ubuntu1) stable release version 2.27.
Compiled by GNU CC version 7.3.0.
----------------------------------------------------------------------------

Tasks
+-- oneshot
+-- babystack
+-- humanstack
+-- alienstack

Binary options per task
+---------------+-------+-----------------+---------+-------+-----+-----+
| Name          | Strip | Static compiled | A r c h | RELRO | N X | PIE |
+---------------+-------+-----------------+---------+-------+-----+-----+
| oneshot       |   O   |        X        |   x64   |   P   |  O  |  X  |
| babystack     |   X   |        X        |   i386  |   P   |  O  |  X  |
| humanstack    |   X   |        X        |   i386  |   P   |  O  |  O  |
| alienstack    |   O   |        X        |   i386  |   P   |  O  |  O  |
+---------------+-------+-----------------+---------+-------+-----+-----+