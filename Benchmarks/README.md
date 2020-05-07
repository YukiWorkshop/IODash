Benchmarks
=========

### Test Platform
- CPU: Intel(R) Core(TM) i7-9750H CPU @ 2.60GHz **(Turbo off)**
- RAM: Micron DDR4 32GB 3200MHz x 2
- Kernel: 5.4.15, `CONFIG_HZ_250`, `PREEMPT`
- Test command: `wrk -t 2 -c 10000 -d 10s`

### Results
- [IODash](./IODash_HTTP.cpp)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:8082
Running 10s test @ http://127.0.0.1:8082
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    45.70ms   44.97ms   1.06s    97.99%
    Req/Sec    22.09k   646.62    23.52k    71.97%
  425827 requests in 10.11s, 65.79MB read
Requests/sec:  42118.21
Transfer/sec:      6.51MB
```

- [Boost](./boost_HTTP.cpp)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:8083 
Running 10s test @ http://127.0.0.1:8083
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    33.99ms  143.76ms   1.98s    95.26%
    Req/Sec    18.32k     1.37k   30.39k    91.49%
  351446 requests in 10.08s, 54.30MB read
  Socket errors: connect 0, read 0, write 0, timeout 1065
Requests/sec:  34882.48
Transfer/sec:      5.39MB
```

- [libuv](./libuv_HTTP.c)
```
$ ./wrk -t 2 -c 10000 -d 10s http://127.0.0.1:10000
Running 10s test @ http://127.0.0.1:10000
  2 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   749.85ms  598.27ms   1.98s    53.53%
    Req/Sec     4.73k     3.27k   11.73k    77.42%
  60663 requests in 10.88s, 0.00B read
  Socket errors: connect 0, read 60663, write 0, timeout 0
Requests/sec:   5576.72
Transfer/sec:       0.00B
```

## Help Wanted

The libuv test is based on an example randomly found on the Internet and is nearly broken. If you are good at libuv stuff, you could improve it.