#define DB_IMPLEMENTATION
#include "db.h"
#include "parser.h"
#include <math.h>
#include <stdlib.h>

char *split(char *expression, s32 length)
{
    if (expression == NULL)
    {
        DEBUG_BREAK;
        return NULL;
    }

    char *ptr = expression;

    while (*ptr != '\n' && *ptr != '\0')
    {
        if (*ptr == '=')
        {
            return ptr;
        }
        ptr++;
    }
    return NULL;
}

void run_tests(db_arena *arena)
{
    const char *file_name = "src/tests.txt";

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Couldn't open file : %s\n", file_name);
        DEBUG_BREAK;
    }

    char expression[124] = {};
    f32  expected_result = -1;
    s32  count           = 1;
    f32  epsilon         = 0.0001;
    while (fgets(expression, 123, file) != NULL)
    {
        char *split_exp = split(expression, 123);
        *split_exp      = '\0';
        expected_result = atof(split_exp + 1);

        printf("No %d -> Exp: %s, exp result: %3f", count, expression, expected_result);
        f32 ans = evaluate(arena, expression);
        printf(",got: %2f\n", ans);
        db_arena_reset(arena);
        count++;
    }
}
b8 is_equal(const char *str_a, const char *str_b)
{
    s32 str_a_length = strlen(str_a);
    s32 str_b_length = strlen(str_b);

    if (str_a_length != str_b_length)
    {
        return false;
    }
    for (s32 i = 0; i < str_a_length; i++)
    {
        if (str_a[i] != str_b[i])
        {
            return false;
        }
    }
    return true;
}

s32 main()
{

    db_arena main_arena = db_arena_init();
    f32      epsilon    = 0.0000001;
#if 0
     run_tests(&arena);
#endif

    printf("Type in your expression: \n");
    while (true)
    {
        char expression[512];
        // Read input from stdin
        fgets(expression, sizeof(expression), stdin);
        if (is_equal("exit\n", expression))
            break;

        f32 ans       = evaluate(&main_arena, expression);
        f32 ans_floor = floor(ans);
        if (fabs(ans_floor - ans) < epsilon)
        {
            printf("%d\n", (s32)ans);
        }
        else
        {
            printf("%.2f\n", ans);
        }
        db_arena_reset(&main_arena);
    }
    db_arena_free(&main_arena);
    return 0;
}
