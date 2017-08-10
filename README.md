
# locktest

The smallest possible test case to confirm that fcntl locks are currently
broke in Windows Subsystem for Linux.

## Example good output


```
============== fcntl ==============
--> 0
0 -->
--> 4
4 -->
--> 1
1 -->
--> 2
2 -->
--> 3
3 -->


============== flock_ex ==============
--> 0
0 -->
--> 1
1 -->
--> 2
2 -->
--> 3
3 -->
--> 4
4 -->
```

Only one process can hold the lock at a given time, even though
there's no guarantee of ordering since they're all starting within the
same short timeframe.


## Example bad output

```
============== fcntl ==============
--> 0
--> 1
--> 2
--> 3
--> 4
0 -->
1 -->
2 -->
3 -->
4 -->


============== flock_ex ==============
--> 0
0 -->
--> 1
1 -->
--> 2
2 -->
--> 3
3 -->
--> 4
4 -->
```

In the fcntl case, the lock is acquired by all 4 processes concurrently, then released.

Bad WSL, no cookie!
