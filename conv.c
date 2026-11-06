/*  conv.c - Proyecto I: convolucion con vectorizacion e hilos 
    Versiones V0 (version simple) y V1 (vectoriazada). */


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

/*
    Inicio de V1
    Función pixel_escalar, sirve para orillas de V1

*/
static unsigned char pixel_escalar(const unsigned char *in, int w, int h, const float *K, int r, int y, int x) {
    int n = 2 * r + 1;
    float suma = 0.0f;
    for (int i = -r; i <= r; i++) {
        int f = y + i; if (f < 0) f = 0; if (f > h - 1) f = h - 1;
        for (int j = -r; j <= r; j++) {
            int c = x + j; if (c < 0) c = 0; if (c > w - 1) c = w - 1;
            suma += K[(i + r) * n + (j + r)] * in[f * w + c];
        }
    }

    if (suma < 0.0f)   suma = -suma;
    if (suma > 255.0f) suma = 255.0f;
    return (unsigned char)(suma + 0.5f);
}



/* V1: vectorizada por el compilador
    Idea: en vez de "para cada pixel, recorrer la ventana", se hace
    "para cada coeficiente de la ventana, recorrer toda la fila".
    Asi el bucle de columnas (x) queda como el mas interno y el
    compilador lo puede vectorizar: varios pixeles vecinos avanzan juntos.
 
    acc[x] es la "suma" de V0, pero una por cada pixel de la fila.
    Cada acc[x] recibe sus terminos en el mismo orden (i, luego j) que
    en V0, por eso el resultado es identico bit a bit.
 
    Solo el interior (r <= x < w-r) va por el camino vectorial, ahi
    x + j nunca se sale de la imagen y no hace falta clamp en columnas.
    Las orillas se calculan con pixel_escalar.
 
    Trabaja sobre las filas [y0, y1) para poder reutilizarla en V3.
 */
static void v1_filas(const unsigned char *in, unsigned char *out, int w, int h, const float *K, int r, int y0, int y1, float *restrict acc) {
    int n  = 2 * r + 1;
    int x0 = r;          // primera columna interior
    int x1 = w - r;      // una despues de la ultima
 
    for (int y = y0; y < y1; y++) {
        // 1. Sumas de la fila en cero 
        for (int x = 0; x < w; x++) {
            acc[x] = 0.0f;
        }
        // 2. Ventana afuera, columnas adentro
        for (int i = -r; i <= r; i++) {
            int f = y + i; if (f < 0) f = 0; if (f > h - 1) f = h - 1;
            const unsigned char *restrict fila = in + (size_t)f * w;
            for (int j = -r; j <= r; j++) {
                float k = K[(i + r) * n + (j + r)];
                for (int x = x0; x < x1; x++) {         // se vectoriza 
                    acc[x] += k * fila[x + j];
                }
            }
        }
 
        /* 3. Cerrar los pixeles interiores: abs, recorte, redondeo */
        unsigned char *restrict o = out + (size_t)y * w;
        for (int x = x0; x < x1; x++) {
        float s = acc[x];
            // Convertir valores negativos a positivos
            if (s < 0.0f) {
                s = -s;
            }
            // Limitar el valor máximo
            if (s > 255.0f) {
                s = 255.0f;
            }
            // Redondear y guardar
            o[x] = (unsigned char)(s + 0.5f);
        }
 
        // 4. Orillas izquierda y derecha, como en V0 
        for (int x = 0; x < x0 && x < w; x++) {
            o[x] = pixel_escalar(in, w, h, K, r, y, x);
        }
        for (int x = (x1 > x0 ? x1 : x0); x < w; x++) {
            o[x] = pixel_escalar(in, w, h, K, r, y, x);
        }
    }
}
 
static void v1(const unsigned char *in, unsigned char *out, int w, int h, const float *K, int r) {
    float *acc = (float *)malloc((size_t)w * sizeof(float));
    if (!acc) {
        fprintf(stderr, "sin memoria\n"); 
        exit(1);
    }
    v1_filas(in, out, w, h, K, r, 0, h, acc);
    free(acc);
}


int main(int argc, char **argv)
{
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <v0|v1> <blur|gauss|sobel> <entrada.pgm> [salida.pgm]\n", argv[0]);
    }

    // Versión 
    int version;
    if (strcmp(argv[1], "v0") == 0) {
        version = 0;
    } else if (strcmp(argv[1], "v1") == 0) {
        version = 1;
    } else {
        fprintf(stderr, "Version desconocida: %s\n", argv[1]);
        return 1;
    }

    // Filtro
    const Filtro *flt = NULL;
    for (int k = 0; k < N_FILTROS; k++) {
        if (strcmp(argv[2], FILTROS[k].nombre) == 0) {
            flt = &FILTROS[k];
        }
    }
    if (!flt) {
        fprintf(stderr, "Filtro desconocido: %s\n", argv[2]);
        return 1;
    }

    int w, h;
    unsigned char *in  = leer_pgm(argv[3], &w, &h);
    unsigned char *out = (unsigned char *)malloc((size_t)w * h);
    if (!out) {
        fprintf(stderr, "sin memoria\n");
        return 1;
    }

    // Medicion de 5 corridas y se reporta la mediana
    double t[REPS];
    for (int k = 0; k < REPS; k++) {
        double t0 = ahora();
        if (version == 0) {
            v0(in, out, w, h, flt->K, flt->r);
        } else {
            v1(in, out, w, h, flt->K, flt->r);
        }
        t[k] = ahora() - t0;
    }

    printf("mediana %.6f s\n", mediana(t, REPS));

    printf("checksum %" PRIu64 "\n", checksum(out, (size_t)w * h));

    if (argc >= 5) escribir_pgm(argv[4], out, w, h);

    free(in);
    free(out);
    return 0;
}