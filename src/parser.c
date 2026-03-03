#include "parser.h"
#include "defines.h"
#include "tokenizer.h"

static arena* main_arena;


tree_node *build_binary_operator_tree(tree_node *left, tree_node *right, operator op)
{
    tree_node *tree = arena_alloc(main_arena, sizeof(tree_node));

    tree->value    = INT_MIN;
    tree->operator = op;

    tree->left  = left;
    tree->right = right;

    return tree;
}

tree_node *get_new_tree_node(float value, operator op)
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

    token *curr_token = token_peek();

    if (curr_token->type == TOKEN_OPEN_PAREN)
    {
        // consume the '('
        token_consume();
        left       = parse(0.0);
        // now the curr_token should be ')'
        curr_token = token_peek();
        if (curr_token->type != TOKEN_CLOSE_PAREN)
        {
            printf("Expected TOKEN_CLOSE_PAREN, but got %s\n", token_get_string(curr_token));
            DEBUG_BREAK;
        }
        // consume the ')'
        token_consume();
        return left;
    }

    if (curr_token->type == TOKEN_INT)
    {
        // if this is an int token then the next token has to be an operator and that operator cannot be a OPEN_PAREN
        token *next_token = token_peek_next();
        if (next_token->type == TOKEN_OPEN_PAREN)
        {
            printf("Expected an operator but got an %s\n", token_get_string(next_token));
            DEBUG_BREAK;
        }
        left = get_new_tree_node(curr_token->value, TOKEN_LEAF_NODE);
        token_consume();
        return left;
    }

    return NULL;
}

tree_node *parse(float parent_precedence)
{
    tree_node *left = parse_leaf();

    while (true)
    {
        token *token = token_peek();
        if (token->type == TOKEN_END || token->type == TOKEN_CLOSE_PAREN)
            break;

        operator op         = (operator)token->type;
        int      precedence = token_get_precedence(op);

        if (precedence < parent_precedence)
            break;
        // consume the operator
        token_consume();

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
/*
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
*/

float tree_dfs(tree_node *tree)
{
    if (tree == NULL)
        return 0;
    if (tree->operator == TOKEN_LEAF_NODE)
    {
        return tree->value;
    }
    float left   = tree_dfs(tree->left);
    float right  = tree_dfs(tree->right);
    float result = 0;

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
        if(right == 0)
        {
            printf("DIVIDE BY ZERO\n");
            DEBUG_BREAK;
       }
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

float evaluate(arena *arena, char *exp)
{
    main_arena = arena;
    bool res = init_tokenizer(arena, exp);
    if(res == false)return INT_MIN;
    tree_node *tree = parse(0.0);

    float result = tree_dfs(tree);

    return result;
}
