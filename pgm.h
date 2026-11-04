/* pgm.h - utilidades para el Proyecto I (Arquitecturas Paralelas y Compiladores)
 *
 * Solo trae lo que NO es parte del proyecto: leer y escribir imagenes PGM,
 * medir tiempo, sacar la mediana y calcular el checksum. La convolucion y
 * todas sus versiones las escribe usted.
 *
 * Funciona con gcc (Linux, WSL) y con clang (macOS). Inclúyalo antes que
 * cualquier otro #include:   #include "pgm.h"
 */
#ifndef PGM_H
#define PGM_H

#if !defined(__APPLE__) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L   /* clock_gettime y pthreads con -std=c11 en Linux */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

/* Lee un PGM binario (P5) de 8 bits. Devuelve el buffer (w*h bytes). */
static inline unsigned char *leer_pgm(const char *ruta, int *w, int *h){
    FILE *f = fopen(ruta, "rb");
    if (!f){ perror(ruta); exit(1); }
    char magic[3] = {0};
    int maxv, c;
    if (fscanf(f, "%2s", magic) != 1 || strcmp(magic, "P5") != 0){
        fprintf(stderr, "%s: solo se acepta PGM binario (P5)\n", ruta); exit(1);
    }
    while ((c = fgetc(f)) == '#' || c == ' ' || c == '\n' || c == '\r' || c == '\t'){
        if (c == '#') while ((c = fgetc(f)) != '\n' && c != EOF);
    }
    ungetc(c, f);
    if (fscanf(f, "%d %d %d", w, h, &maxv) != 3 || maxv != 255){
        fprintf(stderr, "%s: encabezado PGM invalido\n", ruta); exit(1);
    }
    fgetc(f);
    size_t n = (size_t)(*w) * (size_t)(*h);
    unsigned char *img = (unsigned char *)malloc(n);
    if (!img){ fprintf(stderr, "sin memoria\n"); exit(1); }
    if (fread(img, 1, n, f) != n){ fprintf(stderr, "%s: imagen truncada\n", ruta); exit(1); }
    fclose(f);
    return img;
}

/* Escribe un PGM binario (P5). */
static inline void escribir_pgm(const char *ruta, const unsigned char *img, int w, int h){
    FILE *f = fopen(ruta, "wb");
    if (!f){ perror(ruta); exit(1); }
    fprintf(f, "P5\n%d %d\n255\n", w, h);
    fwrite(img, 1, (size_t)w * (size_t)h, f);
    fclose(f);
}

/* Tiempo de pared en segundos. */
static inline double ahora(void){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}

/* Mediana de n tiempos (reordena el arreglo). */
static int cmp_double_(const void *a, const void *b){
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}
static inline double mediana(double *t, int n){
    qsort(t, (size_t)n, sizeof(double), cmp_double_);
    return t[n / 2];
}

/* Checksum de una imagen: s = s*31 + pixel, en aritmetica de 64 bits.
 * Imprimalo con:  printf("%" PRIu64 "\n", checksum(out, (size_t)w*h));  */
static inline uint64_t checksum(const unsigned char *img, size_t n){
    uint64_t s = 0;
    for (size_t i = 0; i < n; i++) s = s * 31u + img[i];
    return s;
}

#endif
