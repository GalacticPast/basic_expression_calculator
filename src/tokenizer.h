#pragma once
#include "arena.h"
#include "defines.h"

#define MAX_TOKEN_ARRAY_COUNT 128
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
    // where does it start in the original exp ------v
    int starting_ind; // needed for error reproting
    // where does it end in the original exp --------^
    int ending_ind;
    // only for the TOKEN_INT type
    int value;
} token;

typedef struct tokenizer_state
{
    // original expression
    char *exp;
    int   exp_length;
    int   curr_token;
    int   tokens_length;
    // we could do a sliding window approach but since I dont expect anyone to write
    // a mathematical expression larger then 124 characters :)
    token tokens[MAX_TOKEN_ARRAY_COUNT];
} tokenizer_state;

const char *token_get_string(token *token);
int         token_get_precedence(token_type op);
int         token_get_atom();
void        token_consume();
token      *token_peek();
token      *token_peek_next();
// this returs false it there was an syntax error  in the expression
bool init_tokenizer(arena *arena, char *exp);
