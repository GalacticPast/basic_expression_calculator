#include "defines.h"
#include "tokenizer.h"

typedef struct tree_node
{
    token_type        operator;
    float             value;
    struct tree_node *left;
    struct tree_node *right;
} tree_node;

tree_node *parse(float parent_precedence);
tree_node *parse_leaf();
float      evaluate(arena *arena, char *exp);
