#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEBUG_BREAK asm("int $3")
#define INT_MIN -2147483648
#define INT_MAX 2147483647
typedef int bool;
#define true 1
#define false 0
#define MAX_TOKEN_ARRAY_COUNT 128
#define MAX(n, m) (int)n >= (int)m ? (int)n : (int)m
#define ASSERT(m, n)                                                                                                   \
    if ((int)m != (int)n)                                                                                              \
    {                                                                                                                  \
        printf("Expected_result: %d, Ans : %d,", m, n);                                                                \
        DEBUG_BREAK;                                                                                                   \
    }

typedef struct arena
{
    int total_size;
    int allocated_till_now;
    // the index
    void *ptr;
    // starting mem
    void *mem;
} arena;

static arena *main_arena;
typedef struct array_header
{
    int   total_length;
    int   type_size;
    int   count;
    void *mem;
} array_header;

// I realized we need this because this will keep track of where we are in terms of parsing
// this takes care of when the recursive function's token gets ahead of its parent and when it returns, the parent
// function/tokenizer doesnt know from where to parse , like a "pointer amnesia"
//  for example:
//  expression : 1 * 2 + 3 * 4
// parent_function thinks that its on the * operator and it calls it self.
//  Now the recursive function starts from * and parses the rest of the exp and it returns a tree.
//  now its the turn of the parent_function and it has no way of knowing that we have finished parsing the exp.
// so this struct will keep track of where we are in terms of parsing.
// we could just pass a double pointer which would fix the problem, but I think this is more neat :)
typedef enum token_type
{
    // end of expression token,
    TOKEN_END            = 0,
    TOKEN_INT            = 1,
    TOKEN_MINUS          = '-',
    TOKEN_PLUS           = '+',
    TOKEN_MULTIPLICATION = '*',
    TOKEN_DIVISION       = '/',
    TOKEN_EXPONENT       = 'e',
    TOKEN_OPEN_PAREN     = '(',
    TOKEN_CLOSE_PAREN    = ')',
    // special type to indicate the leaf node of the tree
    TOKEN_LEAF_NODE      = 1000
} token_type;
typedef token_type operator;

typedef struct token
{
    token_type type;
    // only for the TOKEN_INT type
    int value;
} token;

typedef struct tokenizer_state
{
    // original expression
    char *exp;
    int   curr_token;
    int   tokens_length;
    // we could do a sliding window approach but since I dont expect anyone to write
    // a mathematical expression larger then 124 characters :)
    token tokens[MAX_TOKEN_ARRAY_COUNT];
} tokenizer_state;

typedef struct tree_node
{
    token_type        operator;
    int               value;
    struct tree_node *left;
    struct tree_node *right;
} tree_node;

static tokenizer_state *token_state;

int get_terminal_width(void);

void *arena_alloc(arena *arena, int byte_size)
{
    if (!arena)
    {
        DEBUG_BREAK;
    }
    if (byte_size % 2 != 0)
    {
        DEBUG_BREAK;
    }
    void *ret_mem = NULL;
    if (byte_size + arena->allocated_till_now < arena->total_size)
    {
        ret_mem                    = arena->ptr;
        arena->ptr                += byte_size;
        arena->allocated_till_now += byte_size;
    }
    else
    {
        printf("Arena doesn't have enought space to accodomate allocation of size %d bytes. Arena total size %d, Arena "
               "allocated size till now %d",
               byte_size, arena->total_size, arena->allocated_till_now);
    }

    if (ret_mem == NULL)
    {
        DEBUG_BREAK;
    }

    memset(ret_mem, 0, byte_size);
    return ret_mem;
}

arena arena_init(void *mem, int total_size)
{
    if (!mem)
    {
        DEBUG_BREAK;
    }
    arena arena              = {};
    arena.mem                = mem;
    arena.ptr                = mem;
    arena.total_size         = total_size;
    arena.allocated_till_now = 0;

    return arena;
}

void arena_reset(arena *arena)
{
    arena->allocated_till_now = 0;
    arena->ptr                = arena->mem;
    memset(arena->mem, 0, arena->total_size);
}

void *array_init(arena *arena, int type_size, int length, bool initialize)
{
    int byte_size = length * type_size;

    void *mem = arena_alloc(main_arena, byte_size + sizeof(array_header));

    array_header *header = mem;
    header->total_length = length;
    header->type_size    = type_size;
    header->mem          = mem + sizeof(array_header);

    if (initialize)
    {
        header->count = length;
    }
    else
    {
        header->count = 0;
    }

    return header->mem;
}

void array_insert_element(void *array, void *element, int index)
{
    array_header *header = array - sizeof(array_header);
    if (!header)
    {
        DEBUG_BREAK;
    }

    if (index > header->count)
    {
        printf("out of index , array_count:%d, index:%d\n", header->count, index);
        DEBUG_BREAK;
    }
    void *mem = (index * header->type_size) + header->mem;
    memcpy(mem, element, header->type_size);
}

void *array_get_element(void *array, int index)
{
    array_header *header = array - sizeof(array_header);
    if (!header)
    {
        DEBUG_BREAK;
    }
    if (index > header->count)
    {
        printf("out of index , array_count:%d, index:%d\n", header->count, index);
        DEBUG_BREAK;
    }
    void *elem = (index * header->type_size) + header->mem;
    return elem;
}

int array_get_length(void *array)
{
    array_header *header = array - sizeof(array_header);
    if (!header)
    {
        DEBUG_BREAK;
    }
    return header->count;
}

void array_push_back(void *array, void *element)
{
    array_header *header = array - sizeof(array_header);
    if (!header)
    {
        DEBUG_BREAK;
    }

    if (header->count + 1 > header->total_length)
    {
        printf("Ran out of array space, array_length:%d\n", header->total_length);
        DEBUG_BREAK;
    }
    void *mem = (header->count * header->type_size) + header->mem;
    memcpy(mem, element, header->type_size);
    header->count += 1;
}

int get_terminal_width(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col; // make sure your main returns int
}

int get_precedence(token_type op)
{
    switch (op)
    {
    case TOKEN_MINUS:
        return 1;
    case TOKEN_PLUS:
        return 1;
    case TOKEN_MULTIPLICATION:
        return 2;
    case TOKEN_DIVISION:
        return 2;
    case TOKEN_EXPONENT:
        return 5;
    case TOKEN_OPEN_PAREN:
        return 0;
    case TOKEN_CLOSE_PAREN:
        return 6;
    default:
        return 0.0;
    }
    return INT_MIN;
}

token *peek()
{
    int index = token_state->curr_token;
    return &token_state->tokens[index];
}

token *peek_next()
{
    int index = token_state->curr_token;
    if (index + 1 > token_state->tokens_length)
    {
        // return 'END OF EXP' token;
        return &token_state->tokens[index];
    }
    return &token_state->tokens[index + 1];
}

bool is_digit(char *atom)
{
    if (atom == NULL)
    {
        printf("Expected an atom to check if it's digit but got NULL instead\n");
        DEBUG_BREAK;
    }
    bool ans = (*atom >= '0' && *atom <= '9' ? 1 : 0);
    return ans;
}

int convert_to_digit(char *atom)
{
    if (atom == NULL)
    {
        printf("Expected an digit atom but got NULL instead\n");
        DEBUG_BREAK;
    }
    if (is_digit(atom) == false)
    {
        printf("Expected the provided atom: %c, to be an digit.\n", *atom);
        DEBUG_BREAK;
    }
    int digit = *atom - '0';
    return digit;
}

const char *get_string_from_operator(operator op)
{
    switch (op)
    {
    case TOKEN_INT:
        return "TOKEN_INT";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MULTIPLICATION:
        return "TOKEN_MULTIPLICATION";
    case TOKEN_DIVISION:
        return "TOKEN_DIVISION";
    case TOKEN_EXPONENT:
        return "TOKEN_EXPONENT";
    case TOKEN_OPEN_PAREN:
        return "TOKEN_OPEN_PAREN";
    case TOKEN_CLOSE_PAREN:
        return "TOKEN_CLOSE_PAREN";
    default:
        return "";
    }
    return "";
}

bool is_operator(char *atom)
{
    if (atom == NULL)
    {
        printf("Expected an operator atom but got NULL instead\n");
        DEBUG_BREAK;
    }
    switch (*atom)
    {
    case TOKEN_INT:
    case TOKEN_MINUS:
    case TOKEN_PLUS:
    case TOKEN_MULTIPLICATION:
    case TOKEN_DIVISION:
    case TOKEN_EXPONENT:
    case TOKEN_OPEN_PAREN:
    case TOKEN_CLOSE_PAREN:
        return true;
    default:
        return false;
    }
    return false;
}
void init_tokenizer(char *exp)
{
    // if we have never initialized this then initialize it.
    if (exp == NULL)
    {
        printf("Expression is NULL\n");
        DEBUG_BREAK;
    }
    if (token_state == NULL)
    {
        token_state = arena_alloc(main_arena, sizeof(tokenizer_state));
    }
    token_state->exp           = exp;
    token_state->curr_token    = 0;
    token_state->tokens_length = 0;

    // convert exp into tokens

    int   index = 0;
    token token = {.type = TOKEN_END, .value = INT_MIN};

    int length = strlen(exp);

    char *ptr = exp;

    do
    {
        if (is_digit(ptr))
        {
            int value = convert_to_digit(ptr);
            ptr++;
            while (ptr != NULL && *ptr != '\n' && *ptr != '\0' && is_digit(ptr))
            {
                value *= 10;
                value += convert_to_digit(ptr);
                ptr++;
            }
            token.type  = TOKEN_INT;
            token.value = value;
        }
        else if (is_operator(ptr))
        {
            token.type  = (token_type)*ptr;
            token.value = INT_MIN;
            ptr++;
        }
        else if (*ptr == ' ')
        {
            ptr++;
            continue;
        }
        token_state->tokens[index++] = token;
    } while (ptr != NULL && *ptr != '\0' && *ptr != '\n');

    token_state->tokens_length = index;
}

int get_atom()
{
    int    index = token_state->curr_token;
    token *token = &token_state->tokens[index];
    if (token->type == TOKEN_END)
    {
        return INT_MIN;
    }
    if (token->type != TOKEN_INT)
    {
        printf("Expected TOKEN_TYPE INT but got %s\n", get_string_from_operator(token->type));
        DEBUG_BREAK;
    }
    int value = token->value;
    return value;
}

void consume_token()
{
    token_state->curr_token++;
}
tree_node *build_binary_operator_tree(tree_node *left, tree_node *right, operator op)
{
    tree_node *tree = arena_alloc(main_arena, sizeof(tree_node));

    tree->value    = INT_MIN;
    tree->operator = op;

    tree->left  = left;
    tree->right = right;

    return tree;
}

tree_node *get_new_tree_node(int value, operator op)
{
    tree_node *node = arena_alloc(main_arena, sizeof(tree_node));

    node->operator = op;
    node->value    = value;
    node->right    = NULL;
    node->left     = NULL;

    return node;
}
// refine this

tree_node *parse(float parent_precedence);

tree_node *parse_leaf()
{

    tree_node *left = NULL;

    token *curr_token = peek();

    if (curr_token->type == TOKEN_OPEN_PAREN)
    {
        // consume the '('
        consume_token();
        left  = parse(0.0);
        // now the curr_token should be ')'
        curr_token = peek();
        if (curr_token->type != TOKEN_CLOSE_PAREN)
        {
            printf("Expected TOKEN_CLOSE_PAREN, but got %s\n", get_string_from_operator(curr_token->type));
            DEBUG_BREAK;
        }
        // consume the ')'
        consume_token();
        return left;
    }

    if (curr_token->type == TOKEN_INT)
    {
        // if this is an int token then the next token has to be an operator and that operator cannot be a OPEN_PAREN 
        token* next_token = peek_next();
        if(next_token->type == TOKEN_OPEN_PAREN)
        {
            printf("Expected an operator but got an %s\n", get_string_from_operator(next_token->type));
            DEBUG_BREAK;
        }
        left = get_new_tree_node(curr_token->value, TOKEN_LEAF_NODE);
        consume_token();
        return left;
    }

    return NULL;
}

tree_node *parse(float parent_precedence)
{
    tree_node *left = parse_leaf();

    while (true)
    {
        token *token = peek();
        if (token->type == TOKEN_END || token->type == TOKEN_CLOSE_PAREN)
            break;
        if (token->type == TOKEN_INT)
        {
            // this means there was two consequetive integers seperated by a whitespace: for example 0 + 1 2 ...
            // this is an error                                                                           ^
            printf("Invalid Expression: there was two consequetive integers. Please retry again.\n");
            DEBUG_BREAK;
        }

        operator op         = (operator)token->type;
        int      precedence = get_precedence(op);

        if (precedence < parent_precedence)
            break;
        // consume the operator
        consume_token();

        tree_node *right = parse(precedence + 0.1);

        left = build_binary_operator_tree(left, right, op);
    }
    return left;
}

// this is going to flatten the tree
int get_tree_height(tree_node *tree)
{
    if (tree == NULL)
        return 1;

    int left  = get_tree_height(tree->left) + 1;
    int right = get_tree_height(tree->right) + 1;
    return MAX(left, right);
}

void tree_bfs(tree_node *tree, tree_node *array, int parent_array_index)
{
    if (tree == NULL)
        return;

    if (parent_array_index == 0)
    {
        array_insert_element(array, tree, 0);
    }

    int left_index  = 2 * parent_array_index + 1;
    int right_index = 2 * parent_array_index + 2;

    if (tree->left != NULL)
    {
        array_insert_element(array, tree->left, left_index);
    }
    if (tree->right != NULL)
    {
        array_insert_element(array, tree->right, right_index);
    }

    tree_bfs(tree->left, array, left_index);
    tree_bfs(tree->right, array, right_index);
}

int tree_dfs(tree_node *tree)
{
    if (tree == NULL)
        return 0;
    if (tree->operator == TOKEN_LEAF_NODE)
    {
        return tree->value;
    }
    int left   = tree_dfs(tree->left);
    int right  = tree_dfs(tree->right);
    int result = 0;

    switch (tree->operator)
    {
    case TOKEN_MINUS: {
        result = left - right;
    }
    break;
    case TOKEN_PLUS: {
        result = left + right;
    }
    break;
    case TOKEN_MULTIPLICATION: {
        result = left * right;
    }
    break;
    case TOKEN_DIVISION: {
        result = left / right;
    }
    break;
    case TOKEN_EXPONENT: {
        result = pow(left, right);
    }
    break;
    default: {
    }
    break;
    }
    return result;
}

int evaluate(char *exp)
{
    init_tokenizer(exp);

    tree_node *tree = parse(0.0);

    int result = tree_dfs(tree);

    return result;
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

void run_tests()
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
        int ans = evaluate(expression);
        printf(",got: %d\n", ans);
        ASSERT(expected_result, ans);
        arena_reset(main_arena);
        token_state = NULL;
        count++;
    }
}

int main()
{
    // n * KB
    int  size = 10 * 1024;
    char array[size];

    arena arena = arena_init(&array, size);
    main_arena  = &arena;

    // run_tests();

    char expression[30];
    printf("Type in your expression: \n");
    // Read input from stdin
    fgets(expression, sizeof(expression), stdin);
    int ans = evaluate(expression);
    printf("%d\n", ans);

    return 0;
}
