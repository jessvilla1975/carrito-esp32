#ifndef LIBCAMERA_H
#define LIBCAMERA_H

/*
 * Cámara OV2640 — ESP32-S3-CAM-N16R8 (Keyestudio MB0184)
 *
 * Pinout:
 *   SIOD → 4   SIOC → 5   VSYNC → 6   HREF → 7
 *   XCLK → 15  PCLK → 13
 *   D7→16  D6→17  D5→18  D4→12  D3→10  D2→8  D1→9  D0→11
 *   PWDN = -1  RESET = -1
 *
 * Stream MJPEG disponible en: http://<IP>:81/stream
 */

bool initCamera();
void startCameraStream();

#endif /* LIBCAMERA_H */
