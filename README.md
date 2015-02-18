# Writer
Nanosecond-granularity disk write benchmark

This is an impossibly simple disk benchmark.  All it does is write a configurable amount of
data to disk, sleep for a configurable amount of time, and then do it again, for a number
of rounds you define.  It uses `sendfile(2)` when available, but falls back to `read(2)` and
`write(2)` when it's not, so it spends as much time in the kernel as it possibly can, instead
of spending time manipulating or sending/receiving data in userspace.  Output is in the form
of seconds-per-round with nanosecond granularity.  The degree to which the lowest-order digits
of that number are meaningful to your situation is left to the imagination of the user, but at
least this way the data's there for you to work with.

If you do interesting things with this, please let me know!  I wrote it to stress-test a VM
disk subsystem in KVM.
