#include "tokenizer.h"

static tokenizer_state *state;

bool       is_digit(char *atom);
int        convert_to_digit(char *atom);

int token_get_precedence(token_type op)
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
// this is char* because we are still doing the lexial analysis part: we want to convert  
bool token_is_operator(char* token)
{
    if (token == NULL)
    {
        printf("Expected an operator token but got NULL instead\n");
        DEBUG_BREAK;
    }
    switch (*token)
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


const char *token_get_string(token *token)
{
    if (token == NULL)
    {
        printf("Expected an operator token but got NULL instead\n");
        DEBUG_BREAK;
    }
    switch (token->type)
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

int token_get_atom()
{
    int    index = state->curr_token;
    token *token = &state->tokens[index];
    if (token->type == TOKEN_END)
    {
        return INT_MIN;
    }
    if (token->type != TOKEN_INT)
    {
        printf("Expected TOKEN_TYPE INT but got %s\n", token_get_string(token));
        DEBUG_BREAK;
    }
    int value = token->value;
    return value;
}

void token_consume()
{
    state->curr_token++;
}

token *token_peek()
{
    int index = state->curr_token;
    return &state->tokens[index];
}

token *token_peek_next()
{
    int index = state->curr_token;
    if (index + 1 > state->tokens_length)
    {
        // return 'END OF EXP' token;
        return &state->tokens[index];
    }
    return &state->tokens[index + 1];
}

void init_tokenizer(arena *arena, char *exp)
{
    // if we have never initialized this then initialize it.
    if (exp == NULL)
    {
        printf("Expression is NULL\n");
        DEBUG_BREAK;
    }
    if (state == NULL)
    {
        state = arena_alloc(arena, sizeof(tokenizer_state));
    }
    state->exp           = exp;
    state->curr_token    = 0;
    state->tokens_length = 0;

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
        else if (token_is_operator(ptr))
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
        state->tokens[index++] = token;
    } while (ptr != NULL && *ptr != '\0' && *ptr != '\n');

    state->tokens_length = index;
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
