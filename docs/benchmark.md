
# Benchmarks

## Benchmarks without Pipelined Requests

* Date: 2020-02-08
* Commit: 5eb9fb5344bde1318201aa2d4abad7dddf07de20

### redis-server

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,set,get -P 16 -q 
PING_INLINE: 1267427.12 requests per second
PING_BULK: 1695633.75 requests per second
SET: 906412.88 requests per second
GET: 970402.69 requests per second
```

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,get -P 16 -q 
PING_INLINE: 1281640.50 requests per second
PING_BULK: 1706484.62 requests per second
GET: 1514577.75 requests per second
```

### protodb1, 1 threads

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,set,get -P 16 -q
PING_INLINE: 1869158.75 requests per second
PING_BULK: 1845869.88 requests per second
SET: 905592.06 requests per second
GET: 844416.31 requests per second
```

```
$ redis-benchmark -r 1000000 -n 10000000 -t ping,get -P 16 -q 
PING_INLINE: 1825150.50 requests per second
PING_BULK: 1833180.62 requests per second
GET: 1789549.12 requests per second
```

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        13850.07          ---          ---      1.37800      1066.70 
Gets       138348.50         0.00    138348.50      1.35900      5389.27 
Waits           0.00          ---          ---      0.00000          --- 
Totals     152198.57         0.00    138348.50      1.36100      6455.97 
```

### protodb1, 2 threads

```
$ redis-benchmark -r 1000000 -n 4000000 -t ping,get -P 16 -q 
PING_INLINE: 1817355.75 requests per second
PING_BULK: 1806684.75 requests per second
GET: 1789709.25 requests per second
```

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8 
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        24070.41          ---          ---      0.76700      1853.84 
Gets       240439.62         0.00    240439.62      0.75400      9366.16 
Waits           0.00          ---          ---      0.00000          --- 
Totals     264510.03         0.00    240439.62      0.75500     11220.00 
```

### protodb1, 4 threads

```
$ ./memtier_benchmark --hide-histogram --threads 4 –data-size 8
[...]
=========================================================================
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
-------------------------------------------------------------------------
Sets        40841.95          ---          ---      0.45600      3145.55 
Gets       407970.64         0.00    407970.64      0.44200     15892.22 
Waits           0.00          ---          ---      0.00000          --- 
Totals     448812.59         0.00    407970.64      0.44300     19037.76 
```

## Benchmarks with Pipelined Requests

* Date: 2020-02-13
* Commit: c460ace415494c564f797cc3fab816a6c1f43f21
* Test Command:
```
 $ ./memtier_benchmark --hide-histogram --threads 8 –data-size 8 --pipeline=16 --test-time=10
```

### protodb1, 1 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    1694531.62         0.00   1540454.97      3.77200     71882.07
Totals    1769907.82         0.00   1608992.72      4.51500     75076.03
Totals    1702930.12         0.00   1548104.93      5.63200     72233.89
```

### protodb1, 2 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    2768627.02         0.00   2516917.70      2.30600    117458.38
Totals    2893443.77         0.00   2630378.04      2.75900    122746.38
Totals    3012342.09         0.00   2738473.72      3.18100    127786.68
```

### protodb1, 4 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    3838537.86         0.00   3489562.06      1.66200    162857.94
Totals    3148135.49         0.00   2861918.39      2.53600    133556.38
Totals    2989134.51         0.00   2717368.47      3.20600    126802.38
```

### protodb1, 8 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    2060480.12         0.00   1873142.84      3.09900     87409.09
Totals    2195448.74         0.00   1995837.85      3.63700     93132.19
Totals    2076684.53         0.00   1887867.97      4.61600     88090.99
```

## protodb1 (folly) Benchmarks with Pipelined Requests

* Date: 2020-02-13
* Commit: N/A
* Test Command:
```
$ ./memtier_benchmark --hide-histogram --threads 8 –data-size 8 --pipeline=16 --test-time=10
$ ./memtier_benchmark --hide-histogram --threads 10 –data-size 8 --pipeline=16 --test-time=10
$ ./memtier_benchmark --hide-histogram --threads 12 –data-size 8 --pipeline=16 --test-time=10
```
### protodb1 (folly), 1 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    2385274.27         0.00   2168214.31      3.68800    101178.72
Totals    1669209.85         0.00   1517450.28      4.78700     70804.78
Totals    1607294.64         0.00   1461146.57      5.96700     68175.41
```

### protodb1 (folly), 2 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    4473137.13         0.00   4066081.65      2.12800    189741.83
Totals    2974144.65         0.00   2703751.53      2.68400    126170.60
Totals    3047838.31         0.00   2770729.70      3.14400    129293.93
```

### protodb1 (folly), 4 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    5405032.90         0.00   4913174.90      1.18200    229271.04
Totals    5099177.93         0.00   4635592.49      1.56400    216414.25
Totals    4964658.43         0.00   4513300.18      1.92800    210639.59
```

### protodb1 (folly), 8 threads, 16 pipeline

```
Type         Ops/sec     Hits/sec   Misses/sec      Latency       KB/sec 
Totals    6890682.76         0.00   6263630.63      0.99000    292289.44
Totals    7333200.74         0.00   6666523.57      1.08600    311186.89
Totals    7756781.32         0.00   7051591.22      1.23200    329149.37
```
