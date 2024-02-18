#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

struct complex {
    double real;
    double imag;
    int color;
};

void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE *pgmimg;
    int temp;
    pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n"); // Writing Magic Number to the File
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT); // Writing Width and Height
    fprintf(pgmimg, "255\n");                   // Writing the maximum gray value
    int count = 0;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            temp = image[i][j];
            fprintf(pgmimg, "%d ", temp); // Writing the gray values in the 2D array to the file
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
}

int cal_pixel(struct complex c) {
    

            double z_real = 0;
            double z_imag = 0;

            double z_real2, z_imag2, lengthsq;

            int iter = 0;
            do {
                z_real2 = z_real * z_real;
                z_imag2 = z_imag * z_imag;

                z_imag = 2 * z_real * z_imag + c.imag;
                z_real = z_real2 - z_imag2 + c.real;
                lengthsq =  z_real2 + z_imag2;
                iter++;
            }
            while ((iter < MAX_ITER) && (lengthsq < 4.0));
            return iter;

}


int main() {
    int image[HEIGHT][WIDTH];
    struct complex c;
    int MyRank, NumProcesses;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
    MPI_Comm_size(MPI_COMM_WORLD, &NumProcesses);
    int s = 480 / NumProcesses;
    double AVG = 0;
    int N = 10; // number of trials
    double total_time[N];
    int temp[s][WIDTH];
        clock_t start_time;
    for(int k=0;k<10;k++){
    if(MyRank==0){
	start_time = clock(); // Start measuring time
	for (int i = 0; i < s; i++) {
        for (int j = 0; j < WIDTH; j++) {
		c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
		c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
		temp[i][j] = cal_pixel(c);
		}
	}
    }
    else{
	for (int i = s*	MyRank; i < (MyRank + 1) * s; i++) {
        	for (int j = 0; j < WIDTH; j++) {
            		c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            		c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
            		temp[i - MyRank * s][j] = cal_pixel(c);
        	}
    	}
    }
    	MPI_Barrier(MPI_COMM_WORLD); // Synchronize processes
    	if(MyRank==0){
    		clock_t end_time = clock(); // End measuring time
        	total_time[k] = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
        	printf("Execution time of trial [%d]: %f seconds\n", k, total_time[k]);
       		AVG += total_time[k];
		}
	MPI_Gather(temp, s * WIDTH, MPI_INT, image, s * WIDTH, MPI_INT, 0, MPI_COMM_WORLD);
 
    }
    	if(MyRank==0){
		save_pgm("mandelbrot.pgm", image);
		printf("The average execution time of 10 trials is: %f ms\n", AVG/N*1000);
 	   	}
 	

    MPI_Finalize();
    return 0;
}
