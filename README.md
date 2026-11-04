# Proyecto I: material

Arquitecturas Paralelas y Compiladores, Prof. Ignacio Gómez

- `pgm.h`: leer y escribir imágenes (`leer_pgm`, `escribir_pgm`), medir tiempo (`ahora`),
  mediana de varios tiempos (`mediana`) y checksum de una imagen (`checksum`).
  Inclúyalo antes que cualquier otro `#include`.
- `prueba.pgm`: la imagen de 5x5 de los ejemplos del enunciado.
- `img2048.pgm`, `img4096.pgm`: las imágenes para medir tiempos.

## Resultado esperado con prueba.pgm

    blur                 gauss                sobel
    10 10 23 27 30       10 13 21 29 27        0  0 160 40 120
    10 11 24 28 30       10 13 21 29 27        0 10 160 30 120
    10 11 24 28 30       10 13 22 30 27        0 20 160 20 120
    10 11 24 28 30       10 13 21 29 27        0 10 160 30 120
    10 10 23 27 30       10 13 21 29 27        0  0 160 40 120

## Checksums esperados

|       | img2048              | img4096              |
|-------|----------------------|----------------------|
| blur  | 15228573027849069044 | 14133797607195559027 |
| gauss | 6704049370287127316  | 11810212636788029950 |
| sobel | 13749065433343460504 | 15895088315003003173 |

Para imprimir un checksum:

    #include "pgm.h"
    printf("checksum %" PRIu64 "\n", checksum(out, (size_t)w * h));

Compile siempre con `-ffp-contract=off`.
