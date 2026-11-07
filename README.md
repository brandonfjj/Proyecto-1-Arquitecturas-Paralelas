# Proyecto I: material

Arquitecturas Paralelas y Compiladores, Prof. Ignacio Gómez

`conv.c` aplica tres filtros (blur, gauss, sobel) a imágenes PGM en cuatro versiones:

- **V0**: versión simple, sin optimizaciones.
- **V1**: vectorizada por el compilador. Se reordenan los bucles para que el de columnas sea el más interno y el interior de la imagen se procese sin clamp; las orillas se calculan aparte.
- **V2**: V0 con las filas repartidas entre hilos (`pthread_create` / `pthread_join`).
- **V3**: V1 con las filas repartidas entre hilos; cada hilo tiene su propio acumulador.

## Compilar

```
# V0
gcc -std=c11 -O2 -fno-tree-vectorize -ffp-contract=off -pthread conv.c -o conv_v0
# V1, V2 y V3
gcc -std=c11 -O3 -march=native -ffp-contract=off -pthread conv.c -o conv_v123
```

Para ver qué bucles vectoriza el compilador, agregar `-fopt-info-vec` a la segunda línea.

## Correr

```
./conv_v0   v0 <blur|gauss|sobel> <entrada.pgm>
./conv_v123 <v1|v2|v3> <blur|gauss|sobel> <entrada.pgm> [hilos] [salida.pgm]
```

Cada corrida aplica el filtro 5 veces e imprime la mediana del tiempo y el checksum de la imagen de salida.

## Equipo

- Procesador: 13th Gen Intel(R) Core(TM) i7-13650HX
- Núcleos: 20
- Sistema: WSL (Linux)

## Validación

Todas las versiones, con 1, 2, 4 y 20 hilos, dan los checksums de referencia del enunciado para `img2048.pgm` e `img4096.pgm`, y las tres matrices esperadas con `prueba.pgm`.

## Tiempos (mediana de 5 corridas, en milisegundos)


### img2048.pgm

| Filtro | V0 | V1 | V2 1h | V2 2h | V2 4h | V2 20h | V3 1h | V3 2h | V3 4h | V3 20h |
|---|---|---|---|---|---|---|---|---|---|---|
| blur | 33.05 | 5.55 | 38.69 | 19.86 | 10.68 | 6.32 | 5.53 | 2.96 | 1.69 | 2.11 |
| gauss | 78.63 | 9.73 | 89.66 | 46.41 | 24.65 | 11.81 | 10.11 | 5.16 | 2.87 | 2.71 |
| sobel | 48.41 | 6.09 | 39.74 | 20.85 | 10.70 | 6.30 | 6.09 | 3.19 | 1.84 | 1.65 |

### img4096.pgm

| Filtro | V0 | V1 | V2 1h | V2 2h | V2 4h | V2 20h | V3 1h | V3 2h | V3 4h | V3 20h |
|---|---|---|---|---|---|---|---|---|---|---|
| blur | 130.58 | 22.39 | 157.24 | 79.24 | 40.99 | 22.54 | 21.40 | 11.11 | 6.13 | 4.42 |
| gauss | 319.69 | 38.85 | 372.44 | 198.23 | 102.91 | 55.17 | 38.30 | 20.47 | 10.63 | 6.26 |
| sobel | 198.20 | 23.52 | 157.84 | 83.26 | 43.85 | 21.07 | 24.03 | 12.81 | 6.74 | 4.10 |

## Speedup respecto a V0 (tiempo de V0 / tiempo de la versión)


### img2048.pgm

| Filtro | V1 | V2 1h | V2 2h | V2 4h | V2 20h | V3 1h | V3 2h | V3 4h | V3 20h |
|---|---|---|---|---|---|---|---|---|---|
| blur | 5.96 | 0.85 | 1.66 | 3.10 | 5.23 | 5.97 | 11.16 | 19.54 | 15.68 |
| gauss | 8.08 | 0.88 | 1.69 | 3.19 | 6.66 | 7.78 | 15.25 | 27.39 | 29.01 |
| sobel | 7.95 | 1.22 | 2.32 | 4.53 | 7.68 | 7.95 | 15.17 | 26.24 | 29.27 |

### img4096.pgm

| Filtro | V1 | V2 1h | V2 2h | V2 4h | V2 20h | V3 1h | V3 2h | V3 4h | V3 20h |
|---|---|---|---|---|---|---|---|---|---|
| blur | 5.83 | 0.83 | 1.65 | 3.19 | 5.79 | 6.10 | 11.75 | 21.32 | 29.52 |
| gauss | 8.23 | 0.86 | 1.61 | 3.11 | 5.80 | 8.35 | 15.62 | 30.06 | 51.09 |
| sobel | 8.43 | 1.26 | 2.38 | 4.52 | 9.41 | 8.25 | 15.47 | 29.39 | 48.35 |
