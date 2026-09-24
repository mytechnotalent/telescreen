# Volume 25: RP5 as a Camera

**the UVC pipeline and RTSP**

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

## What You'll Learn

- Capture from a cheap **USB webcam** (UVC) on the RP5
- Serve it over **RTSP** and **MJPEG** so a phone needs **no app** for one path
- The real `camera.c` URL helpers
- Why the device faithfully uses RTSP with no authentication (and why you fix it)

## 1. Hardware

A cheap **Xweiryn USB webcam** (`PARTS.md`) plugs into the RP5 and enumerates as a
standard **UVC** device at `/dev/video0`. No driver work is needed - the kernel
`uvcvideo` driver handles it.

```bash
ls -l /dev/video0
v4l2-ctl --device /dev/video0 --list-formats-ext
```

## 2. Two Ways to View It

The lab serves **both** URLs so a phone needs **no app** for one of them:

| URL | viewer | app |
| --- | ------ | --- |
| `rtsp://<ip>:554/stream` | VLC | one free app |
| `http://<ip>:80/video.mjpg` | any browser | **none** |

## 3. The Real Source - `include/camera.h`

```c
#define CAMERA_DEV_PATH "/dev/video0"
#define CAMERA_RTSP_PATH "/stream"
#define CAMERA_MJPEG_PATH "/video.mjpg"
#define CAMERA_RTSP_PORT 554u
#define CAMERA_HTTP_PORT 80u

bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len);
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len);
```

## 4. The Real Source - `src/camera.c`

```c
static bool camera_url(char *out, size_t out_len, const char *ip, uint16_t port,
                       const char *scheme, const char *path) {
    int n;
    if ((out == NULL) || (ip == NULL) || (out_len == 0u)) return false;
    n = snprintf(out, out_len, "%s://%s:%u%s", scheme, ip, (unsigned)port, path);
    return (n > 0) && ((size_t)n < out_len);
}
```

```bash
$ python3 scripts/run_tests.py | grep camera
:test_camera_rtsp_url:PASS
:test_camera_mjpeg_url:PASS
:test_camera_url_bad:PASS
```

The tests pin the exact URLs, including the port formatting.

## 5. Capture and Serve

Software-only path (no code):

```bash
# RTSP with ffmpeg
ffmpeg -f v4l2 -i /dev/video0 -c:v libx264 -f rtsp rtsp://127.0.0.1:554/stream

# or a simple MJPEG HTTP server
mjpg_streamer -i "input_uvc.so -d /dev/video0" -o "output_http.so -p 80"
```

## 6. The Phone (no custom app)

1. **Router lab:** join the SSID `TELESCREEN-XXXX`; the phone gets a DHCP lease. No app.
2. **Video lab:** open `rtsp://<ip>/stream` in **VLC**, or `http://<ip>/video.mjpg` in the
   browser (no app).
3. **Web/backdoor labs:** open `http://<ip>/` in the browser.

## 7. The Faithful Flaw

The real TELESCREEN streams over **RTSP on 554 with no authentication** (`rtspauth="n"`).
The lab keeps that shape - that is the lesson. The fix is to require a credential before
the stream is delivered, and to check it **before** parsing the URL (the parser-bug
lesson from `docs/14`).

## Labs

1. Plug in the webcam and confirm `/dev/video0`.
2. Serve MJPEG and open it on the phone browser (no app).
3. Serve RTSP and open it in VLC.
4. Show the stream plays with **no authentication**, then add a credential check.

## Reference

- `src/camera.c`, `include/camera.h`
- `docs/24` (router), `docs/14` (dispatcher), `PARTS.md`
