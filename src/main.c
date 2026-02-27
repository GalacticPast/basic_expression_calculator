#include "defines.h"
#include "parser.h"
#include "tokenizer.h"

static arena           *main_arena;
static tokenizer_state *token_state;


int get_terminal_width(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col; // make sure your main returns int
}

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

void run_tests(arena* arena)
{
    const char *file_name = "src/tests.txt";

    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Couldn't open file : %s\n", file_name);
        DEBUG_BREAK;
    }

    char expression[124] = {};
    int  expected_result = INT_MIN;
    int  count           = 1;
    while (fgets(expression, 123, file) != NULL)
    {
        char *split_exp = split(expression, 123);
        *split_exp      = '\0';
        expected_result = atoi(split_exp + 1);

        printf("No %d -> Exp: %s, exp result: %d", count, expression, expected_result);
        int ans = evaluate(arena, expression);
        printf(",got: %d\n", ans);
        ASSERT(expected_result, ans);
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

    arena arena = arena_init(&array, size);
    main_arena  = &arena;

    // run_tests();
    printf("Type in your expression: \n");
    while (true)
    {
        char expression[30];
        // Read input from stdin
        fgets(expression, sizeof(expression), stdin);
        if (is_equal("exit\n", expression))
            break;

        int ans = evaluate(main_arena, expression);
        printf(">>> %d\n", ans);
    }

    return 0;
}
