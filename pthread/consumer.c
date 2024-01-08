#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>  //系统调用相关头文件

//创建互斥量
pthread_mutex_t mutex;

//创建条件变量
pthread_cond_t cond;

//定义链表节点
struct Node{
    int val;
    struct Node* next;
};

//定义头结点
struct Node* head = NULL;

//头插法增加新节点
void* producer(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        //给节点分配空间
        struct Node* newnode = (struct Node*)malloc(sizeof(struct Node));
        newnode->val = rand()%1000;
        newnode->next = head;
        head = newnode;
        printf("add node , value: %d, tid: %ld \n", newnode->val, pthread_self());

        //只要生产了一个，就通知消费者消费
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        //挂起100微秒
        usleep(100);
    }
    return NULL;
}

//消费者
//头删法删除元素
void* consumer(void* arg){
    while(1){
        pthread_mutex_lock(&mutex);
        struct Node* tmp = head;
        //当链表不空时，才进行删除
        if(head != NULL){
            head = head->next;
            printf("del node , value: %d, tid: %ld \n", tmp->val, pthread_self());

            free(tmp);
            pthread_mutex_unlock(&mutex);

            usleep(100);
        }else{
            // 没有数据，需要等待
            // 当这个函数调用阻塞的时候，会对互斥锁进行解锁，当不阻塞的，继续向下执行，会重新加锁。
            pthread_cond_wait(&cond, &mutex);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

int main()
{
    //初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    //创建5个生产者线程和消费者线程
    pthread_t products[5], consumes[5];
    for(int i = 0; i < 5; i++){
        pthread_create(&products[i],NULL,producer,NULL);
        pthread_create(&consumes[i],NULL,consumer,NULL);
    }

    //分离，回收线程资源
    for(int i = 0; i < 5; i++){
        pthread_detach(products[i]);
        pthread_detach(consumes[i]);
    }

    while (1)
    {
        sleep(10);
    }

    //回收条件变量和互斥锁
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    //回收主线程
    pthread_exit(NULL);
    return 0;

    
}