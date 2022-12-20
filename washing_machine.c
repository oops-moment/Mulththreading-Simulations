#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
int count_students;
struct student_thread
{
    int entry_time;
    int washing_time;
    int patience_time;
    int index;
    pthread_cond_t signal;
};
struct student_thread struct_array[1000];
sem_t machine_semaphore;
pthread_mutex_t variable_lock;
int couldnot_wash = 0;
void sort(struct student_thread *studentthread, int N)
{
    int i, j;
    for (i = 0; i < N - 1; i++)
    {
        for (j = i + 1; j < N; j++)
        {
            struct student_thread *i1 = studentthread + i;
            struct student_thread *i2 = studentthread + j;
            if (i1->entry_time > i2->entry_time)
            {
                struct student_thread temp = *i1;
                *i1 = *i2;
                *i2 = temp;
            }
            else if (i1->entry_time == i2->entry_time)
            {
                if (i1->index > i2->index)
                {
                    struct student_thread temp = *i1;
                    *i1 = *i2;
                    *i2 = temp;
                }
            }
        }
    }
}
struct input
{
    int N;
    int M;
    int entry_time;
    int washing_time;
    int patience_time;
    int index;
    int iterator;
};
void print_white(char *string)
{
    printf("\033[0;37m");
    printf("%s\n", string);
    printf("\033[0m");
}
void print_green(char *string)
{

    printf("\033[0;32m");
    printf("%s\n", string);
    printf("\033[0m");
}
void print_yellow(char *string)
{

    printf("\033[0;33m");
    printf("%s\n", string);
    printf("\033[0m");
}
void print_red(char *string)
{

    printf("\033[0;31m");
    printf("%s\n", string);
    printf("\033[0m");
}
void kill_function(int N)
{
    pthread_mutex_destroy(&variable_lock);
    for (int i = 0; i < N; i++)
    {
        pthread_cond_destroy(&struct_array[i].signal);
    }
}
void ensure_fcfs(struct input *args, int time_already, int students)
{
    int time_come = args->entry_time + args->washing_time + time_already; // COMPARES WHO CAME FIRST
    int abhi_time = struct_array[students].entry_time;
    if (time_come < abhi_time)
    {
        sleep(abhi_time - time_come - 0.01); // FCFS IS ENSURED USING SLEEP
    }

    pthread_cond_signal(&struct_array[students].signal);
}
void *main_function(void *myarg)
{

    struct input *args = (struct input *)myarg;
    sleep(args->entry_time - 0.01);
    int time_gone = 0; // CHECK HOW MUCH TIME PERSON WAITED AFTER HE ARRIVED

    printf("\033[0;37m");
    printf("Student %d arrives\n", args->index);
    printf("\033[0m");
    if (sem_trywait(&machine_semaphore) == 0) // CHECKS IF ANY OF MACHINE IS FREE
    {

        printf("\033[0;32m");
        printf("Student %d starts washing\n", args->index);
        printf("\033[0m");
        pthread_mutex_lock(&variable_lock); // COUNT _STUDENTS KEEPS TRACK OF STUDENTS COMING
        count_students++;
        pthread_mutex_unlock(&variable_lock);
        sleep(args->washing_time - 0.01); // SLEEPS FOR WASHING TIME

        printf("\033[0;33m");
        printf("Student %d leaves after washing\n", args->index);
        printf("\033[0m");

        sem_post(&machine_semaphore); // unlock THE MACHINE
    }
    else
    {

        pthread_mutex_t mutex_lock;
        pthread_mutex_init(&mutex_lock, NULL);
        struct timespec ts; // STRUCT TIME PARAMTER TO CONDITONAL WAIT
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += args->patience_time;
        time_gone = time(NULL);
        pthread_mutex_lock(&mutex_lock);
        pthread_cond_timedwait(&struct_array[args->iterator].signal, &mutex_lock, &ts);
        // CHECKS AGAIN AND AGAIN OVER THE TIME IF PATIENCE IS OVER
        pthread_mutex_unlock(&mutex_lock);

        time_gone -= time(NULL);
        sleep(0.001);
        int started;
        if (sem_trywait(&machine_semaphore) == 0)
        {
            printf("\033[0;32m");
            printf("Student %d starts washing\n", args->index);
            printf("\033[0m");
            pthread_mutex_lock(&variable_lock);
            count_students++;
            pthread_mutex_unlock(&variable_lock);
            sleep(args->washing_time - 0.01);
            printf("\033[0;33m");
            printf("Student %d leaves after washing\n", args->index);
            printf("\033[0m");
            sem_post(&machine_semaphore);
        }
        else
        {
            // // int finish_time = args->entry_time + args->washing_time + time_gone;
            // int finish_time = started+args->entry_time + args->washing_time ;
            // printf("%d %d %d\n",started,args->entry_time,args->washing_time);
            // // 5+1+2
            // int now_time = struct_array[count_students].entry_time;
            // printf("%d finish time %d now time \n",finish_time,now_time);
            // if ((finish_time) - (now_time) >= args->patience_time)
            // {
            //     printf("hello\n");
            // }
            pthread_mutex_lock(&variable_lock);
            count_students++;
            pthread_mutex_unlock(&variable_lock);
            printf("\033[0;31m"); // Set the text to the color red
            printf("Student %d leaves without washing\n", args->index);
            printf("\033[0m");
            couldnot_wash++;
            return 0;
        }
        pthread_mutex_destroy(&mutex_lock);
    }
    ensure_fcfs(args, time_gone, count_students); // IF MULTIPLE THREADS WERE WAITING ONE THAT CAME INTIALLY WILL EXECUTE FIRST

    free(args);
}
// IN CASE OF QSORT THIS SERVES AS COMAPRATOR FUNCTION
int comparision(const void *p0, const void *p1)
{
    struct student_thread *ps0 = (struct student_thread *)p0;
    struct student_thread *ps1 = (struct student_thread *)p1;

    if (ps0->entry_time < ps1->entry_time)
    {
        return -1;
    }
    else if (ps0->entry_time == ps1->entry_time)
    {
        if (ps0->index < ps1->index)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 1;
    }
}

int main()
{
    int N, M; // N : STUDENTS , M: WASHING MACHINES
    scanf("%d %d", &N, &M);
    count_students = 0;
    for (int i = 0; i < N; i++) // TAKING INPUT AS ARRAY OF STRUCTS
    {
        int t, w, p;
        scanf("%d %d %d", &t, &w, &p);
        struct_array[i].entry_time = t;
        struct_array[i].patience_time = p;
        struct_array[i].index = (i + 1);
        struct_array[i].washing_time = w;
    }
    pthread_mutex_init(&variable_lock, NULL); // FOR THE STUDENT COUNTER
    pthread_t thread[N];
    sem_init(&machine_semaphore, 0, M); // SEMAPHORE FOR M MACHINES
    for (int i = 0; i < N; i++)
    {
        pthread_cond_init(&struct_array[i].signal, NULL);
    }

    sort(struct_array, N); // SORTING STRUCTS BASED ON THE STARTING TIME
    // for (int i = 0; i < N; i++)
    // {
    //     printf("%d\n", struct_array[i].index);
    // }
    for (int i = 0; i < N; i++)
    {
        if (i != 0 && struct_array[i].entry_time == struct_array[i - 1].entry_time)
        {
            sleep(0.01); // IN CASE THEY ARRIVE EQUALLY INTROUDUCING A DELAY.
        }
        struct input *arg = malloc(sizeof(struct input));
        arg->entry_time = struct_array[i].entry_time;
        arg->index = struct_array[i].index;
        arg->M = M;
        arg->N = N;
        arg->patience_time = struct_array[i].patience_time;
        arg->iterator = i;
        arg->washing_time = struct_array[i].washing_time;
        // SENDING ARGUMENT AS THIS STRUCT INPUT
        pthread_create(&thread[i], NULL, &main_function, arg); // STUDENTS SERVE AS THREADS SO TOTAL N THREADS
    }
    for (int i = 0; i < N; i++)
    {
        pthread_join(thread[i], NULL);
    }
    kill_function(N); // DESTROYING MUTEX LOCKS IF ANY
    printf("%d\n", couldnot_wash);
    int threshold = 0.25 * N; // CHECKING IF PEOPLE WHO WENT WITHOUT WASHING IS GREATER THAN 25 % OR NOT
    if (couldnot_wash > threshold)
    {
        printf("\033[0;37m");
        printf("YES\n");
        printf("\033[0m");
    }
    else
    {
        printf("\033[0;37m");
        printf("NO\n");
        printf("\033[0m");
    }
    return 0;
}