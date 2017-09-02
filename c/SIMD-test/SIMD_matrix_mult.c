#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>
#include <time.h>
#include <stdbool.h>



bool print = false;
size_t M;
size_t N;
size_t O;
size_t limit = 10;

float* random_matrix(size_t m, size_t n, size_t limit);
void print_matrix(float* mat, size_t m, size_t n);
bool equal_mtrx(float* A, float* B, size_t len);

float* MULT(float* A, float* B, size_t m, size_t n, size_t o);
void SIMD_MULT(float *A, float *B, float *C, size_t m, size_t n, size_t o);

int main(int argc, char *argv[]){

		if (argc > 1 && atoi(argv[1]) > 10) limit = atoi(argv[1]);

		clock_t start, end;
		srand(clock());

		M = rand() % limit/2 + limit/2;
		N = rand() % limit/2 + limit/2;
		O = (rand() % limit/8 + 1 + limit/16) * 4;


		float* A = random_matrix(M, N, 10);
		float* B = random_matrix(N, O, 10);
		float* C = malloc(sizeof(float) * M * O);
		float* D;

		printf("Generadas dos matrices aleatorias de (%zu x %zu) y (%zu x %zu)\n", M, N, N, O);

		if (print) {
			print_matrix(A, M, N);
			printf("\n");
			print_matrix(B, N, O);
			printf("\n");
		}

		start = clock();
		D = MULT(A, B, M, N, O);
		end = clock();

		printf("RESULTADO FUERZA BRUTA: (%f)\n", ((double)(end - start))/CLOCKS_PER_SEC);
		if (print) print_matrix(D, M, O);



		start = clock();
		SIMD_MULT(A, B, C, M, N, O);
		end = clock();

		printf("RESULTADO SIMD: (%f)\n", ((double)(end - start))/CLOCKS_PER_SEC);
		if (print) print_matrix(C, M, O);

		if (equal_mtrx(C, D, M*O)) printf("Son iguales!\n");
		else printf("NO son iguales!\n");

		free(A);
		free(B);
		free(C);
		free(D);
}

/** esta es una función que entrega una matriz random con dimensiones (MxN) */
float* random_matrix(size_t m, size_t n, size_t limit){
	float* d = malloc(sizeof(float) * m * n);
	for (size_t i = 0; i < n*m; i++) {
		d[i] = rand() % limit;
	}
	return d;
}

bool equal_mtrx(float* A, float* B, size_t len){
	for (size_t i = 0; i < len; i++) {
		if (A[i] != B[i]) return false;
	}
	return true;
}


void print_matrix(float* mat, size_t m, size_t n){
	for (int i = 0; i < m*n; i += n){
		for (int j = 0; j < n; ++j){
			printf("%6.0f ", mat[i + j]);
		}
		printf("\n");
	}
}

float* MULT(float* A, float* B, size_t m, size_t n, size_t o){
	float *C = malloc(sizeof(float) * m * o);
	for (int i = 0; i < m; ++i){
    for (int j = 0; j < o; ++j){
			C[i*o + j] = 0;
			for (int k = 0; k < n; k++) {
				C[i*o + j] += A[i*n + k] * B[k*o + j];
			}
		}
	}
	return C;
}

/** 'o' DEBE ser múltiplo 4 */
void SIMD_MULT(float *A, float *B, float *C, size_t m, size_t n, size_t o){
	__m128 row, row_j, brod_j;

	/* Iteramos sobre la cantidad de filas en la matriz output */
	for (size_t i = 0; i < m; i++) {

		/* elegimos las 4 columnas que utilizaremos en esta iteración */
		for (size_t k = 0; k < o; k += 4) {

			/* i-esima fila de la nueva matriz*/
		 	row = _mm_set1_ps(0);

			/* Iteramos sobre la cantidad de elementos a multiplicar */
			for (size_t j = 0; j < n; j++) {
				/* j-esima fila de la matriz B */
				row_j = _mm_load_ps(&B[o*j + k]);
				/* vector de 4 copias del j-esimo elemnto de la i-esima fila de la matriz A */
				brod_j = _mm_set1_ps(A[n*i + j]);
				/* agregamos la multiplicacion de estos dos vectores */
				row = _mm_add_ps(row, _mm_mul_ps(brod_j, row_j));
			}

			/* guardamos la i-esima fila en la matriz nueva */
			_mm_store_ps(&C[o*i + k], row);
		}
	}
}

/*
_mm_store_ps(&B[i], r);              // guarda r en 4 posiciones (desde B[i] hasta B[i+3])
__m128 r = _mm_load_ps(&B[i]) 				// carga 4 posiciones (desde B[i] hasta B[i+3]) en r
__m128 r = _mm_set1_ps(f)     				// setea 4 veces el valor f en r
__m128 r = _mm_add_ps(a, b); 			 		// guarda a + b en r
__m128 r = _mm_mul_ps(brod_j, row_j)  // guarda a * b en r
*/
