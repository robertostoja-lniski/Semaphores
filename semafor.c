#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MAX_BUFFER_AMOUNT 20
#define TRUE 1
#define FALSE 0
#define TYPES_NUM 3
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PSHARED 1
#define SLEEP_TIME_NANO_SEC 1
#define SLEEP_TIME_SEC 1
#define NUMBER_OF_PRODUCTS 20000
#define SIGKILL 9

sem_t *mutex;
pid_t *customerTab;

int bufferNum;
int customerNum;
int testType;
int counter = 0;

struct Buffer{
    
    sem_t full;
    sem_t empty;
    char type;
    int amount;
};

struct Buffer *tab;

struct Consumer {

    int code;
    int requestA;
    int requestB;
    int requestC;
};
void Consumer(struct Consumer *K, int code, int A, int B, int C) {

    K -> code = code;
    K -> requestA = A;
    K -> requestB = B;
    K -> requestC = C;
}
void printTab() {

    int index;
    
    for(index = 0; index < bufferNum; ++index) {

        printf("tab nr %d is type %c and has %d amount\n", 
                index, tab[index].type, tab[index].amount );
    }
    puts("");

}
int chooseBuffer() {
    int seed, index;
    time_t tt;

    seed = time(&tt);
    srand(seed);
    index = rand() % bufferNum;
    int cap = 100;
    do {
	
	if(cap <= 0 ) {
	   
	    for(index = 0; index < bufferNum; index++) {
			
		if(tab[index].amount < MAX_BUFFER_AMOUNT) {
				
			return index;
		}
	    }
	   
	}
	
	index = rand() % bufferNum;
	cap --;

   }while(tab[index].amount == MAX_BUFFER_AMOUNT);

   return index;
}
void produceItem(int index) {

    tab[index].amount++;  
}
void producer() {

    int i = NUMBER_OF_PRODUCTS;
    int index;
    while (i--) {

        index = chooseBuffer(); 
        sem_wait(&tab[index].empty);
        sem_wait(mutex);
	
        if(testType <= 0) {
            puts("prod entered");
        }

        counter++;
        printf("Test progress:\n");
        printf("%.5f %%\n", (double)counter/NUMBER_OF_PRODUCTS * 100);
        produceItem(index);
        if(testType > 0 ) {
            
            printTab();
        }
        if(testType <= 0) {
            puts("prod exited");
            puts("");
        }
 	
	sem_post(&tab[index].full);
        sem_post(mutex);
        

        usleep(SLEEP_TIME_NANO_SEC);
	//sleep(SLEEP_TIME_SEC);

    }
}
void bufferDownFull(struct Consumer *K) {
	
   // 0 A, 1 B, 2 C
    int neededTab[TYPES_NUM];
    
    neededTab[0] = K -> requestA;
    neededTab[1] = K -> requestB;
    neededTab[2] = K -> requestC;

    int index, numericBufferType, productsToGet, removed;
    for(index = 0; index < bufferNum; ++index) {

       numericBufferType = (int)(tab[index].type - 'A');

       if(neededTab[numericBufferType] > 0) {

           productsToGet = MIN(neededTab[numericBufferType], tab[index].amount);
           neededTab[numericBufferType] -= productsToGet;
        
           for(removed = 0; removed < productsToGet; ++removed) {
               sem_wait(&tab[index].full);
     
           }
       }
    }
    printf("Consumer %d took products\n", K -> code );
}

void get(struct Consumer *K) {
    // 0 A, 1 B, 2 C
    int neededTab[TYPES_NUM];
    
    neededTab[0] = K -> requestA;
    neededTab[1] = K -> requestB;
    neededTab[2] = K -> requestC;

    int index, numericBufferType, productsToGet, removed;
    for(index = 0; index < bufferNum; ++index) {

       numericBufferType = (int)(tab[index].type - 'A');

       if(neededTab[numericBufferType] > 0) {

           productsToGet = MIN(neededTab[numericBufferType], tab[index].amount);
           neededTab[numericBufferType] -= productsToGet;
           tab[index].amount -= productsToGet;

           for(removed = 0; removed < productsToGet; ++removed) {
               sem_wait(&tab[index].full);
               sem_post(&tab[index].empty);
           }
       }
    }
    printf("Consumer %d took products\n", K -> code );
}
int check(struct Consumer *K) {

    // 0 A, 1 B, 2 C
    int neededTab[TYPES_NUM];
    
    neededTab[0] = K -> requestA;
    neededTab[1] = K -> requestB;
    neededTab[2] = K -> requestC;

    int index, numericBufferType, productsToGet;
    for(index = 0; index < bufferNum; ++index) {

   
       numericBufferType = (int)(tab[index].type - 'A');

       if(neededTab[numericBufferType] > 0) {

           productsToGet = MIN(neededTab[numericBufferType], tab[index].amount);
           neededTab[numericBufferType] -= productsToGet;
       }
    }
    if(neededTab[0] == 0 && neededTab[1] == 0 && neededTab[2] == 0) {
        
        return 1;
    }
    return 0;

}
void consumer(struct Consumer *K) {

    while (TRUE)
    {
        if(check(K)) {
	    
            sem_wait(mutex);
            if (testType <= 0) {
                printf("Consumer %d entered\n", K->code);
            }
            if (check(K)) {
                get(K);
            }
            if (testType <= 0) {
                printf("Consumer %d exited\n\n", K->code);
            }
            sem_post(mutex);

           // usleep(SLEEP_TIME_NANO_SEC);
	    sleep(SLEEP_TIME_SEC);
        }
    
    }
}
void generateBuffers() {

    int seed, index, productType;
    time_t tt;
    
    // zakladam, ze zawsze istnieja bufory 
    // kazdego z typow A B C
    tab[0].type = 'A';
    tab[1].type = 'B';
    tab[2].type = 'C';

    seed = time(&tt);
    srand(seed);

    printf("Buffers created are: ABC");
    for(index = 0; index < bufferNum; ++index) {

        tab[index].amount = 0;
    }
    for(index = 3; index < bufferNum; ++index) {

        productType = rand() % 3 + 'A';
        printf("%c", productType);
        tab[index].type = productType;

    }
    printf("\n");
   
}
void initSemaphores() {

    sem_init( mutex, PSHARED, 1);

    int index;
    for(index = 0; index < bufferNum; index++ ) {
         
         sem_init( &tab[index].empty, PSHARED, MAX_BUFFER_AMOUNT);
         sem_init( &tab[index].full, PSHARED, 0);
         tab[index].amount = 0;
    }

}
void allocateSharedTab() {

    int size = sizeof (*tab) * bufferNum;
    int shm;

    tab = malloc(size);
    shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);

    if (shm == -1) {
        printf("Failed to open mem errno: %d\n", errno);
        return ;
    }

    ftruncate(shm, size);
    tab = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    if (tab == MAP_FAILED) {
        printf("Failed to share mem errno: %d\n", errno);
        return ;
    }
    shm_unlink("/shm");

}
void allocateSharedMutex() {

    int size = sizeof(sem_t);
    mutex = malloc(size);

    int shm = shm_open("/shm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXO);

    if (shm == -1) {
        printf("Failed to open mem errno: %d\n", errno);
        return ;
    }

    ftruncate(shm, size);
    mutex = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0);

    if (tab == MAP_FAILED) {
        printf("Failed to share mem errno: %d\n", errno);
        return ;
    }
    shm_unlink("/shm");
}
void clear() {

    int index;
    for(index = 0; index < customerNum; ++index) {

        kill (customerTab[index], SIGKILL);

    }
    munmap(tab, sizeof (*tab) * bufferNum);
  
}
void createCustomers() {

    customerTab = malloc(sizeof(pid_t)*customerNum);
    
    int index;
    for(index = 0; index < customerNum; ++index) {

        pid_t pidK1 = fork();
        if (pidK1 == 0) {

            struct Consumer *K1 = (struct Consumer *)malloc(sizeof(struct Consumer));
            if(index % 2) {
                Consumer(K1, index, 3, 2, 1);
            } else {
                Consumer(K1, index, 1, 2, 3);
            }
            
            consumer(K1);
        }
        customerTab[index] = pidK1;
    }
}
void readTestParams() {
	
    printf("Podaj ilosc buforow i klientow\n");
    scanf("%d %d", &bufferNum, &customerNum); 

    printf("Podaj rodzaj testu,\n0 - wejscia do sekcji krytycznej\n1 - zawartosc bufora\n");
    scanf("%d", &testType);
}
int main(){

    readTestParams();
    allocateSharedMutex();
    allocateSharedTab();

    initSemaphores();
    generateBuffers();
    
    createCustomers();

    producer();
  
    clear();
    return 0;
}
