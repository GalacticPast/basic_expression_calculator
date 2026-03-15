#include "defines.h"
#include "parser.h"
#include "tokenizer.h"

static arena           *main_arena;
static tokenizer_state *token_state;

char *split(char *expression, int length)
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

void run_tests(arena *arena)
{
    const char *file_name = "src/tests.txt";

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Couldn't open file : %s\n", file_name);
        DEBUG_BREAK;
    }

    char  expression[124] = {};
    float expected_result = -1;
    int   count           = 1;
    float epsilon         = 0.0001;
    while (fgets(expression, 123, file) != NULL)
    {
        char *split_exp = split(expression, 123);
        *split_exp      = '\0';
        expected_result = atof(split_exp + 1);

        printf("No %d -> Exp: %s, exp result: %3f", count, expression, expected_result);
        float ans = evaluate(arena, expression);
        printf(",got: %2f\n", ans);
        ASSERT(expected_result, ans, 0.0001);
        arena_reset(main_arena);
        token_state = NULL;
        count++;
    }
}
bool is_equal(const char *str_a, const char *str_b)
{
    int str_a_length = strlen(str_a);
    int str_b_length = strlen(str_b);

    if (str_a_length != str_b_length)
    {
        return false;
    }
    for (int i = 0; i < str_a_length; i++)
    {
        if (str_a[i] != str_b[i])
        {
            return false;
        }
    }
    return true;
}

int main()
{
    // n * KB
    int  size = 10 * 1024;
    char array[size];

    arena arena   = arena_init(&array, size);
    main_arena    = &arena;
    float epsilon = 0.0000001;
#if 0
     run_tests(&arena);
#endif

    printf("Type in your expression: \n");
    while (true)
    {
        char expression[30];
        // Read input from stdin
        fgets(expression, sizeof(expression), stdin);
        if (is_equal("exit\n", expression))
            break;

        float ans       = evaluate(main_arena, expression);
        float ans_floor = floor(ans);
        if (fabs(ans_floor - ans) < epsilon)
        {
            printf("%d\n", (int)ans);
        }
        else
        {
            printf("%.2f\n", ans);
        }
        arena_reset(main_arena);
    }
    return 0;
}
