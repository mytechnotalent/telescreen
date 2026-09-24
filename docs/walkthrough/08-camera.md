# Walkthrough 08: `camera`

**every line of `src/camera.c`, explained**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why This Module

`camera.c` builds the two URLs a client uses to view the UVC webcam (`docs/25`): the RTSP
URL (for VLC) and the MJPEG URL (for a browser). It is small, and it shows a **safe string
build** in C - the pattern that, done wrong, produces buffer overflows.

```
camera_url       (private)   the shared formatter
camera_rtsp_url  (public)    rtsp://<ip>:<port>/stream
camera_mjpeg_url (public)    http://<ip>:<port>/video.mjpg
```

## 1. The Constants

```c
#define CAMERA_DEV_PATH "/dev/video0"
#define CAMERA_RTSP_PATH "/stream"
#define CAMERA_MJPEG_PATH "/video.mjpg"
#define CAMERA_RTSP_PORT 554u
#define CAMERA_HTTP_PORT 80u
```

One place for every path and port. Change the port here and both URLs follow.

## 2. `camera_url` - the safe formatter

```c
static bool camera_url(char *out, size_t out_len, const char *ip, uint16_t port,
                       const char *scheme, const char *path) {
    int n;
    if ((out == NULL) || (ip == NULL) || (out_len == 0u)) return false;
    n = snprintf(out, out_len, "%s://%s:%u%s", scheme, ip, (unsigned)port, path);
    return (n > 0) && ((size_t)n < out_len);
}
```

Line by line:

| line | what it does |
| ---- | ------------ |
| `int n;` | will hold `snprintf`'s return |
| guard | reject null / zero-length output |
| `snprintf(out, out_len, ...)` | **bounded** format - never writes past `out_len` |
| `(n > 0)` | `snprintf` returns the length it *would* write, or negative on error |
| `(size_t)n < out_len` | the result actually **fit** (snprintf truncates silently otherwise) |

This is the whole lesson. `sprintf` would happily overrun `out`. `snprintf` takes the
buffer size, but it **truncates** rather than failing - so you must check the return. The
comparison `(size_t)n < out_len` is how you turn truncation into a `false`.

> `(n > 0)` also rejects `snprintf`'s negative error return, which a cast to `size_t`
> would otherwise turn into a huge positive number.

## 3. The Two Wrappers

```c
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "rtsp", CAMERA_RTSP_PATH);
}

bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "http", CAMERA_MJPEG_PATH);
}
```

Each wrapper fixes the scheme and the path; only `ip` and `port` vary. The formatting
logic lives once, in `camera_url`.

## 4. The Outputs

| function | example output |
| -------- | -------------- |
| `camera_rtsp_url("192.168.1.88", 554, ...)` | `rtsp://192.168.1.88:554/stream` |
| `camera_mjpeg_url("192.168.1.88", 80, ...)` | `http://192.168.1.88:80/video.mjpg` |

The MJPEG URL is why a phone needs **no app** for one path (`docs/25`).

## 5. AArch64

The wrappers are thin; the compiler may inline them. `snprintf` becomes a call to the C
library, so most of the assembly is argument marshalling (the format string and args in
`x0..x3` and the stack). The bounded write is inside `snprintf`.

## 6. Tests

```bash
$ python3 scripts/run_tests.py | grep camera
:test_camera_rtsp_url:PASS
:test_camera_mjpeg_url:PASS
:test_camera_url_bad:PASS
```

- `rtsp`/`mjpeg` pin the **exact** strings, including the port.
- `url_bad` passes an output buffer of 4 bytes and expects `false` (truncation caught),
  and passes `NULL` ip and expects `false`.

## 7. The Pattern to Reuse

```c
n = snprintf(out, out_len, fmt, ...);
if ((n < 0) || ((size_t)n >= out_len)) {
    /* truncated or error - do not trust out */
}
```

Every string build in the lab follows this. It is the difference between a lab and a
CVE.

## Exercises

1. Call `camera_rtsp_url` with an `out_len` of 10 and confirm it returns `false`.
2. Add a `camera_https_url` with scheme `https` and port 443.
3. Explain why checking `snprintf`'s return is mandatory.
4. State what `sprintf` would have done with the same input.

## Reference

- `src/camera.c`, `include/camera.h`
- `docs/25` (RP5 as a camera)
