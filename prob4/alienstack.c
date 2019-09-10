
/*
 *  314ckC47 @ Stealien
 *  babystack.c - kongju keris pwnable task.
 *  $ gcc -o babystack babystack.c -m32 -no-pie
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#ifdef DEBUG
#define DPRINT(...) {               \
    fprintf(stderr, "[DEBUG] ");    \
    fprintf(stderr, __VA_ARGS__);   \
    fprintf(stderr, "\n");          \
}
#define ERROR(...) {                                        \
    fprintf(stderr, "[-] %s:%d | ", __func__, __LINE__);    \
    fprintf(stderr, __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                  \
    goto err;                                               \
}
#else
#define DPRINT(...)
#define ERROR(...) {                                        \
    fprintf(stderr, "[-] ");                                \
    fprintf(stderr, __VA_ARGS__);                           \
    fprintf(stderr, "\n");                                  \
    goto err;                                               \
}
#endif

#define PAGE_SIZE 0x1000

struct instruction {
    uint32_t cmd;
    uint32_t arg;
};

void *stack;
void *sp;
void *bp;
int stack_map(void);
int stack_unmap(void);
int stack_push(uint32_t val);
int stack_pop(uint32_t* ptr);
int stack_expand(uint32_t size);
int stack_shrink(uint32_t size);

int custom_main(void);
int custom_alloc(void **);
int custom_free(void **);
int custom_puts(void **);
int custom_gets(void **);
int custom_copy(void **);

void myinit(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin,  NULL, _IONBF, 0);
    stack   = NULL;
    sp      = NULL;
    bp      = NULL;
}

int main(void) {

    int ret;

    if (stack_map())
        ERROR("stack_map()");

    ret = custom_main();

    if (stack_unmap())
        ERROR("stack_unmap()");
    
    return ret;
err:
    return -1;
}

int custom_main(void) {

    uint32_t *sel;
    void **buf;

    /* prologue */
    stack_push((uint32_t)bp);
    bp = sp;

    /* allocate variables */
    stack_expand(sizeof(uint32_t));
    sel = (uint32_t *)sp;
    stack_expand(sizeof(*buf));
    buf = (void **)sp;

    while (true) {
        write(1, "sel> ", 5);
        scanf("%u", sel);
        getchar();
        switch (*sel) {
            case 0:
                custom_alloc(bp-0x8);
                break;
            case 1:
                custom_free(bp-0x8);
                break;
            case 2:
                custom_puts(bp-0x8);
                break;
            case 3:
                custom_gets(bp-0x8);
                break;
            case 4:
                custom_copy(bp-0x8);
            default:
                break;
        }
    }

    /* epilogue */
    sp = bp;
    stack_pop((uint32_t*)&bp);

    return 0;
}

int custom_alloc(void **ptr) {
    *ptr = malloc(0x200);
    return 0;
}

int custom_free(void **ptr) {
    free(*ptr);
    return 0;
}

int custom_puts(void **ptr) {
    puts(*ptr);
    return 0;
}

int custom_gets(void **ptr) {
    write(1, ">>> ", 4);
    fgets(*ptr, 0x200, stdin);
    return 0;
}

int custom_copy(void **dst) {

    void **src;

    /* prologue */
    stack_push((uint32_t)bp);
    bp = sp;

    /* allocate variables */
    stack_expand(0x200);
    src = (void **)sp;

    write(1, "input : ", 8);
    fgets(bp-0x200, 0x204, stdin);

    strncpy(*dst, bp-0x200, 0x100);

    /* epilogue */
    sp = bp;
    stack_pop((uint32_t*)&bp);

    return 0;
}

int stack_map(void) {

    /* Check if stack is already initialized. */
    if (stack) ERROR("stack already initialized.");
    
    /* Allocate stack */
    stack = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (stack == NULL)
        ERROR("mmap failed. errno=%d", errno);

    /* setup stack pointer */
    sp = stack + PAGE_SIZE;

    /* push sp for verification */
    if (stack_push((uint32_t)sp))
        ERROR("stack push failed.");

    /* push unmap fptr */
    if (stack_push((uint32_t)&munmap))
        ERROR("stack push failed.");

    /* setup base pointer */
    bp = sp;

    DPRINT("stack = %p", stack);
    DPRINT("sp = %p", sp);
    DPRINT("bp = %p", bp);

    /* stack successfully initialized. */
    return 0;
err:
    stack = NULL;
    sp = NULL;
    bp = NULL;
    return -1;
}

int stack_unmap(void) {

    void (*fp)(void *, uint32_t);
    void *verif_sp = NULL;
    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* shrink stack */
    sp = bp;

    /* pop fptr */
    if (stack_pop((uint32_t*)&fp))
        ERROR("stack pop failed.");

    /* pop sp */
    if (stack_pop((uint32_t*)&verif_sp))
        ERROR("stack pop failed.");

    if (sp != verif_sp)
        ERROR("stack verification failed.");

    /* unmapping stack */
    fp(stack, PAGE_SIZE);

    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_push(uint32_t val) {

    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    sp -= sizeof(val);
    
    /* Check stack size. */
    if (sp < stack)
        ERROR("stack limited.");

    *(uint32_t *)sp = val;

    DPRINT("*sp = 0x%x", *(uint32_t*)sp);

    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_pop(uint32_t *ptr) {

    uint32_t ret;
    void *saved_sp = sp;

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    ret = *(uint32_t*)sp;

    sp += sizeof(*ptr);

    /* Check stack size. */
    if (sp > stack + PAGE_SIZE)
        ERROR("stack limited.");

    DPRINT("ret = 0x%x", ret);

    *ptr = ret;
    return 0;
err:
    sp = saved_sp;
    return -1;
}

int stack_expand(uint32_t size) {

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* Check stack size */
    if (sp - size < stack)
        ERROR("stack limited.");

    sp -= size;

    DPRINT("sp = %p", sp);

    return 0;
err:
    return -1;
}

int stack_shrink(uint32_t size) {

    /* Check if stack initialized. */
    if (stack == NULL) ERROR("stack didn't initialized.");

    /* Check stack size */
    if (sp + size > stack + PAGE_SIZE)
        ERROR("stack limited.");

    sp += size;

    DPRINT("sp = %p", sp);

    return 0;
err:
    return -1;
}