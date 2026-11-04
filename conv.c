/*  conv.c - Proyecto I: convolucion con vectorizacion e hilos 
    Por ahora solo V0 (version simple). */


#include "pgm.h" 

#define N_FILTROS 3

#define REPS 5

/*  Primera parte: Definión de los filtros  */

//  Nota importante: Los filtros se ven como matrices en el enunciado, y sí lo son, pero en este caso se definieron como arrays. Esto es porque, en C una matriz 
//  en memoria se guarda como una array seguido, o sea, no hay distinción entre uno y otroo. Además, para más adelante, es mejor manejar arrays para no tener que estar 
//  indicando tamaños de matrices (entre 3x3 y 5x5) 

static const float K_BLUR[9] = {
    1/9.f, 1/9.f, 1/9.f,
    1/9.f, 1/9.f, 1/9.f,
    1/9.f, 1/9.f, 1/9.f
};

static const float K_GAUSS[25] = {
    1/273.f,  4/273.f,  7/273.f,  4/273.f, 1/273.f,
    4/273.f, 16/273.f, 26/273.f, 16/273.f, 4/273.f,
    7/273.f, 26/273.f, 41/273.f, 26/273.f, 7/273.f,
    4/273.f, 16/273.f, 26/273.f, 16/273.f, 4/273.f,
    1/273.f,  4/273.f,  7/273.f,  4/273.f, 1/273.f
};

static const float K_SOBEL[9] = {
    -1.f, 0.f, 1.f,
    -2.f, 0.f, 2.f,
    -1.f, 0.f, 1.f
};

// Para describir el filtro solo se necesita sus coeficientes y su radio, cada filtro es: coeficientes + radio r (tamano = 2r+1) 
typedef struct {
    const char  *nombre; // puntero al texto
    const float *K;      // puntero al primer coeficiente
    int          r;
} Filtro;

static const Filtro FILTROS[3] = {
    { "blur",  K_BLUR,  1 },
    { "gauss", K_GAUSS, 2 },
    { "sobel", K_SOBEL, 1 },
};


/*  
    V0: version simple
    Para cada pixel: recorre la ventana por filas (arriba->abajo, izq->der), con clamp de cada indice por separado (replica la orilla),
    suma en float, valor absoluto, recorte a 255 y redondeo con (unsigned char)(suma + 0.5f).
*/
static void v0(const unsigned char *in, unsigned char *out,
               int w, int h, const float *K, int r)
{
    int n = 2 * r + 1;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float suma = 0.0f;
            for (int i = -r; i <= r; i++) {
                int f = y + i; if (f < 0) f = 0; if (f > h - 1) f = h - 1;
                for (int j = -r; j <= r; j++) {
                    int c = x + j; if (c < 0) c = 0; if (c > w - 1) c = w - 1;
                    suma += K[(i + r) * n + (j + r)] * in[f * w + c];
                }
            }
            if (suma < 0.0f)   suma = -suma;                // Se quita signo 
            if (suma > 255.0f) suma = 255.0f;               // Se hace recorte a 255
            out[y * w + x] = (unsigned char)(suma + 0.5f);  // Se redondea con + 0.5f
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <blur|gauss|sobel> <entrada.pgm> [salida.pgm]\n", argv[0]);
        return 1;
    }

    const Filtro *flt = NULL;
    for (int k = 0; k < N_FILTROS; k++)
        if (strcmp(argv[1], FILTROS[k].nombre) == 0) flt = &FILTROS[k];
    if (!flt) {
        fprintf(stderr, "Filtro desconocido: %s\n", argv[1]);
        return 1;
    }

    int w, h;
    unsigned char *in  = leer_pgm(argv[2], &w, &h);
    unsigned char *out = (unsigned char *)malloc((size_t)w * h);
    if (!out) { fprintf(stderr, "sin memoria\n"); return 1; }

    // Medicion de 5 corridas y se reporta la mediana
    double t[REPS];
    for (int k = 0; k < REPS; k++) {
        double t0 = ahora();
        v0(in, out, w, h, flt->K, flt->r);
        t[k] = ahora() - t0;
    }
    printf("mediana %.6f s\n", mediana(t, REPS));

    printf("checksum %" PRIu64 "\n", checksum(out, (size_t)w * h));

    if (argc >= 4) escribir_pgm(argv[3], out, w, h);

    free(in);
    free(out);
    return 0;
}