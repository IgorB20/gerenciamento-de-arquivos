#include <stdio.h>

int main()
{
    int a = 10;
    int *p = &a;

    printf("Valor de a: %d\n", a);
    printf("Endereço de a: %p\n", (void *)&a);
    printf("Valor de p: %p\n", (void *)p);
    printf("Valor apontado por p: %d\n", *p);
    printf("Endereço de p: %p\n", &p);
    return 0;
}