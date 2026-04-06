#include "db.h"
#include "tokenizer.h"

typedef struct tree_node
{
    token_type        operator;
    f32               value;
    struct tree_node *left;
    struct tree_node *right;
} tree_node;

tree_node *parse(f32 parent_precedence);
tree_node *parse_leaf();
f32        evaluate(db_arena *arena, char *exp);
