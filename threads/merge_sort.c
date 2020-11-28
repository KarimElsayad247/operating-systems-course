#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    int *array;
    int start;
    int end;
} params;


void print_array(int *array, int size)
{
    for (int i = 0; i < size; ++i)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

void merge(int *array, int start, int mid, int end)
{

    // create temporary arrays and store values of both halfs in them
    int arr1_size = mid  - start + 1;
    int *array1 = malloc(sizeof(int) * arr1_size);
    int arr2_size = end - mid;
    int *array2 = malloc(sizeof(int) * arr2_size) ;
    // int arr3_size = end - start + 1;
    // int *array3 = malloc(sizeof(int) * arr3_size);

    // left half
    for (int i = 0; i < arr1_size; ++i)
        array1[i] = array[start + i];

    // printf("left half\n");
    // print_array(array1, arr1_size);

    // right half
    for (int i = 0; i < arr2_size; ++i)
        array2[i] = array[mid + i + 1];

    // printf("right half\n");
    // print_array(array2, arr2_size);

    // index for left half
    int i = 0;

    // index for right half
    int j = 0;

    // index for combined array
    int k = 0;

    while (i < arr1_size && j < arr2_size)
    {
        if (array1[i] > array2[j])
        {
            array[start + k] = array2[j];
            ++j;
        }
        else
        {
            array[start + k] = array1[i];
            ++i;
        }
            ++k;
    }

    // one array has finished and the other still hasn't
    
    // copy rest of array1
    while (i < arr1_size)
    {
        array[start + k++] = array1[i++];
    }
    
    // copy rest of array2
    while (j < arr2_size)
    {
        array[start + k++] = array2[j++];
    }

    // printf("combined\n");
    // print_array(array, end - start + 1);

    free(array1);
    free(array2);
}

int assert_equal_arrays(int *array1, int *array2, int size)
{
    int i = 0;
    while (i < size)
    {
        if (array1[i] != array2[i])
            return 0;
        ++i;
    }

    return 1;
}

void normal_merge_sort(int *array, int start, int end)
{
    if (end > start)
    {
        int mid = (start + end) / 2;

        normal_merge_sort(array, start, mid);
        normal_merge_sort(array, mid + 1, end);
        merge(array, start, mid, end);
    }
}

void *merge_sort(void *args)
{
    params *actual_params = args;
    // printf("in merge sort\n");
    // printf("%d %d\n", actual_params->end, actual_params->start);
    // print_array(actual_params->array, actual_params->end - actual_params->start + 1);

    if (actual_params->end > actual_params->start)
    {
        int mid = (actual_params->start + actual_params->end) / 2;

        pthread_t thread_id;
        pthread_t thread_id2;

        params *arg_left = malloc(sizeof(params));
        params *arg_right = malloc(sizeof(params));

        arg_left->array = actual_params->array;
        arg_left->start = actual_params->start;
        arg_left->end = mid;

        arg_right->array = actual_params->array;
        arg_right->start = mid + 1;
        arg_right->end = actual_params->end;

        pthread_create(&thread_id, NULL, merge_sort, arg_left);
        pthread_create(&thread_id2, NULL, merge_sort, arg_right);

        pthread_join(thread_id, NULL);
        pthread_join(thread_id2, NULL);

        // merge_sort(arg_left);
        // merge_sort(arg_right);
        merge(actual_params->array, actual_params->start, mid, actual_params->end);
        free(arg_left);
        free(arg_right);
    }
     
     return NULL;
}

// read array from a given file
int *read_array(int size, FILE *input_file)
{
    int *array = malloc(sizeof(int) * size);

    // read a total of $size integers from file
    for (int i = 0; i < size; ++i)
    {
        fscanf(input_file, "%d", &array[i]);
    }
    return array;
}

int main() 
{

    // open input file
    FILE *input_file = fopen("file50k.txt", "r");

    // if input file doesn't exist, program shouldn't run
    if (input_file == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }

    int number_of_arrays;
    fscanf(input_file, "%d", &number_of_arrays);

    for (int i = 0; i < number_of_arrays; ++i)
    {  
        // read the size of array from file and store in variable                  
        int size = 0;
        fscanf(input_file, "%d", &size);

        // read elements of array from file
        int *array = read_array(size, input_file);
        int *solution = read_array(size, input_file); 


        params *args = malloc(sizeof(params));
        args->array = array;
        args->start = 0;
        args->end = size - 1;

        clock_t begin = clock();
        merge_sort(args);
        clock_t end = clock();

        clock_t begin2 = clock();
        normal_merge_sort(array, 0, size - 1);
        clock_t end2 = clock();

        double t1 = (double)(end - begin)/ CLOCKS_PER_SEC;
        double t2 = (double)(end2 - begin2)/ CLOCKS_PER_SEC;


        printf("threads time = %f VS normal time = %f\n",t1, t2);

        // if (assert_equal_arrays(array, solution, size))
        // { 
        //     printf("test %d passed WITH threads time = %f VS normal time = %f\n", i, t1, t2);
        // }
        // else
        // {
        //     printf("TEST FAILED %d\n", i);
        //     print_array(array, size);
        //     print_array(solution, size);
        // }
        

        free(args);
        free(array);
        free(solution);
        
    }
    return 0;
}