#include <iostream>
#include <mpi.h>
#include <vector>
#include <cstdlib>
#include <ctime>

using std::vector;
using std::cout;
using std::endl;

typedef vector<vector<int>> Matrix;

void fillMatrixWithRandomValue(int n, int m, Matrix& currentMatrix);
int countNegative(const vector<vector<int>>& rows);
void printMatrix(const Matrix& matrix);

int main(int argc, char** argv) {
    srand(time(nullptr));
    bool flag = false;

    int n, m;
    if(argc > 2){
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    else{
        n = 6;
        m = 5;
    }
    
    if(n < 15 && m < 15) flag = true;

    Matrix matrix(n, vector<int>(m));

    fillMatrixWithRandomValue(n, m, matrix);
    
    
    MPI_Init(&argc, &argv);
    double start_time = MPI_Wtime();
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(rank == 0) {
        cout << "\nLab4: Parallel Programming with MPI\n";
        cout << "Group: DS-31\n";
        cout << "Task Variant: 4\n";
        cout << "Student: Lytvak Oleksandra\n";

        if(flag){
            cout << "Generated Matrix:\n";
            printMatrix(matrix);
        }
    }

    vector<vector<int>> rows;
    int rows_per_process = n / size;
    int remaining_rows = n % size;
    int start_row = rank * rows_per_process + std::min(rank, remaining_rows);
    int rows_to_receive = rows_per_process + (rank < remaining_rows ? 1 : 0);

    if(rank == 0){
        int send_row_start = 0;
        for (int i = 1; i < size; ++i) {
            int rows_for_process = rows_per_process + (i < remaining_rows ? 1 : 0);
            MPI_Send(&matrix[send_row_start][0], rows_for_process * m, MPI_INT, i, 0, MPI_COMM_WORLD);
            send_row_start += rows_for_process;
        }
        rows = vector<vector<int>>(matrix.begin() + start_row, matrix.begin() + start_row + rows_to_receive);
    } else {
        // Получаем данные в одномерный массив
        vector<int> temp(rows_to_receive * m);
        MPI_Recv(temp.data(), rows_to_receive * m, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Преобразуем одномерный массив в двумерный вектор
        rows = vector<vector<int>>(rows_to_receive, vector<int>(m));
        for (int i = 0; i < rows_to_receive; ++i) {
            for (int j = 0; j < m; ++j) {
                rows[i][j] = temp[i * m + j];
            }
        }
    }

    int local_count = countNegative(rows);
    MPI_Barrier(MPI_COMM_WORLD);
    if(flag){
        cout << "\nProcess " << rank << " is handling rows from " << start_row << " to "
        << start_row + rows_to_receive - 1 << endl;
        printMatrix(rows);
        cout << "Process " << rank << " found " << local_count << " negative numbers." << endl;
    }
    int global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    double local_execution_time = end_time - start_time;

    MPI_Finalize();
    
    if(rank == 0){
        cout << "\nNumber of unprocessed rows: " << remaining_rows;
        cout << "\nTotal number of negative elements in the matrix: "
        << global_count;
        cout << "\nExecution time: " << local_execution_time << endl;
    }
    return 0;
}

void fillMatrixWithRandomValue(int n, int m, Matrix& currentMatrix) {
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m; j++){
            currentMatrix[i][j] = rand() % 201 - 100;
        }
    }
}

int countNegative(const Matrix& matrix){
    int count = 0;
    for(const auto& row : matrix){
        for(int val : row){
            if(val < 0)
                ++count;
        }
    }
    return count;
}

void printMatrix(const Matrix& matrix){
    for(const auto& row : matrix){
        for(const auto& elem : row){
            cout << elem << " ";
        }
        cout << endl;
    }
}
