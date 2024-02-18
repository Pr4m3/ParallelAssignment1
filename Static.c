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

void Master(int NumProcesses, int s, struct complex c, int image[HEIGHT][WIDTH]) {
    int row;
    for (int i = 0; i < NumProcesses; i++, row += s) {
        MPI_Send(&row, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < s; i++) {
        for (int j = 0; j < WIDTH; j++) {
            c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
            image[i][j] = cal_pixel(c);
        }
    }
        for (int i = 0; i < 480*640; i++) {
            MPI_Recv(&c, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            image[(int)c.real][(int)c.imag] = c.color;
        }

    save_pgm("mandelbro.pgm", image);
}

void Slave(int s, struct complex c, int image[HEIGHT][WIDTH]) {
    int row;
    MPI_Recv(&row, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // receive row start from master
    for (int i = row; i < row + s; i++) {
        for (int j = 0; j < WIDTH; j++) {
            c.real = (j - WIDTH / 2.0) * 4.0 / WIDTH;
            c.imag = (i - HEIGHT / 2.0) * 4.0 / HEIGHT;
            c.color = cal_pixel(c);
            MPI_Send(&c, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); // send coordinates and color to master
        }
    }
}

int main() {
    int image[HEIGHT][WIDTH];
    struct complex c;
    int MyRank, NumProcesses;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
    MPI_Comm_size(MPI_COMM_WORLD, &NumProcesses);
    int s = 480 / NumProcesses;
    
    if (MyRank == 0) {
        Master(NumProcesses, s, c, image);
    } else {
        Slave(s, c, image);
    }

    printf("complete\n");

    MPI_Finalize();

    return 0;
}
